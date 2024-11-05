#include "mainwindow.h"
#include "utils.h"

#include <QApplication>
#include <QDir>
#include <QTranslator>

#include <QPushButton>

int main(int argc, char *argv[])
{
    // Sets the current directory to make resources easier to import
    QDir::setCurrent("../..");
    QApplication a(argc, argv);
    QTranslator translator;
    Q_UNUSED(translator.load(":/OpenCV_Integration_en_US"));
    a.installTranslator(&translator);

    // For translating
    QObject::tr("This is a test");

    MainWindow w;
    w.show();
    return a.exec();
}
