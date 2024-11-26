#include "frequencydetection.h"
#include "ui_frequencydetection.h"
#include "utils.h"

#include <QFileDialog>
#include <QGraphicsScene>
#include <QMessageBox>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <qtconcurrentmap.h>
#include <QFutureWatcher>
#include <QDialog>



FrequencyDetection::FrequencyDetection(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::frequencyDetection) {
    ui->setupUi(this);
    m_numFrames = 0;
}

FrequencyDetection::~FrequencyDetection() {
    delete ui;
}

void FrequencyDetection::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        // Update the UI elements to the new language
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

// Select Folder
void FrequencyDetection::on_pushButton_clicked() {
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Open a Folder"));
    ui->lineEdit->setText(folderPath);
}

// Slider for frame selection
void FrequencyDetection::on_horizontalSlider_valueChanged(int value) {
    if(m_numFrames <= 0) {
        return;
    }

    // Assumes value is in range
    cv::Mat image = cv::imread(ui->lineEdit->text().toStdString() + "/" + m_imageFiles[value - 1].toStdString());
    //cv::Mat image2 = processImage(image);
    cv::Mat image2 = image;
    applyThreshold(image2);
    if(ui->checkBox_2->isChecked())
        applyLines(image2);
    QImage toDisplay = matToQImage(image2);
    //QImage toDisplay(ui->lineEdit->text() + "/" + imageFiles[value - 1]);
    ui->graphicsView->scene()->clear();
    ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(toDisplay));
    ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
}

// Set button (set image and display first frame
void FrequencyDetection::on_pushButtonSet_clicked() {
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
    m_imageFiles = dir.entryList(QDir::Files);

    // Error if no images
    if(m_imageFiles.size() < 1) {
        QMessageBox::warning(this, tr("Warning"), tr("No images in folder. Please select another folder"));
        return;
    }

    m_numFrames = m_imageFiles.size();

    // Set trackbar and numbox max to number of images
    ui->horizontalSlider->setMaximum(m_numFrames);
    ui->spinBox->setMaximum(m_numFrames);



    // Load first image
    QImage toDisplay = QImage(folderPath + "/" + m_imageFiles[0]);

    ui->horizontalSliderLowerLimit->setMaximum(toDisplay.height());
    ui->horizontalSliderUpperLimit->setMaximum(toDisplay.height());
    ui->spinBoxLowerLimit->setMaximum(toDisplay.height());
    ui->spinBoxUpperLimit->setMaximum(toDisplay.height());

    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(toDisplay));
    ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
}

// Return edges of image [Redundant - Switched to thresholding]
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
    } else if(ui->radioButton_bininverted->isChecked()) {
        threshType = 1;
    } else {
        qErrnoWarning("ERROR: TYPE NOT DEFINED");
        return;
    }
    int thresh = ui->thresholdSlider->value();

    // Note: maxval of 255 assumes bit depth of 8
    cv::threshold(image, image, thresh, 255, threshType);
}

// Applies line overlay
void FrequencyDetection::applyLines(cv::Mat &image) {
    // Upper line
    cv::Point start_point(0, m_upper);
    cv::Point end_point(image.cols, m_upper);

    // Line color
    cv::Scalar color(0, 0, 255);

    int thickness = 2;

    cv::line(image, start_point, end_point, color, thickness);

    // Lower line
    start_point.y = m_lower;
    end_point.y = m_lower;

    cv::line(image, start_point, end_point, color, thickness);
}

void FrequencyDetection::on_thresholdSlider_valueChanged(int value){
    // Return if no images
    if(m_numFrames <= 0) {
        return;
    }
    // On change, update image
    // Assumes value is in range
    cv::Mat image = cv::imread(ui->lineEdit->text().toStdString() + "/" + m_imageFiles[ui->horizontalSlider->value() - 1].toStdString());
    //cv::Mat image2 = processImage(image);
    cv::Mat image2 = image;
    applyThreshold(image2);
    if(ui->checkBox_2->isChecked())
        applyLines(image2);
    QImage toDisplay = matToQImage(image2);
    ui->graphicsView->scene()->clear();
    ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(toDisplay));
    ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
}

