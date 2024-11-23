#include "frequencydetection.h"
#include "ui_frequencydetection.h"

#include <QFileDialog>
#include <QGraphicsScene>
#include <QMessageBox>

FrequencyDetection::FrequencyDetection(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::frequencyDetection) {
    ui->setupUi(this);
    numFrames = 0;
}

FrequencyDetection::~FrequencyDetection() {
    delete ui;
}

// Select Folder
void FrequencyDetection::on_pushButton_clicked() {
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Open a Folder"));
    ui->lineEdit->setText(folderPath);
}

// Slider for frame selection
void FrequencyDetection::on_horizontalSlider_valueChanged(int value) {
    if(numFrames <= 0) {
        return;
    }

    // Assumes value is in range
    QImage toDisplay(ui->lineEdit->text() + "/" + imageFiles[value - 1]);
    ui->spinBox->setValue(value);

    ui->graphicsView->scene()->clear();
    ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(toDisplay));
    ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
}

// Set button (set image and display first frame
void FrequencyDetection::on_pushButton_2_clicked() {
    QString folderPath = ui->lineEdit->text();
    if(folderPath == "") {
        QMessageBox::warning(this, tr("Warning"), tr("Invalid file path. Please try again"));
        return;
    }
    QDir dir(folderPath);

    // Return if folder is invalid
    if(!dir.exists()) {
        QMessageBox::warning(this, tr("Warning"), tr("Invalid file path. Please try again"));
        return;
    }

    // Filters to only include image files
    QStringList filters;
    filters << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp";
    dir.setNameFilters(filters);

    // Get the list of image files
    imageFiles = dir.entryList(QDir::Files);

    // Error if no images
    if(imageFiles.size() < 1) {
        QMessageBox::warning(this, tr("Warning"), tr("No images in folder. Please select another folder"));
        return;
    }

    numFrames = imageFiles.size();

    // Set trackbar and numbox max to number of images
    ui->horizontalSlider->setMaximum(numFrames);
    ui->spinBox->setMaximum(numFrames);


    // Load first image
    QImage toDisplay = QImage(folderPath + "/" + imageFiles[0]);

    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(toDisplay));
    ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
}

// Spinbox next to frame scroll bar
void FrequencyDetection::on_spinBox_valueChanged(int arg1) {
    ui->horizontalSlider->setValue(arg1);
}

