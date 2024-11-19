#ifndef CALCULATEWIDTH_H
#define CALCULATEWIDTH_H

#include <QWidget>

namespace Ui {
class CalculateWidth;
}

class CalculateWidth : public QWidget
{
    Q_OBJECT

public:
    explicit CalculateWidth(QWidget *parent = nullptr);
    ~CalculateWidth();

private slots:
    void on_checkBox_checkStateChanged(const Qt::CheckState &arg1);
    void on_horizontalSlider_sliderMoved(int position);
    void on_pushButton_7_clicked();
    void on_pushButton_6_clicked();
    void on_checkBox_2_checkStateChanged(const Qt::CheckState &arg1);

private:
    Ui::CalculateWidth *ui;
};

#endif // CALCULATEWIDTH_H
