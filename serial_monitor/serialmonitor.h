/*
 * Serial monitor for arduino
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

#ifndef SERIALMONITOR_H
#define SERIALMONITOR_H

#include <QMainWindow>
#include <QSerialPort>

namespace Ui {
class SerialMonitor;
}

class SerialMonitor : public QMainWindow
{
    Q_OBJECT

public:
    explicit SerialMonitor(QWidget *parent = 0);
    ~SerialMonitor();

private slots:
    void connectSerial();
    void disconnectSerial();
    void sendData();
    void handleError(QSerialPort::SerialPortError error);
    void readData();
    void clearReceive();
    void identifyPorts();
    void changedBoudRate(int index);
private:
    Ui::SerialMonitor *ui;
    enum LINE_TERMINATION { NONE, LF, CR, CR_LF};
    QSerialPort *serial;
};

#endif // SERIALMONITOR_H
