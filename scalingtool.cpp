#include "scalingtool.h"
#include "ui_scalingtool.h"

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
