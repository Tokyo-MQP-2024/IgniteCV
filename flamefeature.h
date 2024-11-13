#ifndef FLAMEFEATURE_H
#define FLAMEFEATURE_H

#include <QDialog>

#include "mainwindow.h"

namespace Ui {
class FlameFeature;
}

class FlameFeature : public QDialog
{
    Q_OBJECT

public:
    explicit FlameFeature(MainWindow *mainWindow, QWidget *parent = nullptr);
    ~FlameFeature();

private slots:
    void on_pushButton_clicked();

    void on_toolButton_clicked();

private:
    Ui::FlameFeature *ui;
    MainWindow *m_mainWindow;
};

#endif // FLAMEFEATURE_H
