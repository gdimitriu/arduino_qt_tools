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

#ifndef READWRITEEEPROM_H
#define READWRITEEEPROM_H

#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QSerialPort>
#include <fstream>

namespace Ui {
class ReadWriteEEPROM;
}

class ReadWriteEEPROM : public QMainWindow
{
    Q_OBJECT

public:
    explicit ReadWriteEEPROM(QWidget *parent = 0);
    ~ReadWriteEEPROM();
private slots:
    void readOpenFile();
    void writeOpenFile();
    void changedBoudRate(int index);
    void handleError(QSerialPort::SerialPortError error);
    void connectSerial();
    void disconnectSerial();
    void detectPorts();
    void clearLogs();
    void clearReadWrite();
    void selectEpromType(int index);
    void sendReadCommand();
    void readDataToViewOrDump();
    void sendWriteCommand();
    void sendBufferSizeCommand();
private:
    QMenu *fileMenu;

    QAction *readFileAction;
    QAction *writeFileAction;

    void createMenus();
    void setupComs();
    void setupEproms();
    enum LINE_TERMINATION { NONE, LF, CR, LF_CR};
    void setupCommComboBoxDefault();
    Ui::ReadWriteEEPROM *ui;
    QSerialPort *serial;
    std::ofstream outFile;
    unsigned char getByteFromString(QString str);
    void sendCommand(QString command);
    void sendWriteMultiple(QString deviceAddress, long address, long length, long startPos);
};

#endif // READWRITEEEPROM_H
