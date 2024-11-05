#include "utils.h"

#include <QDir>
#include <opencv2/imgcodecs.hpp>
#include <QMutex>
#include <QtConcurrent/QtConcurrentMap>
#include <opencv2/opencv.hpp>
#include <QFileInfoList>
#include <QImage>
#include <QList>
#include <QtConcurrent/QtConcurrent>
#include <opencv2/opencv.hpp>
#include <algorithm>

// UNTESTED
QImage matToQImage(const cv::Mat &mat) {
    // Handle different types of Mat formats
    switch (mat.type()) {
    case CV_8UC3:  // 3-channel RGB image
        return QImage((uchar*) mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888)
            .rgbSwapped(); // Convert from BGR (OpenCV default) to RGB

    case CV_8UC1:  // Grayscale image
        return QImage((uchar*) mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);

    case CV_8UC4:  // 4-channel RGBA image
        return QImage((uchar*) mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGBA8888);

    default:
        qWarning("matToQImage() - unsupported cv::Mat type");
        return QImage();
    }
}

// Map function to average a batch of images
cv::Mat averageBatch(const QStringList& batchPaths) {
    QFileInfoList fileList;

    // Convert each QString in batchPaths to a QFileInfo and add it to fileList
    for (const QString& path : batchPaths) {
        fileList.append(QFileInfo(path));
    }

    // Initialize an accumulator matrix with the size of the first valid image and float precision
    cv::Mat accumulator;

    int imageCount = 0;
    for (const QFileInfo& fileInfo : fileList) {
        QString filePath = fileInfo.absoluteFilePath();
        cv::Mat img = cv::imread(filePath.toStdString());  // Load the image using OpenCV

        // Check if the image is valid
        if (img.empty()) {
            qWarning() << QFileDevice::tr("Skipping invalid or incompatible file:") << filePath;
            continue;
        }

        // Convert the image to float format for accumulation
        cv::Mat floatImg;
        img.convertTo(floatImg, CV_32FC3);

        // Initialize accumulator on the first valid image
        if (accumulator.empty()) {
            accumulator = cv::Mat::zeros(floatImg.size(), CV_32FC3);
        }

        // Add the current image to the accumulator
        accumulator += floatImg;
        ++imageCount;
    }

    // Ensure we have at least one valid image to average
    if (imageCount == 0) {
        qWarning() << QFileDevice::tr("No valid images could be processed for averaging.");
        return cv::Mat();
    }

    // Divide the accumulator by the number of images to get the average
    accumulator /= static_cast<float>(imageCount);

    // Convert the result back to 8-bit for display or saving - DONT DO THIS
    //cv::Mat averageImage;
    //accumulator.convertTo(averageImage, CV_8UC3);

    return accumulator;
}

// Reduce function to accumulate each batch average
void accumulateBatch(cv::Mat& accumulator, const cv::Mat& batchAverage) {
    if (batchAverage.empty()) return;

    // Initialize accumulator on the first valid batch
    if (accumulator.empty()) {
        accumulator = batchAverage.clone();
    } else {
        accumulator += batchAverage;
    }
}

// Main function to average images from a folder using optimized batch size
cv::Mat averageImagesFromFolder(const QString& folderPath) {
    QDir directory(folderPath);
    directory.setFilter(QDir::Files);
    directory.setNameFilters({"*.jpg", "*.jpeg", "*.png", "*.bmp", "*.tif", "*.tiff"});
    QFileInfoList fileList = directory.entryInfoList();

    if (fileList.isEmpty()) {
        qWarning() << "No valid images found in the folder.";
        return cv::Mat();
    }

    // Convert QFileInfoList to QStringList of file paths
    QStringList filePaths;
    for (const QFileInfo& fileInfo : fileList) {
        filePaths << fileInfo.absoluteFilePath();
    }

    // Calculate optimized batch size with a minimum of 1
    int idealThreadCount = QThread::idealThreadCount();
    int batchSize = std::max(1, (int)(filePaths.size() / idealThreadCount));

    // Divide file paths into batches, ensuring all files are included
    QList<QStringList> batches;
    for (int i = 0; i < filePaths.size(); i += batchSize) {
        batches.append(filePaths.mid(i, batchSize));
    }

    // Run QtConcurrent::mappedReduced with optimized batch size
    cv::Mat accumulator = QtConcurrent::mappedReduced<cv::Mat>(
                              batches,                      // Input batches
                              averageBatch,                 // Map function
                              accumulateBatch//,              // Reduce function
                              //QtConcurrent::OrderedReduce   // Ensures reduction order
                              ).result();

    // Final averaging
    int totalBatchCount = batches.size();
    if (totalBatchCount > 0 && !accumulator.empty()) {
        accumulator /= static_cast<float>(totalBatchCount);
    } else {
        qWarning() << "No valid images could be processed for averaging.";
        return cv::Mat();
    }

    // Convert to 8-bit for display or saving
    cv::Mat averageImage;
    accumulator.convertTo(averageImage, CV_8UC3);

    return averageImage;
}


