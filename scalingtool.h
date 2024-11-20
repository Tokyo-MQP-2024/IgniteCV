#ifndef SCALINGTOOL_H
#define SCALINGTOOL_H

#include <QWidget>
#include "FlameProcessing.h"


//#include "mainwindow.h"
//#include "FlameProcessing.h"


namespace Ui {
class ScalingTool;
}

class ScalingTool : public QWidget
{
    Q_OBJECT

public:
    explicit ScalingTool(QWidget *parent = nullptr);
    ~ScalingTool();
    std::string videoFilePath;
    FlameProcessing *flame_process;

private slots:
    void on_manualButton_clicked();

    void on_AutoDetectButton_clicked();

    void on_pushButton_clicked();

private:
    Ui::ScalingTool *ui;



};

#endif // SCALINGTOOL_H
