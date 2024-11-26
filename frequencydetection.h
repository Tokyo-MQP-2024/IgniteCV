#ifndef FREQUENCYDETECTION_H
#define FREQUENCYDETECTION_H

#include <QProgressDialog>
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
    void on_pushButtonSet_clicked();
    void on_thresholdSlider_valueChanged(int value);
    void on_runButton_clicked();

    void on_horizontalSliderUpperLimit_valueChanged(int value);

    void on_horizontalSliderLowerLimit_valueChanged(int value);

    void on_checkBoxApplyLimits_checkStateChanged(const Qt::CheckState &arg1);

private:
    Ui::frequencyDetection *ui;
    QProgressDialog *progressDialog = nullptr;
    void changeEvent(QEvent* event);

    // MEMBERS

    // Stores m_areas of bottom region over time
    std::vector<double> m_areas;

    // Stores number of frames (images) in folder
    int m_numFrames;

    // Store list of image files in folder
    QStringList m_imageFiles;

    // Upper and lower bounds for image processing
    int m_upper;
    int m_lower;

    QString m_fileName;

    // Store width and height of first image to check for discrepancies
    int m_width;
    int m_height;

    //FUNCTIONS

    // Refresh image in QGraphicsView
    void refreshImage();

    // Applies threshold based on settings from UI
    void applyThreshold(cv::Mat &image);

    // Save areas to CSV
    void exportAreasToCSV(const std::vector<double> &areas);

    // Overlay bound lines on image
    void applyLines(cv::Mat &image);

    double countWhitePixels(const QString &item);

    // For use in QtConcurrent
    void handleComputationCompletion();




};

#endif // FREQUENCYDETECTION_H
