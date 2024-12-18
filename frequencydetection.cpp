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
    dir.setSorting(QDir::Name | QDir::LocaleAware); // Natural order sorting

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

// Update image when threshold slider changes
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

// For FFT
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
    QString message = QString("Approximate Oscillation Frequency: %1 Hz").arg(frequency);
    QMessageBox::information(this, tr("Frequency Result"), message);

}

// Run - start processing
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

// Export areas
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
    out << "Index,Area,Time\n";

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


// CURVE DETECTION STUFF ----------------------------------------


// On Tab Switch, clear member variables
void FrequencyDetection::on_tabWidget_currentChanged(int index) {


    // Clear graphicsview
    //ui->graphicsView->scene()->clear();
}

// Select Folder
void FrequencyDetection::on_pushButtonSelectCurve_clicked() {
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Open a Folder"));
    ui->lineEditFilenameCurve->setText(folderPath);
}

// Set
void FrequencyDetection::on_pushButtonSetCurve_clicked() {
    QString folderPath = ui->lineEditFilenameCurve->text();
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
    dir.setSorting(QDir::Name | QDir::LocaleAware); // Natural order sorting

    // Get the list of image files
    m_imageFilesCurve = dir.entryList(QDir::Files);

    // Error if no images
    if(m_imageFilesCurve.size() < 1) {
        QMessageBox::warning(this, tr("Warning"), tr("No images in folder. Please select another folder"));
        return;
    }

    m_numFramesCurve = m_imageFilesCurve.size();

    // Set trackbar and numbox max to number of images
    ui->horizontalSliderFrameCurve->setMaximum(m_numFramesCurve);
    ui->spinBoxFrameCurve->setMaximum(m_numFramesCurve);



    // Load first image
    QImage toDisplay = QImage(folderPath + "/" + m_imageFilesCurve[0]);
    m_widthCurve = toDisplay.width();
    m_heightCurve = toDisplay.height();

    // Set max values for ROI boxes. No error checking.
    ui->spinBoxX->setMaximum(m_widthCurve);
    ui->spinBoxY->setMaximum(m_heightCurve);
    ui->spinBoxWidth->setMaximum(m_widthCurve);
    ui->spinBoxHeight->setMaximum(m_heightCurve);

    ui->graphicsView->setScene(new QGraphicsScene(this));
    ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(toDisplay));
    ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
}

// Apply Canny Edge Detection, returns edges.
cv::Mat FrequencyDetection::applyCanny(cv::Mat &image) {
    // Convert to grayscale
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    // Experimental
    cv::Mat gradX, gradY, gradMagnitude;
    cv::Sobel(gray, gradX, CV_64F, 1, 0, -1);
    cv::Sobel(gray, gradY, CV_64F, 0, 1, -1);
    cv::magnitude(gradX, gradY, gradMagnitude);
    // Normalize to 8-bit range
    cv::Mat gradMagnitude8U;
    gradMagnitude.convertTo(gradMagnitude8U, CV_8U);
    cv::Mat edges;
    cv::threshold(gradMagnitude8U, edges, 50, 255, cv::THRESH_BINARY);
    // Apply blur to remove noise
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.5);

    // The smallest value between threshold1 and threshold2 is used for edge linking.
    // The largest value is used to find initial segments of strong edges
    double thresh1 = ui->horizontalSliderThresh1->value();
    double thresh2 = ui->horizontalSliderThresh2->value();
    cv::Canny(blurred, edges, thresh1, thresh2, -1, true);
    return edges;
}

