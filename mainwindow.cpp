#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "imagesubtraction.h"
#include <QFuture>
#include <QThreadPool>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <qtconcurrentrun.h>
#include <qtranslator.h>
#include <QtUiTools/QUiLoader>
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
    ui->ProcessVideoButton->setEnabled(false);

    // Test separate ui file
    QFile file("../../calculateWidth.ui");
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << "Failed to open file: " << file.errorString();
        return;
    }
    QUiLoader loader;
    QWidget *page = loader.load(&file, this); // Pass the QFile to load()
    file.close();

    if (!page) {
        qDebug() << "Failed to load page: " << loader.errorString();
        return;
    }

    ui->stackedWidget->addWidget(page);

    int index = ui->stackedWidget->indexOf(page);
    ui->stackedWidget->setCurrentIndex(index);

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

void MainWindow::on_FlameToolButton_clicked() {
    ui->ProcessVideoButton->setEnabled(false);
    ui->stackedWidget->setCurrentIndex(5);
}

// BACKGROUND REMOVAL FUNCTIONS

void MainWindow::on_image1_clicked() {
    if(ui->radioButton->isChecked()) {
        //QMessageBox::information(this, tr("Info"), tr("File"));
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open A File"), "C://");
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
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open A File"), "C://");
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



// // Open Image for width calculation
// void MainWindow::on_pushButton_6_clicked() {
//     QString fileName = QFileDialog::getOpenFileName(this, "Open A File", "C://");
//     ui->label_5->setText(fileName);
//     cv::Mat image = cv::imread(fileName.toStdString());
//     // Display to graphics window
//     QImage toDisplay = matToQImage(image);
//     ui->graphicsView_2->setScene(new QGraphicsScene(this));
//     ui->graphicsView_2->scene()->addPixmap(QPixmap::fromImage(toDisplay));
//     ui->graphicsView_2->fitInView(ui->graphicsView_2->scene()->sceneRect(), Qt::KeepAspectRatioByExpanding);
// }

// // Run width calculation on image
// void MainWindow::on_pushButton_7_clicked() {
//     QString fileName = ui->label_5->text();
//     cv::Mat image = cv::imread(fileName.toStdString());

//     // Apply threshold from slider
//     int sliderValue = ui->horizontalSlider->value();
//     int threshType = 0;

//     // Manually check radio buttons
//     if(ui->radioButton_binary->isChecked()) {
//         threshType = 0;
//     } else if(ui->radioButton_bininverted->isChecked()) {
//         threshType = 1;
//     } else if(ui->radioButton_threshtrunc->isChecked()) {
//         threshType = 2;
//     } else if(ui->radioButton_thresh0->isChecked()) {
//         threshType = 3;
//     } else if(ui->radioButton_thresh0inv->isChecked()) {
//         threshType = 4;
//     } else {
//         qErrnoWarning("ERROR: TYPE NOT DEFINED");
//     }

//     cv::threshold(image, image, sliderValue, 255, threshType);

//     //QImage screen(ui->graphicsView_2->viewport()->size(), QImage::Format_RGB32);
//     //QPainter painter(&screen);
//     //ui->graphicsView_2->render(&painter);
//     //cv::Mat image = QImageToCvMat(screen);

//     // Modify image in place
//     imageWidthOverlay(image);
//     //cv::Rect2d r = cv::selectROI(image);
//     //cv::Mat imCrop = image(r);
//     //cv::imshow("Result", imCrop);
//     cv::imshow("Result", image);

//     // Display to graphics window
//     // QImage toDisplay = matToQImage(image);
//     // ui->graphicsView_2->setScene(new QGraphicsScene(this));
//     // ui->graphicsView_2->scene()->addPixmap(QPixmap::fromImage(toDisplay));
//     // ui->graphicsView_2->fitInView(ui->graphicsView_2->scene()->sceneRect(), Qt::KeepAspectRatio);

// }

// // When slider changes, edit image in display with appropriate thresholding
// void MainWindow::on_horizontalSlider_sliderMoved(int position) {
//     QString label = ui->label_5->text();
//     if(label == "")
//         return;
//     cv::Mat image = cv::imread(label.toStdString());
//     cv::Mat grey, output;
//     // Convert to grey
//     cv::cvtColor(image, grey, cv::COLOR_BGR2GRAY);
//     int threshType = 0;

//     // Manually check radio buttons
//     if(ui->radioButton_binary->isChecked()) {
//         threshType = 0;
//     } else if(ui->radioButton_bininverted->isChecked()) {
//         threshType = 1;
//     } else if(ui->radioButton_threshtrunc->isChecked()) {
//         threshType = 2;
//     } else if(ui->radioButton_thresh0->isChecked()) {
//         threshType = 3;
//     } else if(ui->radioButton_thresh0inv->isChecked()) {
//         threshType = 4;
//     } else {
//         qErrnoWarning("ERROR: TYPE NOT DEFINED");
//     }

//     // Threshold type hardcoded for now
//     cv::threshold(grey, output, position, 255, threshType);
//     QImage toDisplay = matToQImage(output);
//     ui->graphicsView_2->scene()->clear();
//     ui->graphicsView_2->scene()->addPixmap(QPixmap::fromImage(toDisplay));
//     ui->graphicsView_2->fitInView(ui->graphicsView_2->scene()->sceneRect(), Qt::KeepAspectRatio);

// }

// button for running video processing program
void MainWindow::on_ProcessVideoButton_clicked()
{
    flame_process->parseVideo(videoFilePath.toStdString(), ui->VideoView);
}


// button to uplaod video for processing
void MainWindow::on_VideoSelectButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open a File"), "C://");
    std::string filePathSTD = filePath.toStdString();
    flame_process = new FlameProcessing(); // create new instance of flame process
    if(flame_process->checkMP4(filePathSTD)) {
        ui->VideoLabel->setText(filePath);
        ui->ProcessVideoButton->setEnabled(true);
        // TODO: display video on the graphics viewer
        // TODO: pass the file into parseVideo
        videoFilePath = filePath;
    } else {
        ui->VideoLabel->setText("File must be .mp4");
    }
}

// // Checkbox for ROI
// void MainWindow::on_checkBox_checkStateChanged(const Qt::CheckState &arg1) {
//     if(arg1 == Qt::Checked) {
//         for(int i = 0; i < ui->horizontalLayout_roi->count(); ++i) {
//             QWidget* widget = ui->horizontalLayout_roi->itemAt(i)->widget();
//             if(QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget)) {
//                 spinBox->setEnabled(true);
//             }
//         }
//     } else {
//         for(int i = 0; i < ui->horizontalLayout_roi->count(); ++i) {
//             QWidget* widget = ui->horizontalLayout_roi->itemAt(i)->widget();
//             if(QSpinBox* spinBox = qobject_cast<QSpinBox*>(widget)) {
//                 spinBox->setEnabled(false);
//             }
//         }
//     }
// }


void MainWindow::on_CancelButton_clicked()
{
    //TODO: stop button for processing
    if(flame_process != nullptr) {
        flame_process->setStopProcess(true);
    }
}


void MainWindow::on_VideoView_rubberBandChanged(const QRect &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint)
{

}


