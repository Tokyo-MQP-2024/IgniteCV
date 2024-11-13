#include "flamefeature.h"
#include "ui_flamefeature.h"
#include "FlameProcessing.h"

FlameFeature::FlameFeature(MainWindow *mainWindow, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FlameFeature)
    , m_mainWindow(mainWindow) // Initialize m_mainWindow
{
    ui->setupUi(this);
}

FlameFeature::~FlameFeature()
{
    delete ui;
}

void FlameFeature::on_pushButton_clicked()
{

    this->hide();
    if (m_mainWindow) {
        m_mainWindow->show(); // Show the main window
    }
    // FlameFeature flamePage;
    // flamePage.setModal(true);
    // flamePage.exec();




}


void FlameFeature::on_toolButton_clicked()
{
    parseVideo();
}

