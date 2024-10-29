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

private:
    Ui::MainWindow *ui;
    QGraphicsPixmapItem pixmap;
    cv::VideoCapture video;
};
#endif // MAINWINDOW_H
