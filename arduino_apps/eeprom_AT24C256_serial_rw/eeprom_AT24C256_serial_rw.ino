/*
 * Read write eeprom AT24C256 using serial
 *
 * Copyright 2024 Gabriel Dimitriu
 *
 * This file is part of arduino_qt_tools project.

 * arduino_qt_tools is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * arduino_qt_tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with arduino_qt_tools; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
*/

#include <Wire.h>

#define SERIAL_BUFFER 50
boolean cleanupSerial;
bool isValidInput;
char inData[SERIAL_BUFFER]; // Allocate some space for the string
char inChar; // Where to store the character read
byte index = 0; // Index into array; where to store the character
bool isData = false;
byte *receiveSendBuffer;
unsigned int readWriteBufferLen = 512;


byte readByte_EEPROM(int deviceAddress, unsigned int address)
{
  char rdata = 0xFF;
  Wire.beginTransmission(deviceAddress);
//  Wire.write((int) (address >>8)); //MSB
//  Wire.write((int) (address & 0xFF)); //LSB
  Wire.write(address);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, 1);
  while (!Wire.available()) {
    
  }
  rdata = Wire.read();
  Wire.endTransmission();
  return rdata;
}

void writeByte_EEPROM(int deviceAddress, unsigned int address, byte data)
{
  int rdata  = data;
  Wire.beginTransmission(deviceAddress);
//  Wire.write((int) (address >> 8)); //MSB
//  Wire.write((int) (address & 0xFF)); //LSB
  Wire.write(address);
  Wire.write(rdata);
  Wire.endTransmission();
}

void readBytes_EEPROM(int deviceAddress, unsigned int address, byte * buffer, int length)
{  
  Wire.beginTransmission(deviceAddress);
//  Wire.write((int)(address >> 8)); //MSB
//  Wire.write((int) (address & 0xFF)); //LSB
  Wire.write(address);
  Wire.endTransmission();
  Wire.requestFrom(deviceAddress, length);
  for(int idx = 0; idx < length; idx++) {
    while (!Wire.available()) {
      
    }
    buffer[idx] = Wire.read();
  }
  Wire.endTransmission();
}

void writeBytes_EEPROM(int deviceAddress, unsigned int address, byte *data, int length)
{
  Wire.beginTransmission(deviceAddress);
//  Wire.write((int) (address >> 8)); //MSB
//  Wire.write((int) (address & 0xFF)); //LSB
  Wire.write(address);
  for(int idx = 0; idx < length; idx++) {
    Wire.write(data[idx]);
  }
  Wire.endTransmission();
}


boolean isValidNumber( char *data, int size )
{
  if ( size == 0 ) return false;
   if( !( data[0] == '+' || data[0] == '-' || isDigit(data[0]) ) ) return false;

   for( byte i=1; i<size; i++ )
   {
       if( !( isDigit(data[i]) || data[i] == '.' ) ) return false;
   }
   return true;
}

boolean makeCleanup() {
  for ( index = 0; index < SERIAL_BUFFER; index++ ) {
    inData[index] = '\0';
  }
  index = 0;
  inChar ='0';
  Serial.flush();
}

void setup() {
  Serial.begin(9600);   
  Wire.begin();
  isValidInput = false;
  cleanupSerial = false;
  makeCleanup();
  receiveSendBuffer = new byte[readWriteBufferLen];
}