// For use in QtConcurrent::mapped
double FrequencyDetection::countWhitePixels(const QString &item) {
    // Return early if cancelled
    if(QThread::currentThread()->isInterruptionRequested()) {
        return 0;
    }
    // Open file
    cv::Mat image = cv::imread(ui->lineEdit->text().toStdString() + "/" + item.toStdString());

    // Convert to grayscale
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    // Apply thresholding (should be binary inverted probably)
    cv::Mat binary = gray;
    applyThreshold(binary);

    // If checkbox set, use custom region, else use default
    cv::Mat bottomRegion;
    if(ui->checkBox_2->isChecked()) {
        bottomRegion = binary(cv::Range(ui->horizontalSliderUpperLimit->value(), ui->horizontalSliderLowerLimit->value()), cv::Range(0, binary.cols));
    } else { // Use entire image
        int height = binary.rows;
        int width = binary.cols;
        bottomRegion = binary(cv::Range(0, height), cv::Range(0, width));
    }

    // Count the number of white pixels (area)
    int area = cv::countNonZero(bottomRegion);
    return area;
}

void FrequencyDetection::handleComputationCompletion() {
    if(m_areas.size() < m_imageFiles.size()) {
        QMessageBox::information(this, tr("Notice"), tr("Processing incomplete or cancelled by user"));
        m_areas.clear();
        return;
    }
    if(ui->checkBox->isChecked())
        exportAreasToCSV(m_areas);

    // Compute FFT of area data
    std::vector<double> amplitudeSpectrum;
    computeFFT(m_areas, amplitudeSpectrum);

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
    // Calculate with fps
    double fps = ui->spinBoxFPS->value();
    double frequency = (dominantIndex * fps) / N;
    //std::cout << "Approximate Oscillation Frequency: " << frequency << " Hz" << std::endl;
    QString message = QString("Approximate Oscillation Frequency: %1 Hz").arg(frequency);
    QMessageBox::information(this, tr("Frequency Result"), message);

}

void FrequencyDetection::on_runButton_clicked() {
    // Check for empty files
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

    // Check if limits are reasonable
    if(ui->checkBox_2->isChecked() && ui->horizontalSliderUpperLimit->value() >= ui->horizontalSliderLowerLimit->value()) {
        QMessageBox::warning(this, tr("Warning"), tr("Upper limit must be above lower limit"));
        ui->runButton->setEnabled(true);
        return;
    }

    if(ui->checkBox->isChecked()) {
        // Show the Save File Dialog and get the file path
        m_fileName = QFileDialog::getSaveFileName(
            nullptr,
            "Save File",
            "",
            "CSV Files (*.csv);;All Files (*)"
            );

        // Strict file validity checks
        if (m_fileName.isEmpty()) {
            QMessageBox::warning(this, tr("Warning"), tr("No file selected. Please choose a file to save."));
            return;
        }
        QFile file(m_fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Error"), tr("The file could not be opened for writing. Please check the file path and permissions."));
            return;
        }
    }


    // Create the progress dialog with a cancel button
    progressDialog = new QProgressDialog(tr("Processing images"), tr("Cancel"), 0, m_imageFiles.size(), this);
    progressDialog->setWindowModality(Qt::WindowModal); // Modal dialog to block interaction with the main window
    progressDialog->setMinimumDuration(0); // Show instantly
    progressDialog->setAutoClose(true);    // Close automatically when done
    progressDialog->setAutoReset(true);    // Reset automatically when done

    // Lock UI elements to prevent errors
    ui->runButton->setEnabled(false);

    // Clear areas variable
    m_areas.clear();

    // Create FutureWatcher to monitor progress
    QFutureWatcher<double> *watcher = new QFutureWatcher<double>(this);

    // Connect progress dialog cancel button to stop processing
    connect(progressDialog, &QProgressDialog::canceled, watcher, &QFutureWatcher<double>::cancel);

    // Connect progress updates
    connect(watcher, &QFutureWatcher<double>::progressValueChanged, progressDialog, &QProgressDialog::setValue);

    // Connect when processing is finished
    connect(watcher, &QFutureWatcher<double>::finished, this, [this, watcher](){
        // Collect results when finished
        QFuture<double> future = watcher->future();
        m_areas.reserve(future.resultCount());

        for (int i = 0; i < future.resultCount(); ++i) {
            m_areas.push_back(future.resultAt(i));
        }

        // Enable the run button
        ui->runButton->setEnabled(true);

        // Further processing: FFT calculation, exporting, etc.
        handleComputationCompletion();
        watcher->deleteLater(); // Clean up the watcher
        progressDialog->deleteLater(); // Clean up progress dialog
    });

    // Cool and chill little lambda function to replace std::bind
    QFuture<double> future = QtConcurrent::mapped(m_imageFiles, [this](const QString s){return countWhitePixels(s);});
    watcher->setFuture(future);
    progressDialog->setMaximum(m_imageFiles.size());

    // future.waitForFinished();
    // areas.reserve(future.resultCount());
    // // Collect results from future
    // for(int i = 0; i < future.resultCount(); i++) {
    //     areas.push_back(future.resultAt(i));
    // }

    // if(ui->checkBox->isChecked())
    //     exportAreasToCSV(areas);

    // // Compute FFT of area data
    // std::vector<double> amplitudeSpectrum;
    // computeFFT(areas, amplitudeSpectrum);
    // //exportAreasToCSV(amplitudeSpectrum);

    // // Find the dominant frequency
    // int N = amplitudeSpectrum.size();
    // double maxAmplitude = 0.0;
    // int dominantIndex = 0;

    // for (int i = 1; i < N / 2; ++i) { // Use N/2 because of symmetry in FFT output
    //     if (amplitudeSpectrum[i] > maxAmplitude) {
    //         maxAmplitude = amplitudeSpectrum[i];
    //         dominantIndex = i;
    //     }
    // }

    // // Calculate with fps
    // double fps = ui->spinBoxFPS->value();
    // double frequency = (dominantIndex * fps) / N;
    // std::cout << "Approximate Oscillation Frequency: " << frequency << " Hz" << std::endl;
}

