#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addItem(&pixmap);
    ui->graphicsView->fitInView(&pixmap, Qt::KeepAspectRatioByExpanding);

    connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(startBtnPressed()));

    // // read an image
    // cv::Mat image = cv::imread("image.png", 1);
    // // create image window named "My Image"
    // cv::namedWindow("My Image");
    // // show the image on window
    // cv::imshow("My Image", image);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startBtnPressed() {
    using namespace cv;
    if(video.isOpened()) {
        ui->pushButton->setText("Start");
        video.release();
        return;
    }

    bool isCamera;
    int cameraIndex = ui->lineEdit->text().toInt(&isCamera);

    // Maybe implement camera later
    if(isCamera) {
        QMessageBox::critical(this, "NO CAMERA YET", "CRY");
        return;
    }

    if(!video.open(ui->lineEdit->text().trimmed().toStdString())) {
        QMessageBox::critical(this, "Video Error", "Cannot find file path");
        return;
    }

    ui->pushButton->setText("Stop");

    Mat frame;
    while(video.isOpened()) {
        video >> frame;
        if(!frame.empty()) {
            copyMakeBorder(frame,
                           frame,
                           frame.rows/2,
                           frame.rows/2,
                           frame.cols/2,
                           frame.cols/2,
                           BORDER_REFLECT);

            QImage qimg(frame.data,
                        frame.cols,
                        frame.rows,
                        frame.step,
                        QImage::Format_RGB888);
            pixmap.setPixmap( QPixmap::fromImage(qimg.rgbSwapped()) );
            ui->graphicsView->fitInView(&pixmap, Qt::KeepAspectRatio);
        }
        qApp->processEvents();
    }
    ui->pushButton->setText("Start");
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if(video.isOpened()) {
        QMessageBox::warning(this, "Warning", "Stop the video before closing the application!");
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::on_actionOpen_triggered()
{
    // Create file dialog
    QString fileName = QFileDialog::getOpenFileName(this, "Open A File", "C://");
    ui->lineEdit->setText(fileName);
}