// Draw center line, averaging min x and max x at every y level
std::vector<int> FrequencyDetection::computeCenter(cv::Mat &image) {
    // Return early if cancelled
    if(QThread::currentThread()->isInterruptionRequested()) {
        return std::vector<int>();
    }
    std::vector<int> centerlineX;

    // Variables to store previous valid centerline values for interpolation
    int prevCenterX = -1;
    int nextCenterX = -1;

    for (int y = 0; y < image.rows; ++y) {
        int left = -1;
        int right = -1;

        // Find the first (leftmost) edge
        for (int x = 0; x < image.cols; ++x) {
            if (image.at<uchar>(y, x) == 255) {
                left = x;
                break;
            }
        }

        // Find the last (rightmost) edge
        for (int x = image.cols - 1; x >= 0; --x) {
            if (image.at<uchar>(y, x) == 255) {
                right = x;
                break;
            }
        }

        // If valid edges are found, compute the centerline
        if (left != -1 && right != -1) {
            int centerX = (left + right) / 2;
            centerlineX.push_back(centerX);
            prevCenterX = centerX;
            nextCenterX = -1;  // Reset the nextCenterX for interpolation
        } else {
            // Handle case where no edges are found in this row
            if (prevCenterX != -1) {
                // Interpolate or use previous centerline
                centerlineX.push_back(prevCenterX);
            } else if (nextCenterX != -1) {
                // If previous centerline is not available, use the next valid centerline
                centerlineX.push_back(nextCenterX);
            } else {
                // If both previous and next are unavailable, set to a default (image center)
                centerlineX.push_back(image.cols / 2);
            }
        }
    }
    return centerlineX;
}


// Refresh image in graphics view
void FrequencyDetection::refreshImageCurve() {
    if(m_numFramesCurve <= 0) {
        return;
    }
    // Assumes value is in range
    QString filePath = ui->lineEditFilenameCurve->text() + "/" + m_imageFilesCurve[ui->horizontalSliderFrameCurve->value() - 1];
    QFile file(filePath);
    file.open(QFile::ReadOnly);
    qint64 sz = file.size();
    std::vector<uchar> buf(sz);
    file.read((char*) buf.data(), sz);


    cv::Mat image = cv::imdecode(buf, cv::IMREAD_COLOR);

    //applyThreshold(image);
    //if(ui->checkBoxApplyLimits->isChecked())
    //    applyLines(image);

    // If ROI checked
    if(ui->checkBoxROI->isChecked()) {
        cv::Rect2d roi;
        roi.x = ui->spinBoxX->value();
        roi.y = ui->spinBoxY->value();
        roi.width = ui->spinBoxWidth->value();
        roi.height = ui->spinBoxHeight->value();

        if(roi.width == 0 || roi.height == 0) {
            QMessageBox::warning(this, tr("Warning"), tr("Selection cannot be empty"));
            ui->checkBoxROI->setCheckState(Qt::Unchecked);
            return;
        }

        try {
            image = image(roi);
        } catch (const cv::Exception &e) {
            QMessageBox::warning(this, tr("OpenCV Error"), QString::fromStdString(e.what()) + "\n The region of interest may be out of bounds");
            ui->checkBoxROI->setCheckState(Qt::Unchecked);
            return;
        }

    }

    if(ui->checkBoxApplyCanny->isChecked()) {
        cv::Mat edges = applyCanny(image);

        image = edges;

        std::vector<int> centerlineX = computeCenter(image);
        // Convert to color so centerline gets displayed
        cv::cvtColor(image, image, cv::COLOR_GRAY2BGR);
        for(int i = 0; i < centerlineX.size(); i++) {
            int centerX = centerlineX[i];
            if(centerX >= 0) {
                cv::circle(image, cv::Point(centerX, i), 1, cv::Scalar(0,0,255), -1);
            }
        }
    }



    QImage toDisplay = matToQImage(image);
    ui->graphicsView->scene()->clear();
    ui->graphicsView->scene()->addPixmap(QPixmap::fromImage(toDisplay));
    ui->graphicsView->fitInView(ui->graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);
}

// Refresh image on slider change
void FrequencyDetection::on_horizontalSliderFrameCurve_valueChanged(int value) {
    refreshImageCurve();
}

// On thresh1 value change, refresh image
void FrequencyDetection::on_horizontalSliderThresh1_valueChanged(int value) {
    refreshImageCurve();
}

// On thresh2 value change, refresh image
void FrequencyDetection::on_horizontalSliderThresh2_valueChanged(int value) {
    refreshImageCurve();
}

