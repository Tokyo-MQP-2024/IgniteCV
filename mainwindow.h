#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QPixmap>
#include <QCloseEvent>
#include <QMessageBox>

// For file dialog
#include <QFileDialog>

#include "opencv2/opencv.hpp"
#include "FlameProcessing.h"
#include "calculateWidth.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void startBtnPressed();
    void on_actionOpen_triggered();
    void on_actionHome_triggered();

    void on_backgroundRemoval_clicked();

    void on_image1_clicked();

    void on_image2_clicked();

    void on_run_clicked();

    void on_pushButton_2_clicked();

    void on_averageImages_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    //void on_pushButton_6_clicked();

    //void on_pushButton_7_clicked();

    //void on_horizontalSlider_sliderMoved(int position);

    void on_FlameToolButton_clicked();

    void on_ProcessVideoButton_clicked();

    void on_VideoSelectButton_clicked();

    void on_CancelButton_clicked();

    void on_VideoView_rubberBandChanged(const QRect &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint);

    //void on_checkBox_checkStateChanged(const Qt::CheckState &arg1);
private:
    Ui::MainWindow *ui;
    CalculateWidth *calculateWidth;
    QGraphicsPixmapItem pixmap;
    cv::VideoCapture video;
    QString videoFilePath;
    FlameProcessing *flame_process;
};
#endif // MAINWINDOW_H
