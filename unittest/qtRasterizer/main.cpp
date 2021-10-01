#include "mainwindow.h"
#include <QApplication>

double sc_time_stamp() {
    return 0;
}


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