void processCommand() {
  //remove #
  if ( index > 0 ) {
     inData[index-1] = '\0';
  }
  if ( strlen(inData) > 1 ) {
    //read one byte
    if ( inData[0] == 'r' ) {
      //remove r
      for ( uint8_t i = 0 ; i < strlen(inData); i++ ) {
        inData[i]=inData[i+1];
      }
      inData[strlen(inData)] = '\0';
      int position;
      for ( uint8_t i = 0; i < strlen(inData); i++ ) {
        if ( inData[i] == ',' ) {
          position = i;
          break;
        }
      }
      
      char buf[10];
      memset(buf,'\0', 10 * sizeof(char));
      for ( int i = 0 ; i < position; i++ ) {
        buf[i] = inData[i];
      }
      int deviceAddress = strtol(buf, 0, 16);
      
      memset(buf,'\0', 10 * sizeof(char));
      for ( int i = position + 1, idx = 0; i < strlen(inData); i++, idx++ ) {
        buf[idx] = inData[i];
      }
      int address = strtol(buf, 0, 16);
      byte value = readByte_EEPROM(deviceAddress, address);
      Serial.print(value);
      Serial.flush();
    }
    //write one byte
    else if ( inData[0] == 'w' ) {
      //remove w
      for ( uint8_t i = 0 ; i < strlen(inData); i++ ) {
        inData[i]=inData[i+1];
      }
      inData[strlen(inData)] = '\0';
      uint8_t position;
      for ( uint8_t i = 0; i < strlen(inData); i++ ) {
        if ( inData[i] == ',' ) {
          position = i;
          break;
        }
      }
      char buf[10];
      memset(buf,'\0', 10 * sizeof(char));
      
      for ( int i = 0 ; i < position; i++ ) {
        buf[i] = inData[i];
      }      
      int deviceAddress = strtol(buf, 0, 16);

      memset(buf,'\0', 10 * sizeof(char));
      
      uint8_t oldPosition = position;
      for ( uint8_t i = oldPosition + 1; i < strlen(inData); i++ ) {
        if ( inData[i] == ',' ) {
          position = i;
          break;
        }
      }
      for ( int i = oldPosition + 1, idx = 0; i < position; i++, idx++ ) {
        buf[idx] = inData[i];
      }
      int address = atoi(buf);

      memset(buf,'\0', 10 * sizeof(char));

      for ( int i = position + 1, idx = 0; i < strlen(inData); i++, idx++ ) {
        buf[idx] = inData[i];
      }
      int value = atoi(buf);
      writeByte_EEPROM(deviceAddress, address, value);
    }
    // read multiple bytes
    else if ( inData[0] == 'R' ) {
      //remove R
      for ( uint8_t i = 0 ; i < strlen(inData); i++ ) {
        inData[i]=inData[i+1];
      }
      inData[strlen(inData)] = '\0';
      uint8_t position;
      for ( uint8_t i = 0; i < strlen(inData); i++ ) {
        if ( inData[i] == ',' ) {
          position = i;
          break;
        }
      }
      char buf[10];
      memset(buf,'\0', 10 * sizeof(char));
      
      for ( int i = 0 ; i < position; i++ ) {
        buf[i] = inData[i];
      }      
      int deviceAddress = strtol(buf, 0, 16);

      memset(buf,'\0', 10 * sizeof(char));
      
      uint8_t oldPosition = position;
      for ( uint8_t i = oldPosition + 1; i < strlen(inData); i++ ) {
        if ( inData[i] == ',' ) {
          position = i;
          break;
        }
      }
      for ( int i = oldPosition + 1, idx = 0; i < position; i++, idx++ ) {
        buf[idx] = inData[i];
      }
      int address = atoi(buf);

      memset(buf,'\0', 10 * sizeof(char));

      for ( int i = position + 1, idx = 0; i < strlen(inData); i++, idx++ ) {
        buf[idx] = inData[i];
      }
      int length = atoi(buf);
      if ( length > readWriteBufferLen ) {
        delete [] receiveSendBuffer;
        readWriteBufferLen = length;
        receiveSendBuffer = new byte[readWriteBufferLen];
      }
      memset(receiveSendBuffer, '\0', sizeof(byte) * readWriteBufferLen);
      readBytes_EEPROM(deviceAddress, address, receiveSendBuffer, length);
      
      Serial.write(receiveSendBuffer,sizeof(byte) * length);
      Serial.flush();
    }
    // write multiple bytes
    else if ( inData[0] == 'W' ) {
      //remove W
      for ( uint8_t i = 0 ; i < strlen(inData); i++ ) {
        inData[i]=inData[i+1];
      }
      inData[strlen(inData)] = '\0';
      uint8_t position;
      for ( uint8_t i = 0; i < strlen(inData); i++ ) {
        if ( inData[i] == ',' ) {
          position = i;
          break;
        }
      }
      char buf[10];
      memset(buf,'\0', 10 * sizeof(char));
      
      for ( int i = 0 ; i < position; i++ ) {
        buf[i] = inData[i];
      }      
      int deviceAddress = strtol(buf, 0, 16);

      memset(buf,'\0', 10 * sizeof(char));
      
      uint8_t oldPosition = position;
      for ( uint8_t i = oldPosition + 1; i < strlen(inData); i++ ) {
        if ( inData[i] == ',' ) {
          position = i;
          break;
        }
      }
      int idx = 0;
      for ( int i = oldPosition + 1; i < position; i++ ) {
        buf[idx] = inData[i];
        idx++;
      }
      int address = atoi(buf);

      memset(buf,'\0', 10 * sizeof(char));

      idx = 0;
      for ( int i = position + 1; i < strlen(inData); i++ ) {
        buf[idx] = inData[i];
        idx++;
      }
      int length = atoi(buf);
      if ( length > readWriteBufferLen ) {
        delete [] receiveSendBuffer;
        readWriteBufferLen = length;
        receiveSendBuffer = new byte[readWriteBufferLen];
      }
      memset(receiveSendBuffer, '\0', sizeof(byte) * readWriteBufferLen);
      int index = 0;
      char inByte;
      while ( index < length ) {
        while ( Serial.available() > 0 ) // Don't read unless there you know there is data
        {
          inByte = Serial.read(); // Read a character
          receiveSendBuffer[index] = inByte;
          ++index;
        }
      }
      writeBytes_EEPROM(deviceAddress, address, receiveSendBuffer, length);
    }
    //set buffer length
    else if ( inData[0] == 'b' ) {
      //remove b from command
      for ( uint8_t i = 0 ; i < strlen(inData); i++ ) {
        inData[i]=inData[i+1];
      }
      if ( !isValidNumber(inData, index - 2) ) {
        isValidInput = false;
        makeCleanup();
        return;
      }
      unsigned int val = atoi(inData);
      if ( val > readWriteBufferLen ) {
        delete [] receiveSendBuffer;
        readWriteBufferLen = val;
        receiveSendBuffer = new byte[readWriteBufferLen];
      }
    }
  }
  makeCleanup();
}

void loop() {
  while ( Serial.available() > 0 ) // Don't read unless there you know there is data
  {
     if ( index < (SERIAL_BUFFER - 1) ) // One less than the size of the array
     {
        inChar = Serial.read(); // Read a character
        if ( inChar=='\r' || inChar=='\n' || inChar =='\0' || inChar < 35 || inChar > 122 ) {
          continue;
        }
        //commands start with a letter capital or small
        if ( index == 0 && !( ( inChar >64 && inChar <91 ) || ( inChar > 96 && inChar<123 ) ) ) {
          continue;
        }    
        inData[index++] = inChar; // Store it
        inData[index] = '\0'; // Null terminate the string
        if ( inChar == '#' ) {
          break;
        }
     } else {
        makeCleanup();
        cleanupSerial = true;
     }
  }
  if ( index >= 1 ) {
    if ( inData[index - 1] == '#' ) {    
      processCommand();
    } else if ( cleanupSerial ) {
      makeCleanup();
      cleanupSerial = false;
    } else {
      delay(10);
    }
  }
}
