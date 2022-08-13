#include "mainwindow.h"
#include <QApplication>

double sc_time_stamp() {
    return 0;
}


int main(int argc, char *argv[])
{
    static QApplication a(argc, argv);
    static MainWindow w;
    w.show();

    return a.exec();
}
