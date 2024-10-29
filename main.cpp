#include "mainwindow.h"

#include <QApplication>
#include <QDir>

int main(int argc, char *argv[])
{
    // Sets the current directory to make resources easier to import
    QDir::setCurrent("../..");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
