#include "calculateWidth.h"
#include "ui_calculateWidth.h"
#include "utils.h"

#include <QFileDialog>
#include <QMessageBox>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>


CalculateWidth::CalculateWidth(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CalculateWidth)
{
    ui->setupUi(this);

    // Explicit signal and slot connections
    //connect(ui->pushButton_6, &QPushButton::clicked, this, &CalculateWidth::on_pushButton_6_clicked);
}

CalculateWidth::~CalculateWidth()
{
    delete ui;
}

// Open Image for width calculation
void CalculateWidth::on_pushButton_6_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open A File", "C://");
    ui->label_5->setText(fileName);
    cv::Mat image = cv::imread(fileName.toStdString());
    // Display to graphics window
    QImage toDisplay = matToQImage(image);
    ui->graphicsView_2->setScene(new QGraphicsScene(this));
    ui->graphicsView_2->scene()->addPixmap(QPixmap::fromImage(toDisplay));
    ui->graphicsView_2->fitInView(ui->graphicsView_2->scene()->sceneRect(), Qt::KeepAspectRatioByExpanding);
}

// Run width calculation on image
void CalculateWidth::on_pushButton_7_clicked() {
    QString fileName = ui->label_5->text();
    cv::Mat image = cv::imread(fileName.toStdString());

    // If ROI selected
    if(ui->checkBox_2->isChecked()) {
        // If manual mode is checked
        cv::Rect roi;
        if(ui->checkBox->isChecked()) {
            roi.x = ui->spinBox->value();
            roi.y = ui->spinBox_2->value();
            roi.width = ui->spinBox_3->value();
            roi.height = ui->spinBox_4->value();
        } else {
            roi = cv::selectROI(image);
        }
        cv::Mat imCropped;
        try {
            imCropped = image(roi);
        } catch (const cv::Exception &e) {
            QMessageBox::warning(this, tr("OpenCV Error"), QString::fromStdString(e.what()));
            return;
        }

        //imshow("ROI", imCropped);
        //cv::waitKey(0);
        image = imCropped;

        // Populate number boxes
        ui->spinBox->setValue(roi.x);
        ui->spinBox_2->setValue(roi.y);
        ui->spinBox_3->setValue(roi.width);
        ui->spinBox_4->setValue(roi.height);
    }

    // Apply threshold from slider
    int sliderValue = ui->horizontalSlider->value();
    int threshType = 0;

    // Manually check radio buttons
    if(ui->radioButton_binary->isChecked()) {
        threshType = 0;
    } else if(ui->radioButton_bininverted->isChecked()) {
        threshType = 1;
    } else if(ui->radioButton_threshtrunc->isChecked()) {
        threshType = 2;
    } else if(ui->radioButton_thresh0->isChecked()) {
        threshType = 3;
    } else if(ui->radioButton_thresh0inv->isChecked()) {
        threshType = 4;
    } else {
        qErrnoWarning("ERROR: TYPE NOT DEFINED");
    }

    cv::threshold(image, image, sliderValue, 255, threshType);

    //QImage screen(ui->graphicsView_2->viewport()->size(), QImage::Format_RGB32);
    //QPainter painter(&screen);
    //ui->graphicsView_2->render(&painter);
    //cv::Mat image = QImageToCvMat(screen);

    // Modify image in place
    imageWidthOverlay(image);
    //cv::Rect2d r = cv::selectROI(image);
    //cv::Mat imCrop = image(r);
    //cv::imshow("Result", imCrop);
    cv::imshow("Result", image);

    // Display to graphics window
    // QImage toDisplay = matToQImage(image);
    // ui->graphicsView_2->setScene(new QGraphicsScene(this));
    // ui->graphicsView_2->scene()->addPixmap(QPixmap::fromImage(toDisplay));
    // ui->graphicsView_2->fitInView(ui->graphicsView_2->scene()->sceneRect(), Qt::KeepAspectRatio);

}

// When slider changes, edit image in display with appropriate thresholding
void CalculateWidth::on_horizontalSlider_sliderMoved(int position) {
    QString label = ui->label_5->text();
    if(label == "")
        return;
    cv::Mat image = cv::imread(label.toStdString());
    cv::Mat grey, output;
    // Convert to grey
    cv::cvtColor(image, grey, cv::COLOR_BGR2GRAY);
    int threshType = 0;

    // Manually check radio buttons
    if(ui->radioButton_binary->isChecked()) {
        threshType = 0;
    } else if(ui->radioButton_bininverted->isChecked()) {
        threshType = 1;
    } else if(ui->radioButton_threshtrunc->isChecked()) {
        threshType = 2;
    } else if(ui->radioButton_thresh0->isChecked()) {
        threshType = 3;
    } else if(ui->radioButton_thresh0inv->isChecked()) {
        threshType = 4;
    } else {
        qErrnoWarning("ERROR: TYPE NOT DEFINED");
    }

    // Threshold type hardcoded for now
    cv::threshold(grey, output, position, 255, threshType);
    QImage toDisplay = matToQImage(output);
    ui->graphicsView_2->scene()->clear();
    ui->graphicsView_2->scene()->addPixmap(QPixmap::fromImage(toDisplay));
    ui->graphicsView_2->fitInView(ui->graphicsView_2->scene()->sceneRect(), Qt::KeepAspectRatio);

}

// Checkbox for ROI
void CalculateWidth::on_checkBox_checkStateChanged(const Qt::CheckState &arg1) {
    if(arg1 == Qt::Checked) {
        ui->spinBox->setEnabled(true);
        ui->spinBox_2->setEnabled(true);
        ui->spinBox_3->setEnabled(true);
        ui->spinBox_4->setEnabled(true);
    } else {
        ui->spinBox->setEnabled(false);
        ui->spinBox_2->setEnabled(false);
        ui->spinBox_3->setEnabled(false);
        ui->spinBox_4->setEnabled(false);
    }
}

void CalculateWidth::on_checkBox_2_checkStateChanged(const Qt::CheckState &arg1) {
    if(arg1 == Qt::Checked) {
        ui->checkBox->setEnabled(true);
    } else {
        ui->checkBox->setCheckState(Qt::Unchecked);
        ui->checkBox->setEnabled(false);
    }
}

