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
#include "serialmonitor.h"
#include "ui_serialmonitor.h"
#include <QMessageBox>
#include <QtSerialPort/QSerialPortInfo>

SerialMonitor::SerialMonitor(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SerialMonitor)
{
    ui->setupUi(this);
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
    serial = nullptr;
    connect(ui->connectButton, SIGNAL(clicked(bool)), this, SLOT(connectSerial()));
    connect(ui->disconnectButton, SIGNAL(clicked(bool)), this, SLOT(disconnectSerial()));
    connect(ui->sendButton, SIGNAL(clicked(bool)), this, SLOT(sendData()));
    ui->statusLine->setText("Disconnected");
    connect(ui->clearButton, SIGNAL(clicked(bool)), this, SLOT(clearReceive()));
    connect(ui->comListButton, SIGNAL(clicked(bool)), this, SLOT(identifyPorts()));
    connect(ui->boudRate, SIGNAL(currentIndexChanged(int)), this, SLOT(changedBoudRate(int)));
}

SerialMonitor::~SerialMonitor()
{
    delete ui;
}

void SerialMonitor::connectSerial()
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
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
    if (serial->open(QIODevice::ReadWrite))
    {
        serial->setDataTerminalReady(true);
        ui->statusLine->setText("Connected");
    }
    ui->connectButton->clearFocus();
}

void SerialMonitor::disconnectSerial()
{
    if ( serial != nullptr )
    {
        ui->statusLine->setText("Disconnected");
        disconnect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this,
               SLOT(handleError(QSerialPort::SerialPortError)));
        disconnect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
        serial->close();
        delete serial;
        serial = nullptr;
    }
    ui->disconnectButton->clearFocus();
}

void SerialMonitor::sendData()
{
    if ( serial == nullptr || !serial->isOpen() )
    {
        QMessageBox::critical(this, tr("First connect to serial."), tr("First connect to serial"), QMessageBox::Ok);
        return;
    }
    QString str;
    switch ( ui->lineTermination->currentData().toInt() )
    {
    case LINE_TERMINATION::NONE :
        serial->write(ui->serialSendMessage->text().toLatin1());
        break;
    case LINE_TERMINATION::LF :
        str = ui->serialSendMessage->text();
        str.append('\n');
        serial->write(str.toLatin1());
        break;
    case LINE_TERMINATION::CR :
        str = ui->serialSendMessage->text();
        str.append('\r');
        serial->write(str.toLatin1());
        break;
    case LINE_TERMINATION::LF_CR :
        str = ui->serialSendMessage->text();
        str.append("\n\r");
        serial->write(str.toLatin1());
        break;
    default:
        serial->write(ui->serialSendMessage->text().toLatin1());
    }
    serial->flush();
    ui->sendButton->clearFocus();
}

void SerialMonitor::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        disconnectSerial();
    }
}

void SerialMonitor::readData()
{
    QString readData = QString::fromStdString(serial->readAll().toStdString());
    ui->receiveTexts->moveCursor(QTextCursor::End);
    ui->receiveTexts->insertPlainText(readData);
    serial->flush();
    QMainWindow::repaint();
}

void SerialMonitor::clearReceive()
{
    ui->receiveTexts->clear();
    ui->clearButton->clearFocus();
    QMainWindow::repaint();
}

void SerialMonitor::identifyPorts()
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
        ui->receiveTexts->moveCursor(QTextCursor::End);
        ui->receiveTexts->insertPlainText(infoData);
        ui->comPorts->addItem(info.portName());
    }
    ui->comListButton->clearFocus();
    QMainWindow::repaint();
}

void SerialMonitor::changedBoudRate(int index)
{
    if ( serial != nullptr && serial->isOpen() )
    {
        serial->setBaudRate(ui->boudRate->currentText().toInt());
    }
}
