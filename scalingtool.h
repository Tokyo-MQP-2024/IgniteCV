#ifndef SCALINGTOOL_H
#define SCALINGTOOL_H

#include <QWidget>

namespace Ui {
class ScalingTool;
}

class ScalingTool : public QWidget
{
    Q_OBJECT

public:
    explicit ScalingTool(QWidget *parent = nullptr);
    ~ScalingTool();

private:
    Ui::ScalingTool *ui;
};

#endif // SCALINGTOOL_H
