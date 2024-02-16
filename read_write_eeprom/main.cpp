#include "readwriteeeprom.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ReadWriteEEPROM w;
    w.show();

    return a.exec();
}
