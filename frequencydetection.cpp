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
#include <QImageReader>



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

// Refresh image in graphics view
void FrequencyDetection::refreshImage() {
    // Assumes value is in range
    QString filePath = ui->lineEdit->text() + "/" + m_imageFiles[ui->horizontalSlider->value() - 1];
    QFile file(filePath);
    file.open(QFile::ReadOnly);
    qint64 sz = file.size();
    std::vector<uchar> buf(sz);
    file.read((char*) buf.data(), sz);


    cv::Mat image = cv::imdecode(buf, cv::IMREAD_COLOR);

    applyThreshold(image);
    if(ui->checkBoxApplyLimits->isChecked())
        applyLines(image);
    QImage toDisplay = matToQImage(image);
    ui->graphicsView->scene()->clear();
    ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(toDisplay));
    ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
}

// Slider for frame selection
void FrequencyDetection::on_horizontalSlider_valueChanged(int value) {
    if(m_numFrames <= 0) {
        return;
    }
    refreshImage();
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
    m_width = toDisplay.width();
    m_height = toDisplay.height();

    // Loop through images to make sure width and height are same [TAKES TOO LONG]
    // for(const QString item : m_imageFiles) {
    //     QImageReader reader(ui->lineEdit->text() + "/" + item);
    //     QSize size = reader.size();
    //     if(size.height() != m_height || size.width() != m_width) {
    //         QMessageBox::warning(this, tr("Error"), tr("Images have different dimensions"));
    //         ui->lineEdit->clear();
    //         m_height = -1;
    //         m_width = -1;
    //         return;
    //     }
    // }

    ui->horizontalSliderLowerLimit->setMaximum(toDisplay.height());
    ui->horizontalSliderUpperLimit->setMaximum(toDisplay.height());
    ui->spinBoxLowerLimit->setMaximum(toDisplay.height());
    ui->spinBoxUpperLimit->setMaximum(toDisplay.height());

    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(toDisplay));
    ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
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
    refreshImage();
}

// For use in QtConcurrent::mapped
double FrequencyDetection::countWhitePixels(const QString &item) {
    // Return early if cancelled
    if(QThread::currentThread()->isInterruptionRequested()) {
        return 0;
    }
    // Open file 
    QString filePath = ui->lineEdit->text() + "/" + item;
    QFile file(filePath);
    file.open(QFile::ReadOnly);
    qint64 sz = file.size();
    std::vector<uchar> buf(sz);
    file.read((char*) buf.data(), sz);


    cv::Mat image = cv::imdecode(buf, cv::IMREAD_COLOR);

    // Convert to grayscale
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    // Apply thresholding (should be binary inverted probably)
    cv::Mat binary = gray;
    applyThreshold(binary);

    // If checkbox set, use custom region, else use default
    cv::Mat bottomRegion;
    if(ui->checkBoxApplyLimits->isChecked()) {
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
    if(ui->checkBoxApplyLimits->isChecked() && ui->horizontalSliderUpperLimit->value() >= ui->horizontalSliderLowerLimit->value()) {
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
    out << tr("Index") + "," + tr("Area") + "," + tr("Time") + "\n";

    // Write each element of the vector to the file
    for (size_t i = 0; i < areas.size(); ++i) {
        // Convert frame to time
        double time = static_cast<double>(i) / ui->spinBoxFPS->value();
        out << i << "," << areas[i] << "," << time << "\n";
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
    refreshImage();
}

// Lower limit
void FrequencyDetection::on_horizontalSliderLowerLimit_valueChanged(int value) {
    if(m_numFrames <= 0) {
        return;
    }
    m_lower = value;
    refreshImage();
}

// Show lines checkbox
void FrequencyDetection::on_checkBoxApplyLimits_checkStateChanged(const Qt::CheckState &arg1) {
    if(m_numFrames <= 0) {
        return;
    }
    if(arg1 == Qt::Checked) {
        ui->horizontalSliderLowerLimit->setEnabled(true);
        ui->horizontalSliderUpperLimit->setEnabled(true);
    } else {
        ui->horizontalSliderLowerLimit->setEnabled(false);
        ui->horizontalSliderUpperLimit->setEnabled(false);
    }  
    refreshImage();
}

