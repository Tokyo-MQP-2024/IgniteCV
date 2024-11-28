#include "calculateWidth.h"
#include "ui_calculateWidth.h"
#include "utils.h"

#include <QFileDialog>
#include <QImageReader>
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
    connect(ui->buttonGroup_threshtype, &QButtonGroup::buttonClicked, this, &CalculateWidth::refreshImage);
}

CalculateWidth::~CalculateWidth()
{
    delete ui;
}

// For retranslating
void CalculateWidth::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        // Update the UI elements to the new language
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

// Open Image for width calculation
void CalculateWidth::on_pushButton_6_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open A File", "C://");
    ui->label_5->setText(fileName);
    //cv::Mat image = cv::imread(fileName.toStdString());

    if(ui->label_5->text() == "") {
        return;
    }

    // Assumes value is in range
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        // Show a simple error message box
        QMessageBox::critical(nullptr, tr("Error"), tr("Failed to open file: ") + file.errorString());
        ui->label_5->setText("");
        return; // Exit the function
    }
    // Use QImageReader to check if the file is a valid image
    QImageReader reader(fileName);
    if (!reader.canRead()) {
        QMessageBox::critical(nullptr, tr("Error"), tr("The file is not a valid image: ") + reader.errorString());
        ui->label_5->setText("");
        return;
    }

    qint64 sz = file.size();
    std::vector<uchar> buf(sz);
    file.read((char*) buf.data(), sz);


    cv::Mat image = cv::imdecode(buf, cv::IMREAD_COLOR);


    // Display to graphics window
    QImage toDisplay = matToQImage(image);
    ui->graphicsView_2->setScene(new QGraphicsScene(this));
    ui->graphicsView_2->scene()->addPixmap(QPixmap::fromImage(toDisplay));
    ui->graphicsView_2->fitInView(ui->graphicsView_2->scene()->sceneRect(), Qt::KeepAspectRatioByExpanding);
}

// Run width calculation on image
void CalculateWidth::on_pushButton_7_clicked() {
    QString fileName = ui->label_5->text();



    // Assumes value is in range
    QFile file(fileName);
    // Attempt to open the file
    if (!file.open(QFile::ReadOnly)) {
        // Show a simple error message box
        QMessageBox::critical(nullptr, tr("Error"), tr("Failed to open file: ") + file.errorString());
        return; // Exit the function
    }
    qint64 sz = file.size();
    std::vector<uchar> buf(sz);
    file.read((char*) buf.data(), sz);


    cv::Mat image = cv::imdecode(buf, cv::IMREAD_COLOR);

    //cv::Mat image = cv::imread(fileName.toStdString());
    if (image.empty()) {
        // Error handling
        QMessageBox::critical(nullptr, tr("Error"), tr("Failed to load the image. Please check the file path and format."));
        return;
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
            QMessageBox::information(this, tr("Information"), tr("Select a ROI and then press SPACE or ENTER button. Cancel the selection process by pressing c button and closing the ROI window."));
            roi = cv::selectROI(image);
        }

        // If ROI is empty, return
        if(roi.width == 0 || roi.height == 0) {
            return;
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

    // Modify image in place
    imageWidthOverlay(image);
    cv::imshow("Result", image);
}

// Helper function to refresh image on certain UI actions
void CalculateWidth::refreshImage() {
    QString label = ui->label_5->text();
    if(label == "")
        return;

    // Assumes value is in range
    QFile file(label);
    if (!file.open(QFile::ReadOnly)) {
        // Show a simple error message box
        QMessageBox::critical(nullptr, tr("Error"), tr("Failed to open file: ") + file.errorString());
        return; // Exit the function
    }
    qint64 sz = file.size();
    std::vector<uchar> buf(sz);
    file.read((char*) buf.data(), sz);

    cv::Mat image = cv::imdecode(buf, cv::IMREAD_COLOR);

    //cv::Mat image = cv::imread(label.toStdString());
    cv::Mat grey, output;
    // Convert to grey
    cv::cvtColor(image, grey, cv::COLOR_BGR2GRAY);
    int threshType = -1;

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
    } else if(ui->radioButton_none->isChecked()) {
        threshType = -1;
    } else {
        qErrnoWarning("ERROR: TYPE NOT DEFINED");
    }

    if(threshType != -1) {
        cv::threshold(grey, output, ui->horizontalSlider->value(), 255, threshType);
    } else {
        output = grey;
    }
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

// When slider changes, edit image in display with appropriate thresholding
void CalculateWidth::on_horizontalSlider_valueChanged(int value) {
    refreshImage();
}


