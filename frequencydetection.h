#ifndef FREQUENCYDETECTION_H
#define FREQUENCYDETECTION_H

#include <QWidget>

namespace Ui {
class frequencyDetection;
}

class FrequencyDetection : public QWidget
{
    Q_OBJECT

public:
    explicit FrequencyDetection(QWidget *parent = nullptr);
    ~FrequencyDetection();

private slots:
    void on_pushButton_clicked();

    void on_horizontalSlider_valueChanged(int value);

    void on_pushButton_2_clicked();

    void on_spinBox_valueChanged(int arg1);

private:
    Ui::frequencyDetection *ui;

    // Stores number of frames (images) in folder
    int numFrames;

    // Store list of image files in folder
    QStringList imageFiles;

};

#endif // FREQUENCYDETECTION_H