void FrequencyDetection::exportAreasToCSV(const std::vector<double> &areas) {
    // Show the Save File Dialog and get the file path
    // QString fileName = QFileDialog::getSaveFileName(
    //     nullptr,
    //     "Save File",
    //     "",
    //     "CSV Files (*.csv);;All Files (*)"
    //     );
    QString fileName = m_fileName;

    // Check if the user canceled the dialog
    if (fileName.isEmpty()) {
        return;
    }

    // Open the selected file for writing
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(nullptr, "Error", "Could not open file for writing!");
        return;
    }

    // Create a QTextStream to write data to the file
    QTextStream out(&file);

    // Write CSV headers (optional)
    out << "Index,Area\n";

    // Write each element of the vector to the file
    for (size_t i = 0; i < areas.size(); ++i) {
        out << i << "," << areas[i] << "\n";
    }

    // Close the file
    file.close();
}

// Upper limit
void FrequencyDetection::on_horizontalSliderUpperLimit_valueChanged(int value) {
    if(m_numFrames <= 0) {
        return;
    }
    m_upper = value;
    // Assumes value is in range
    cv::Mat image = cv::imread(ui->lineEdit->text().toStdString() + "/" + m_imageFiles[ui->horizontalSlider->value() - 1].toStdString());
    //cv::Mat image2 = processImage(image);
    cv::Mat image2 = image;
    applyThreshold(image2);
    if(ui->checkBox_2->isChecked())
        applyLines(image2);
    QImage toDisplay = matToQImage(image2);
    ui->graphicsView->scene()->clear();
    ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(toDisplay));
    ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
}

// Lower limit
void FrequencyDetection::on_horizontalSliderLowerLimit_valueChanged(int value) {
    if(m_numFrames <= 0) {
        return;
    }
    m_lower = value;
    // Assumes value is in range
    cv::Mat image = cv::imread(ui->lineEdit->text().toStdString() + "/" + m_imageFiles[ui->horizontalSlider->value() - 1].toStdString());
    //cv::Mat image2 = processImage(image);
    cv::Mat image2 = image;
    applyThreshold(image2);
    if(ui->checkBox_2->isChecked())
        applyLines(image2);
    QImage toDisplay = matToQImage(image2);
    ui->graphicsView->scene()->clear();
    ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(toDisplay));
    ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
}

// Show lines checkbox
void FrequencyDetection::on_checkBox_2_checkStateChanged(const Qt::CheckState &arg1) {
    if(m_numFrames <= 0) {
        return;
    }
    // Assumes value is in range
    cv::Mat image = cv::imread(ui->lineEdit->text().toStdString() + "/" + m_imageFiles[ui->horizontalSlider->value() - 1].toStdString());
    //cv::Mat image2 = processImage(image);
    cv::Mat image2 = image;
    applyThreshold(image2);
    if(arg1 == Qt::Checked) {
        applyLines(image2);
        ui->horizontalSliderLowerLimit->setEnabled(true);
        ui->horizontalSliderUpperLimit->setEnabled(true);
    } else {
        ui->horizontalSliderLowerLimit->setEnabled(false);
        ui->horizontalSliderUpperLimit->setEnabled(false);
    }
    QImage toDisplay = matToQImage(image2);
    ui->graphicsView->scene()->clear();
    ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(toDisplay));
    ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
}