// On Canny checkbox clicked refresh image
void FrequencyDetection::on_checkBoxApplyCanny_clicked() {
    refreshImageCurve();
}

// Select ROI region and store attributes in boxes
void FrequencyDetection::on_pushButtonSelectRegion_clicked() {
    // Assumes value is in range
    QString filePath = ui->lineEditFilenameCurve->text() + "/" + m_imageFilesCurve[ui->horizontalSliderFrameCurve->value() - 1];
    QFile file(filePath);
    file.open(QFile::ReadOnly);
    qint64 sz = file.size();
    std::vector<uchar> buf(sz);
    file.read((char*) buf.data(), sz);


    cv::Mat image = cv::imdecode(buf, cv::IMREAD_COLOR);

    QMessageBox::information(this, tr("Information"), tr("Select a ROI and then press SPACE or ENTER button. Cancel the selection process by pressing c button and closing the ROI window."));
    cv::Rect2d roi = cv::selectROI("Select ROI", image);

    if(roi.width == 0 || roi.height == 0) {
        QMessageBox::warning(this, tr("Warning"), tr("Selection cannot be empty"));
        return;
    }

    cv::Mat imCropped;

    try {
        imCropped = image(roi);
    } catch (const cv::Exception &e) {
        QMessageBox::warning(this, tr("OpenCV Error"), QString::fromStdString(e.what()) + "\n The region of interest may be out of bounds");
        return;
    }
    cv::destroyWindow("Select ROI");

    // Set spinBoxes
    ui->spinBoxX->setValue(roi.x);
    ui->spinBoxY->setValue(roi.y);
    ui->spinBoxWidth->setValue(roi.width);
    ui->spinBoxHeight->setValue(roi.height);


}

// When ROI selected, apply, give error if bad.
void FrequencyDetection::on_checkBoxROI_checkStateChanged(const Qt::CheckState &arg1) {
    // cv::Rect2d roi;
    // roi.x = ui->spinBoxX->value();
    // roi.y = ui->spinBoxY->value();
    // roi.width = ui->spinBoxWidth->value();
    // roi.height = ui->spinBoxHeight->value();
    refreshImageCurve();
}

// Function to compute standard deviation for each column (y-value)
double computeStdDevForColumn(const cv::Mat& mat, int colIdx) {
    std::vector<double> columnData;
    for (int i = 0; i < mat.rows; ++i) {
        columnData.push_back(mat.at<double>(i, colIdx)); // Collect data for the column (y-value)
    }

    double mean = std::accumulate(columnData.begin(), columnData.end(), 0.0) / columnData.size();
    double variance = std::accumulate(columnData.begin(), columnData.end(), 0.0,
                                      [mean](double acc, double x) { return acc + (x - mean) * (x - mean); }) /
                      columnData.size();
    return std::sqrt(variance); // Return standard deviation
}

