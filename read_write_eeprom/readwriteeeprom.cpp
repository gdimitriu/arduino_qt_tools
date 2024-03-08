/*
 * Read/write eeproms using serial and arduino.
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

#include "readwriteeeprom.h"
#include "ui_readwriteeeprom.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QtSerialPort/QSerialPortInfo>

ReadWriteEEPROM::ReadWriteEEPROM(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ReadWriteEEPROM)
{
    ui->setupUi(this);
    setupCommComboBoxDefault();
    createMenus();
    setupComs();
    setupEproms();
}

ReadWriteEEPROM::~ReadWriteEEPROM()
{
    if ( outFile.is_open() ) {
        outFile.close();
    }
    delete ui;
}

void ReadWriteEEPROM::setupComs()
{
    serial = nullptr;
    connect(ui->connectButton, SIGNAL(clicked(bool)), this, SLOT(connectSerial()));
    connect(ui->disconnectButton, SIGNAL(clicked(bool)), this, SLOT(disconnectSerial()));
    ui->statusLine->setText("Disconnected");
    connect(ui->detectComs, SIGNAL(clicked(bool)), this, SLOT(detectPorts()));
    connect(ui->boudRate, SIGNAL(currentIndexChanged(int)), this, SLOT(changedBoudRate(int)));
    //clear views button
    connect(ui->clearReadWriteButton, SIGNAL(clicked(bool)), this, SLOT(clearReadWrite()));
    connect(ui->clearLogsButton, SIGNAL(clicked(bool)), this, SLOT(clearLogs()));
    connect(ui->readButton, SIGNAL(clicked(bool)), this, SLOT(sendReadCommand()));
    connect(ui->writeButton, SIGNAL(clicked(bool)), this, SLOT(sendWriteCommand()));
    ui->bufferSize->setText("512");
    connect(ui->setBuffer, SIGNAL(clicked(bool)), this, SLOT(sendBufferSizeCommand()));
}

void ReadWriteEEPROM::setupCommComboBoxDefault()
{
    //setup line termination
    ui->lineTermination->addItem("None", LINE_TERMINATION::NONE);
    ui->lineTermination->addItem("LF", LINE_TERMINATION::LF);
    ui->lineTermination->addItem("CR", LINE_TERMINATION::CR);
    ui->lineTermination->addItem("LF&CR", LINE_TERMINATION::LF_CR);
    //setup boud rate
    ui->boudRate->addItem("2400");
    ui->boudRate->addItem("4800");
    ui->boudRate->addItem("9600");
    ui->boudRate->addItem("19200");
    ui->boudRate->addItem("38400");
    ui->boudRate->addItem("57600");
    ui->boudRate->addItem("115200");
}

void ReadWriteEEPROM::setupEproms()
{
    //setup eprom type
    ui->eepromType->addItem("AT24Cxxx");
    ui->eepromType->addItem("24C02C");
    ui->eepromType->addItem("FM24C02");
    connect(ui->eepromType, SIGNAL(currentIndexChanged(int)), this, SLOT(selectEpromType(int)));
    ui->epromSize->addItem("128k",16384);
    ui->epromSize->addItem("256k",32768);
    ui->address->setText("0");
    ui->allBytes->setChecked(true);
}

void ReadWriteEEPROM::selectEpromType(int index)
{
    ui->epromSize->clear();
    switch (index)
    {
    case 0:
        ui->epromSize->addItem("128k",16384);
        ui->epromSize->addItem("256k",32768);
        break;
    case 1:
        ui->epromSize->addItem("2k",256);
        break;
    case 2:
        ui->epromSize->addItem("2k", 256);
        ui->epromSize->addItem("4k", 512);
        ui->epromSize->addItem("8k", 1024);
        ui->epromSize->addItem("16k", 2048);
        break;
    default:
        ui->epromSize->addItem("unkown");
    }
}
void ReadWriteEEPROM::createMenus()
{
    readFileAction = new QAction(tr("&ReadFile"), this);
    readFileAction->setShortcut(tr("Ctrl+R"));
    readFileAction->setStatusTip(tr("Open a file to send to EEPROM"));
    connect(readFileAction, SIGNAL(triggered()), this, SLOT(readOpenFile()));
    writeFileAction = new QAction(tr("&WriteFile"), this);
    writeFileAction->setShortcut(tr("Ctrl+W"));
    writeFileAction->setStatusTip(tr("Open a file to write from EEPROM"));
    connect(writeFileAction, SIGNAL(triggered()), this, SLOT(writeOpenFile()));

    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(readFileAction);
    fileMenu->addAction(writeFileAction);
}

void ReadWriteEEPROM::readOpenFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file to send to EEPROM"), ".", tr("Dat files (*.dat)"));
    if ( !fileName.isEmpty() ) {
        QMessageBox::information(this, tr("Sucessfull open file to send to EEPROM"), fileName, QMessageBox::Yes);
        ui->inFileName->clear();
        ui->inFileName->insert(fileName);
    }
}

void ReadWriteEEPROM::writeOpenFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Open file to write from EEPROM"), ".", tr("Dat files (*.dat)"));
    if ( !fileName.isEmpty() ) {
        QMessageBox::information(this, tr("Sucessfull open file to write from EEPROM"), fileName, QMessageBox::Yes);
        ui->outFileName->clear();
        ui->outFileName->insert(fileName);
    }
}


void ReadWriteEEPROM::changedBoudRate(int index)
{
    if ( serial != nullptr && serial->isOpen() )
    {
        serial->setBaudRate(ui->boudRate->currentText().toInt());
    }
}


void ReadWriteEEPROM::connectSerial()
{
    if (serial != nullptr && serial->isOpen() )
    {
        serial->close();
        delete serial;
    } else if ( serial != nullptr )
    {
        delete serial;
    }
    QString portName = ui->comPorts->currentText();
    if ( portName.isEmpty() )
        return;
    serial = new QSerialPort(this);
    serial->setPortName(portName);
    serial->setBaudRate(ui->boudRate->currentText().toInt());
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this,
            SLOT(handleError(QSerialPort::SerialPortError)));
    if (serial->open(QIODevice::ReadWrite))
    {
        serial->setDataTerminalReady(true);
        ui->statusLine->setText("Connected");
    }
    ui->connectButton->clearFocus();
}

void ReadWriteEEPROM::disconnectSerial()
{
    if ( serial != nullptr )
    {
        ui->statusLine->setText("Disconnected");
        disconnect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this,
               SLOT(handleError(QSerialPort::SerialPortError)));
        serial->close();
        delete serial;
        serial = nullptr;
    }
    ui->disconnectButton->clearFocus();
    if ( outFile.is_open() ) {
        outFile.close();
    }
}


void ReadWriteEEPROM::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        disconnectSerial();
    }
}


void ReadWriteEEPROM::detectPorts()
{
    ui->comPorts->clear();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QString infoData = QObject::tr("Port: ") + info.portName() + "\n"
                    + QObject::tr("Location: ") + info.systemLocation() + "\n"
                    + QObject::tr("Description: ") + info.description() + "\n"
                    + QObject::tr("Manufacturer: ") + info.manufacturer() + "\n"
                    + QObject::tr("Serial number: ") + info.serialNumber() + "\n"
                    + QObject::tr("Busy: ") + (info.isBusy() ? QObject::tr("Yes") : QObject::tr("No")) + "\n"
                    + "-------------------------------------------\n";
        ui->logView->moveCursor(QTextCursor::End);
        ui->logView->insertPlainText(infoData);
        ui->comPorts->addItem(info.portName());
    }
    ui->detectComs->clearFocus();
    QMainWindow::repaint();
}

void ReadWriteEEPROM::clearLogs()
{
    ui->logView->clear();
    ui->clearLogsButton->clearFocus();
}

void ReadWriteEEPROM::clearReadWrite()
{
    ui->readWriteView->clear();
    ui->clearReadWriteButton->clearFocus();
}

void ReadWriteEEPROM::sendReadCommand()
{
    if ( serial == nullptr || !serial->isOpen() )
    {
        QMessageBox::critical(this, tr("First connect to serial."), tr("First connect to serial"), QMessageBox::Ok);
        return;
    }
    if ( ui->deviceAddress->text().isEmpty() )
    {
        QMessageBox::critical(this, tr("First complete the device address."), tr("First complete the device address"), QMessageBox::Ok);
        return;
    }
    disconnect(serial, SIGNAL(readyRead()), this, SLOT(readDataToViewOrDump()));
    connect(serial, SIGNAL(readyRead()), this, SLOT(readDataToViewOrDump()));
    QString str;
    //read one byte
    if ( ui->length->text().isEmpty() )
    {
        str.push_back('r');
        str.push_back(ui->deviceAddress->text());
        str.push_back(',');
        str.push_back(ui->address->text());
        str.push_back('#');
    } else {
        str.push_back('R');
        str.push_back(ui->deviceAddress->text());
        str.push_back(',');
        str.push_back(ui->address->text());
        str.push_back(',');
        str.push_back(ui->length->text());
        str.push_back('#');
    }
    if ( !ui->outFileName->text().isEmpty() )
    {
        if ( outFile.is_open() ) {
            outFile.close();
        }
        outFile.open(ui->outFileName->text().toStdString().c_str(), std::ios::trunc);
    }
    switch ( ui->lineTermination->currentData().toInt() )
    {
    case LINE_TERMINATION::NONE :
        serial->write(str.toLatin1());
        break;
    case LINE_TERMINATION::LF :
        str.append('\n');
        serial->write(str.toLatin1());
        break;
    case LINE_TERMINATION::CR :
        str.append('\r');
        serial->write(str.toLatin1());
        break;
    case LINE_TERMINATION::LF_CR :
        str.append("\n\r");
        serial->write(str.toLatin1());
        break;
    default:
        serial->write(str.toLatin1());
    }
    ui->readButton->clearFocus();
}

void ReadWriteEEPROM::readDataToViewOrDump()
{
    QString readData = QString::fromStdString(serial->readAll().toStdString());
    ui->readWriteView->moveCursor(QTextCursor::End);
    ui->readWriteView->insertPlainText(readData);
    serial->flush();
    if ( outFile.is_open() )
    {
        outFile<<readData.toLatin1().toStdString();
        outFile.flush();
    }
    QMainWindow::repaint();
}

void ReadWriteEEPROM::sendWriteCommand()
{
    if ( serial == nullptr || !serial->isOpen() )
    {
        QMessageBox::critical(this, tr("First connect to serial."), tr("First connect to serial"), QMessageBox::Ok);
        return;
    }
    if ( ui->deviceAddress->text().isEmpty() )
    {
        QMessageBox::critical(this, tr("First complete the device address."), tr("First complete the device address"), QMessageBox::Ok);
        return;
    }
    if ( ui->onlyByte->isChecked() )
    {
        QString command;
        command.push_back('w');
        command.push_back(ui->deviceAddress->text());
        command.push_back(',');
        command.push_back(ui->address->text());
        command.push_back(',');
        command.push_back(getByteFromString(ui->dataToSend->text()));
        command.push_back('#');
        sendCommand(command);
    } else if ( ui->inFileName->text().isEmpty() ){
        if ( ui->dataToSend->text().length() <= ui->bufferSize->text().toLong() ) {
            sendWriteMultiple(ui->deviceAddress->text(), ui->address->text().toLong(), ui->dataToSend->text().length(), 0);
        }
        else
        {
            long bufferSize = ui->bufferSize->text().toLong();
            long length = ui->dataToSend->text().length();
            long currentIndex = 0;
            long currentAddress = 0;
            while ( ( currentIndex + bufferSize) <= length )
            {
                sendWriteMultiple(ui->deviceAddress->text(), currentAddress, bufferSize, currentIndex);
                currentIndex += bufferSize;
                currentAddress += bufferSize;
            }
            if ( ( currentIndex + bufferSize) > length )
            {
                sendWriteMultiple(ui->deviceAddress->text(), currentAddress, length - currentIndex, currentIndex);
            }
        }
    }
    ui->writeButton->clearFocus();
}

void ReadWriteEEPROM::sendWriteMultiple(QString deviceAddress, long address, long length, long startPos)
{
    QString command;
    command.push_back('W');
    command.push_back(deviceAddress);
    command.push_back(',');
    command.push_back(QString::number(address));
    command.push_back(',');
    command.push_back(QString::number(length));
    command.push_back('#');
    sendCommand(command);
    serial->write(ui->dataToSend->text().mid(startPos, startPos + length).toLatin1());
}

unsigned char ReadWriteEEPROM::getByteFromString(QString str)
{
   int index1 = str.indexOf("0x");
   int index2 = str.indexOf("0x", index1 + 2);
   if ( index2 !=  -1 )
   {
       return str.left(index2).toInt();
   }
   bool ok;
   unsigned char value =  str.toInt(&ok,16);
   return value;
}

void ReadWriteEEPROM::sendCommand(QString command)
{
    switch ( ui->lineTermination->currentData().toInt() )
    {
    case LINE_TERMINATION::NONE :
        serial->write(command.toLatin1());
        break;
    case LINE_TERMINATION::LF :
        command.append('\n');
        serial->write(command.toLatin1());
        break;
    case LINE_TERMINATION::CR :
        command.append('\r');
        serial->write(command.toLatin1());
        break;
    case LINE_TERMINATION::LF_CR :
        command.append("\n\r");
        serial->write(command.toLatin1());
        break;
    default:
        serial->write(command.toLatin1());
    }
}

void ReadWriteEEPROM::sendBufferSizeCommand()
{
    QString command;
    command.push_back('b');
    command.push_back(ui->bufferSize->text());
    command.push_back('#');
    sendCommand(command);
    ui->setBuffer->clearFocus();
}
