#include "frequencydetection.h"
#include "ui_frequencydetection.h"
#include "utils.h"

#include <QFileDialog>
#include <QGraphicsScene>
#include <QMessageBox>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>


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
    cv::Mat image = cv::imread(ui->lineEdit->text().toStdString() + "/" + imageFiles[value - 1].toStdString());
    //cv::Mat image2 = processImage(image);
    cv::Mat image2 = image;
    applyThreshold(image2);
    QImage toDisplay = matToQImage(image2);
    //QImage toDisplay(ui->lineEdit->text() + "/" + imageFiles[value - 1]);
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

// Return edges of image [TODO: Add preprocessing options (there are a lot)]
cv::Mat FrequencyDetection::processImage(cv::Mat &image) {
    // Pre-process
    // Convert to gray
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    // Blur image
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5,5), 0);

    // Get edges [TODO: MAKE CANNY THRESHOLDS MODIFIABLE THROUGH UI]
    cv::Mat edges;
    cv::Canny(blurred, edges, 50, 100);

    return edges;

}

// Applies thresholding based on selected settings in the UI (modifies image in place)
void FrequencyDetection::applyThreshold(cv::Mat &image) {
    int threshType;
    // Manually check radio buttons
    if(ui->radioButton_none->isChecked()) {
        return;
    } else if(ui->radioButton_binary->isChecked()) {
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
        return;
    }
    int thresh = ui->thresholdSlider->value();

    // Note: maxval of 255 assumes bit depth of 8
    cv::threshold(image, image, thresh, 255, threshType);

}



void FrequencyDetection::on_thresholdSlider_valueChanged(int value){
    // Return if no images
    if(numFrames <= 0) {
        return;
    }
    // On change, update image
    // Assumes value is in range
    cv::Mat image = cv::imread(ui->lineEdit->text().toStdString() + "/" + imageFiles[ui->horizontalSlider->value() - 1].toStdString());
    //cv::Mat image2 = processImage(image);
    cv::Mat image2 = image;
    applyThreshold(image2);
    QImage toDisplay = matToQImage(image2);
    ui->graphicsView->scene()->clear();
    ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(toDisplay));
    ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
}


void FrequencyDetection::on_runButton_clicked() {
    // Lock UI elements to prevent errors
    // Clear areas variable
    areas.clear();

    // Loop through frames and count area of white pixels inside ROI
    for(const QString &item : imageFiles) {
        // Open file
        cv::Mat image = cv::imread(ui->lineEdit->text().toStdString() + "/" + item.toStdString());

        // Convert to grayscale
        cv::Mat gray;
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

        // Apply thresholding (should be binary inverted probably)
        cv::Mat binary = gray;
        applyThreshold(binary);

        // Focus on bottom region (make this more intuitive later)
        int height = binary.rows;
        int width = binary.cols;
        cv::Mat bottomRegion = binary(cv::Range(height * 0.7, height), cv::Range(0, width));

        // Count the number of white pixels (area)
        int area = cv::countNonZero(bottomRegion);
        areas.push_back(area);

        // Display the current frame with the highlighted bottom region
        cv::rectangle(image, cv::Point(0, height * 0.7), cv::Point(width, height), cv::Scalar(0, 255, 0), 2);
        cv::imshow("Stream Analysis", image);

        // Press 'q' to exit early
        if (cv::waitKey(10) == 'q') break;
    }

    cv::destroyAllWindows();

    // Compute FFT of area data
    std::vector<double> amplitudeSpectrum;
    computeFFT(areas, amplitudeSpectrum);

    // Find the dominant frequency
    int N = amplitudeSpectrum.size();
    double maxAmplitude = 0.0;
    int dominantIndex = 0;

    for (int i = 1; i < N / 2; ++i) { // Use N/2 because of symmetry in FFT output
        if (amplitudeSpectrum[i] > maxAmplitude) {
            maxAmplitude = amplitudeSpectrum[i];
            dominantIndex = i;
        }
    }

    // Assuming a frame rate of 30 FPS for calculation (adjust as necessary)
    double fps = ui->spinBox_3->value();
    double frequency = (dominantIndex * fps) / N;
    std::cout << "Approximate Oscillation Frequency: " << frequency << " Hz" << std::endl;
}