// Run the program with selected parameters
void FrequencyDetection::on_pushButtonRunCurve_clicked() {
    // Check for empty files
    QString folderPath = ui->lineEditFilenameCurve->text();
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

    // Set m_fileName member variable to be used by helper function
    if(ui->checkBoxCSVCurve->isChecked()) {
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


    // The meat of the matter

    // Create the progress dialog with a cancel button
    progressDialog = new QProgressDialog(tr("Processing images"), tr("Cancel"), 0, m_imageFilesCurve.size(), this);
    progressDialog->setWindowModality(Qt::WindowModal); // Modal dialog to block interaction with the main window
    progressDialog->setMinimumDuration(0); // Show instantly
    progressDialog->setAutoClose(true);    // Close automatically when done
    progressDialog->setAutoReset(true);    // Reset automatically when done

    // Lock UI elements to prevent errors
    ui->pushButtonRunCurve->setEnabled(false);

    // Create FutureWatcher to monitor progress
    QFutureWatcher<std::vector<int>> *watcher = new QFutureWatcher<std::vector<int>>(this);

    // Connect progress dialog cancel button to stop processing
    connect(progressDialog, &QProgressDialog::canceled, watcher, &QFutureWatcher<std::vector<int>>::cancel);

    // Connect progress updates
    connect(watcher, &QFutureWatcher<std::vector<int>>::progressValueChanged, progressDialog, &QProgressDialog::setValue);

    // Connect when processing is finished
    connect(watcher, &QFutureWatcher<std::vector<int>>::finished, this, [this, watcher](){
        // Collect results when finished
        QFuture<std::vector<int>> future = watcher->future();
        m_centerlines.reserve(future.resultCount());


        // Note: Rows are frames, Columns are every x value in that frame, index of column is y value.
        int numFrames = future.resultCount(); // Rows
        int numYValues = future.resultAt(0).size(); // Columns

        m_centerlines = cv::Mat(numFrames, numYValues, CV_32S);

        // Step 3: Fill the cv::Mat with data
        for (int i = 0; i < numFrames; ++i) {
            std::vector<int> frameData = future.resultAt(i);
            for (int j = 0; j < numYValues; ++j) {
                m_centerlines.at<int>(i, j) = frameData[j];
            }
        }

        // Check mat
        // std::cout << "Numrows: " << m_centerlines.rows << std::endl;
        // std::cout << "Numcols: " << m_centerlines.cols << std::endl;

        if(ui->checkBoxCSVCurve->isChecked()) {
            exportCenterlineToCSV(m_centerlines);
        }



        // Enable the run button
        ui->pushButtonRunCurve->setEnabled(true);

        // Further processing: FFT calculation, exporting, etc.
        //handleComputationCompletion();
        watcher->deleteLater(); // Clean up the watcher
        progressDialog->deleteLater(); // Clean up progress dialog
    });

    // Cool and chill little lambda function to replace std::bind
    QFuture<std::vector<int>> future = QtConcurrent::mapped(m_imageFilesCurve, [this](const QString s){
        // Open file
        QString filePath = ui->lineEditFilenameCurve->text() + "/" + s;
        QFile file(filePath);
        file.open(QFile::ReadOnly);
        qint64 sz = file.size();
        std::vector<uchar> buf(sz);
        file.read((char*) buf.data(), sz);

        cv::Mat image = cv::imdecode(buf, cv::IMREAD_COLOR);

        cv::Rect2d roi;
        roi.x = ui->spinBoxX->value();
        roi.y = ui->spinBoxY->value();
        roi.width = ui->spinBoxWidth->value();
        roi.height = ui->spinBoxHeight->value();

        try {
            image = image(roi);
        } catch (const cv::Exception &e) {
            QMessageBox::warning(this, tr("OpenCV Error"), QString::fromStdString(e.what()) + "\n The region of interest may be out of bounds");
            ui->checkBoxROI->setCheckState(Qt::Unchecked);
            return std::vector<int>();
        }


        cv::Mat edges = applyCanny(image);
        return computeCenter(edges);
    });
    watcher->setFuture(future);
    progressDialog->setMaximum(m_imageFilesCurve.size());
}

// Export areas
void FrequencyDetection::exportCenterlineToCSV(const cv::Mat &data) {
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
    out << tr("Frame");

    // Write headers for each column
    for (int col = 0; col < data.cols; ++col) {
        out << "," << tr("Y ") + QString::number(col + 1); // Optional: you can name columns based on index
    }
    out << "\n"; // New line after header

    // Write each row of the matrix
    for (int row = 0; row < data.rows; ++row) {
        out << row;  // Write row index

        // Write each column value in the current row
        for (int col = 0; col < data.cols; ++col) {
            out << "," << data.at<int>(row, col); // Assuming float type, adjust if needed
        }

        out << "\n"; // New line after each row
    }

    // Close the file
    file.close();
}


void FrequencyDetection::on_pushButtonReset_clicked() {
    ui->checkBoxROI->setCheckState(Qt::Unchecked);
    ui->spinBoxX->setValue(0);
    ui->spinBoxY->setValue(0);
    ui->spinBoxWidth->setValue(0);
    ui->spinBoxHeight->setValue(0);
}

