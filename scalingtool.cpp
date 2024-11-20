#include "scalingtool.h"
#include "ui_scalingtool.h"
#include "mainwindow.h"

ScalingTool::ScalingTool(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScalingTool)
{
    ui->setupUi(this);
}

ScalingTool::~ScalingTool()
{
    delete ui;
}

void ScalingTool::on_manualButton_clicked()
{
    // int index = ui->stackedWidget->indexOf(scalingTool);
    // ui->stackedWidget->setCurrentIndex(index);
    ui->stackedWidget->setCurrentIndex(0);
}


void ScalingTool::on_AutoDetectButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}


void ScalingTool::on_pushButton_clicked()
{
    flame_process->imageScaling(videoFilePath);



}

