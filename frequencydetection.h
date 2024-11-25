#ifndef FREQUENCYDETECTION_H
#define FREQUENCYDETECTION_H

#include <QWidget>
#include <opencv2/core.hpp>

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
    void on_thresholdSlider_valueChanged(int value);
    void on_runButton_clicked();

    void on_horizontalSlider_2_valueChanged(int value);

    void on_horizontalSlider_3_valueChanged(int value);

    void on_checkBox_2_checkStateChanged(const Qt::CheckState &arg1);

private:
    Ui::frequencyDetection *ui;

    // MEMBERS
    // Stores areas of bottom region over time
    std::vector<double> areas;

    // Stores number of frames (images) in folder
    int numFrames;

    // Store list of image files in folder
    QStringList imageFiles;

    // Upper and lower bounds for image processing
    int upper;
    int lower;


    //FUNCTIONS
    // Processes image. [TO BE REMOVED]
    cv::Mat processImage(cv::Mat &image);

    // Applies threshold based on settings from UI
    void applyThreshold(cv::Mat &image);

    // Save areas to CSV
    void exportAreasToCSV(const std::vector<double> &areas);

    // Overlay bound lines on image
    void applyLines(cv::Mat &image);

    double countWhitePixels(const QString &item);




};

#endif // FREQUENCYDETECTION_H
