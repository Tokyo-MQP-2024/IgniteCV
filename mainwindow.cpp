#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "imagesubtraction.h"

#include <QFuture>
#include <QThreadPool>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <qtconcurrentrun.h>
#include <qtranslator.h>
#include "utils.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addItem(&pixmap);
    ui->graphicsView->fitInView(&pixmap, Qt::KeepAspectRatioByExpanding);


    connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(startBtnPressed()));

    // Disable bad buttons
    ui->backgroundRemoval->setDisabled(true);

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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open A File"), "C://");
    ui->lineEdit->setText(fileName);
}


void MainWindow::on_actionHome_triggered() {
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_backgroundRemoval_clicked() {
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_averageImages_clicked() {
    ui->stackedWidget->setCurrentIndex(2);
}

void MainWindow::on_pushButton_5_clicked() {
    ui->stackedWidget->setCurrentIndex(3);
}

// BACKGROUND REMOVAL FUNCTIONS

void MainWindow::on_image1_clicked() {
    if(ui->radioButton->isChecked()) {
        //QMessageBox::information(this, tr("Info"), tr("File"));
        QString fileName = QFileDialog::getOpenFileName(this, "Open A File", "C://");
        ui->label_2->setText(fileName);
    } else if(ui->radioButton_2->isChecked()) {
        QMessageBox::information(this, tr("Info"), tr("Folder"));
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please choose an option"));
    }
}


void MainWindow::on_image2_clicked() {
    if(ui->radioButton_3->isChecked()) {
        //QMessageBox::information(this, tr("Info"), tr("File"));
        QString fileName = QFileDialog::getOpenFileName(this, "Open A File", "C://");
        ui->label_3->setText(fileName);
    } else if(ui->radioButton_4->isChecked()) {
        QMessageBox::information(this, tr("Info"), tr("Auto"));
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Please choose an option"));
    }
}


void MainWindow::on_run_clicked() {
    // NO ERROR CHECKING YET
    //cv::Mat im1 = cv::imread("original.tif");
    //cv::Mat im2 = cv::imread("background.tif");
    std::string image1 = ui->label_2->text().toStdString();
    std::string image2 = ui->label_3->text().toStdString();
    if(image1 == "" || image2 == "") {
        QMessageBox::warning(this, tr("Warning"), tr("Please select images for subtraction"));
        return;
    }

    cv::Mat im1 = cv::imread(image1);
    cv::Mat im2 = cv::imread(image2);

    cv::Mat diff;

    // Multithreading is a bit useless here but I'm planning on extending
    // the ImageSubtraction class to accept videos & batch images
    QMutex mutex;
    ImageSubtraction *subtraction = new ImageSubtraction(im1, im2, &diff, &mutex);
    QThreadPool::globalInstance()->start(subtraction);

    QThreadPool::globalInstance()->waitForDone();
    cv::imshow("My iamg", diff);
}

// IMAGE AVERAGING PAGE FUNCTIONS

void MainWindow::on_pushButton_2_clicked() {
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Open a Folder"), "C://");
    ui->label_4->setText(folderPath);
}

// Average images in selected folder
void MainWindow::on_pushButton_4_clicked() {
    // Helper function from utils
    //cv::Mat averageImage = averageImagesFromFolder(ui->label_4->text());
    ui->pushButton_4->setDisabled(true);
    QFuture<cv::Mat> future = QtConcurrent::run(averageImagesFromFolder, ui->label_4->text());
    cv::Mat averageImage = future.result();
    cv::imshow("test", averageImage);
    ui->pushButton_4->setDisabled(false);
}



// Open Image for width calculation
void MainWindow::on_pushButton_6_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open A File", "C://");
    ui->label_5->setText(fileName);
}

// Run width calculation on image
void MainWindow::on_pushButton_7_clicked() {
    QString fileName = ui->label_5->text();
    cv::Mat image = cv::imread(fileName.toStdString());

    // Modify image in place
    imageWidthOverlay(image);
    cv::imshow("Result", image);

    // Display to graphics window
    // QImage toDisplay = matToQImage(image);
    // ui->graphicsView_2->setScene(new QGraphicsScene(this));
    // ui->graphicsView_2->scene()->addPixmap(QPixmap::fromImage(toDisplay));
    // ui->graphicsView_2->fitInView(ui->graphicsView_2->scene()->sceneRect(), Qt::KeepAspectRatio);




}

