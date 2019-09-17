#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    a.installNativeEventFilter(&w);
    w.show();

    return a.exec();
}
