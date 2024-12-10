#ifndef SCALINGTOOL_H
#define SCALINGTOOL_H

#include <QWidget>
//#include "qcustomplot.h"

#include "utils.h"

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

#include <QTimer>


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



    void on_scaleX_textEdited(const QString &arg1);

    void on_comboBox_currentIndexChanged(int index);

    void on_ROIButton_clicked();

    void on_scaleYButton_clicked();

    void on_ScaleYEdit_textEdited(const QString &arg1);

    void on_ScaleXEdit_editingFinished();

    void on_ScaleYEdit_editingFinished();

    void on_BeginButton_clicked();

    void on_manualButton2_clicked();

    void on_AutoButton2_clicked();

    void on_pushButton_2_clicked();

    void on_VideoScroll_actionTriggered(int action);

    void on_VideoScroll_valueChanged(int value);

    void on_pushButton_7_clicked();

    void on_pushButton_4_clicked();

    void imageROISelect(std::string videoFilePath);

    void applyROIMask(cv::Mat &frame);

    void adjustLevels(cv::Mat image);

    void on_BlackSlider_valueChanged(int value);

    void on_WhiteSlider_valueChanged(int value);

    //void setupHistogram();

    void on_MinRadSlider_valueChanged(int value);

    void on_MaxRadSlider_valueChanged(int value);

    void on_CannySlider_valueChanged(int value);

    void on_Accumulator_valueChanged(int value);



    void on_HSVbutton_clicked();




    void on_HMin_valueChanged(int arg1);

    void on_HMax_valueChanged(int arg1);

    void on_SMin_valueChanged(int arg1);

    void on_SMax_valueChanged(int arg1);

    void on_VMin_valueChanged(int arg1);

    void on_VMax_valueChanged(int arg1);

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    //void writePixelDataToCSV(const std::vector<std::vector<double>>& totalPixelData, const std::string& filePath);



    void on_SavePosDataButton_clicked();



    void on_SegmentSpinBox_valueChanged(int arg1);

    void on_TrackAnglesButton_clicked();

    void on_AngleSaveButton_clicked();

    void on_AngleThresholdWheel_valueChanged(int arg1);

private:
    Ui::ScalingTool *ui;

    double manual_scaleX;
    double manual_scaleY;

    bool scaleXDone = false;
    bool scaleYDone = false;
    bool editXDone = false;
    bool editYDone = false;
    bool stopProcess = false;

    cv::VideoCapture globalCap;
    QGraphicsScene *scene = new QGraphicsScene();
    // QTimer *timer;
    QThread *workerThread = nullptr;



    cv::Mat currSelectFrame;
    cv::Mat croppedFrame;

    int maskX = -1;
    int maskY = -1;
    int maskH = -1;
    int maskW = -1;

    bool roiSelected = false;

    int clipBlack = 0;
    int clipWhite = 255;

    cv::Mat levelsIMG;

    int minRad = 7;
    int maxRad = 11;
    int canny = 10;
    int accumulator = 5;


    int minHue = 0, maxHue = 50;
    int minSat = 50, maxSat = 255;
    int minVal =  150, maxVal = 255;


    std::vector<double> dataSegments;
    std::vector<std::vector<double>> totalPosData;
    std::vector<std::vector<double>> totalAngleData;

    int segmentLines;

    int angleThreshold;







};

#endif // SCALINGTOOL_H
