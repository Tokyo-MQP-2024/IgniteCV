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

// Function to convert QImage to cv::Mat
cv::Mat QImageToCvMat(const QImage& image) {
    cv::Mat mat;
    switch (image.format()) {
    case QImage::Format_RGB32: {
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    case QImage::Format_RGB888: {
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
        break;
    }
    case QImage::Format_Grayscale8: {
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    default:
        qWarning("QImage format not supported for conversion to cv::Mat.");
        break;
    }
    return mat;
}
//--------------------- IMAGE AVERAGING FUNCTIONS ---------------------

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

//-------------------- WIDTH MEASUREMENT FUNCTIONS --------------------

// Compute midpoint of two (x, y) coordinates
cv::Point midpoint(double x1, double y1, double x2, double y2) {
    double midX = (x1 + x2) / 2.0;
    double midY = (y1 + y2) / 2.0;
    return cv::Point(midX, midY);
}

// Function to sort contours based on their x-coordinate (left-to-right)
void sortContoursLeftToRight(std::vector<std::vector<cv::Point>>& contours) {
    std::sort(contours.begin(), contours.end(), [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
        return cv::boundingRect(a).x < cv::boundingRect(b).x;
    });
}

// Function to order the points in the correct sequence (top-left, top-right, bottom-right, bottom-left)
std::vector<cv::Point> order_points(cv::Point2f *pts) {
    std::vector<cv::Point> ordered_pts(4);

    // Sort the points by x and y coordinates
    std::vector<cv::Point2f> sorted_pts(pts, pts + 4);
    sort(sorted_pts.begin(), sorted_pts.end(), [](cv::Point2f a, cv::Point2f b) {
        return a.x < b.x;
    });

    // Top-left, bottom-left, top-right, bottom-right
    cv::Point2f tl = sorted_pts[0].x < sorted_pts[1].x ? sorted_pts[0] : sorted_pts[1];
    cv::Point2f tr = sorted_pts[2].x < sorted_pts[3].x ? sorted_pts[2] : sorted_pts[3];
    cv::Point2f bl = sorted_pts[0].x < sorted_pts[1].x ? sorted_pts[1] : sorted_pts[0];
    cv::Point2f br = sorted_pts[2].x < sorted_pts[3].x ? sorted_pts[3] : sorted_pts[2];

    ordered_pts[0] = tl;
    ordered_pts[1] = tr;
    ordered_pts[2] = br;
    ordered_pts[3] = bl;

    return ordered_pts;
}

// Modifies image in place. Overlays bounding box and width measurements
void imageWidthOverlay(cv::Mat &image) {
    // Process image here to get rid of extraneous pixels


    // Convert to grayscale
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    // May not need this
    //cv::GaussianBlur(gray, gray, cv::Size(7,7), 0);

    // Edge detection, dilation, erosion
    cv::Mat edge;
    cv::Canny(gray, edge, 50, 100);
    cv::dilate(edge, edge, cv::Mat());
    cv::erode(edge, edge, cv::Mat());

    // Find contours in the edge map
    std::vector<std::vector<cv::Point>> cnts;
    cv::findContours(edge, cnts, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Might not need
    sortContoursLeftToRight(cnts);

    // Loop over contours
    for(const auto &pt : cnts) {
        if(cv::contourArea(pt) < 100)
            continue;
        cv::RotatedRect box = cv::minAreaRect(pt);
        cv::Point2f vertices[4];
        box.points(vertices);

        // Order points
        std::vector<cv::Point> ordered_points = order_points(vertices);

        // Draw the rotated bounding box
        for (int j = 0; j < 4; j++) {
            line(image, ordered_points[j], ordered_points[(j + 1) % 4], cv::Scalar(0, 255, 0), 1);
        }

        // Loop over the original points and draw them
        for (int j = 0; j < 4; j++) {
            circle(image, ordered_points[j], 3, cv::Scalar(0, 0, 255), -1);
        }

        // Unpack points, then compute midpoint between top left and top right, then bottom left and bottom right
        cv::Point tl = ordered_points[0], tr = ordered_points[1], br = ordered_points[2], bl = ordered_points[3];

        cv::Point tltr = midpoint(tl.x, tl.y, tr.x, tr.y);
        cv::Point blbr = midpoint(bl.x, bl.y, br.x, br.y);

        // between top left and top right points
        cv::Point tlbl = midpoint(tl.x, tl.y, bl.x, bl.y);
        cv::Point trbr = midpoint(tr.x, tr.y, br.x, br.y);

        cv::circle(image, tltr, 3, cv::Scalar(255,0,0), -1);
        cv::circle(image, blbr, 3, cv::Scalar(255,0,0), -1);
        cv::circle(image, tlbl, 3, cv::Scalar(255,0,0), -1);
        cv::circle(image, trbr, 3, cv::Scalar(255,0,0), -1);

        // Draw lines between midpoints
        cv::line(image, tltr, blbr, cv::Scalar(255,0,0), 1);
        cv::line(image, tlbl, trbr, cv::Scalar(255,0,0), 1);

        double dA = std::hypot(tltr.x - blbr.x, tltr.y - blbr.y);
        double dB = std::hypot(tlbl.x - trbr.x, tlbl.y - trbr.y);

        // CHANGE THIS LATER WITH MEASUREMENTS
        double dimA = dA;
        double dimB = dB;

        // Draw the object sizes on the image
        cv::putText(image, cv::format("%.1fpx", dimA),
                    cv::Point(static_cast<int>(tltr.x - 15), static_cast<int>(tltr.y - 10)),
                    cv::FONT_HERSHEY_SIMPLEX, 0.65, cv::Scalar(255, 255, 255), 1);

        cv::putText(image, cv::format("%.1fpx", dimB),
                    cv::Point(static_cast<int>(trbr.x + 10), static_cast<int>(trbr.y)),
                    cv::FONT_HERSHEY_SIMPLEX, 0.65, cv::Scalar(255, 255, 255), 1);

    }


    //cv::imshow("Result", image);
}


// ------------------- FLAME TOOL FUNCTIONS --------------------------

// Edits image and circles vector in place.
void detectCircles(cv::Mat &image, std::vector<cv::Vec3f> &circles, int min, int max, int canny, int accum) {

    // Convert to gray
    std::cout << "made it to function call\n";
    cv::Mat gray;
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    //cv::imshow("gray", gray);

    std::cout << "converted to gray\n";

    // Blur - may not be necessary. Test
    cv::medianBlur(gray, gray, 21);  //add ui (must be odd)
    std::cout << "median blurr\n";

    cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 1,
                    gray.rows/19, // change this to detect circles with different distances to each other
                     canny, accum, min, max // change last two params (min_radius and max_radius) to detect larger circles
                     );

    std::cout << "circle detection\n";

    // Loop through array and draw circle
    for(size_t i = 0; i < circles.size(); i++) {
        cv::Vec3i c = circles[i];
        cv::Point center = cv::Point(c[0], c[1]);

        // Circle center
        cv::circle(image, center, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);
        // Circle outline
        int radius = c[2];
        circle(image, center, radius, cv::Scalar(255, 0, 255), 3, cv::LINE_AA);

        //std::cout << "drawing circles\n";
    }

}

void createGridlines(cv::Mat &image, std::vector<cv::Vec3f> &circles) {
    // Sort circle vector by x values
    std::sort(circles.begin(), circles.end(), [](const cv::Vec3i &a, const cv::Vec3i &b) {
        return a[0] < b[0];
    });

    for (const auto& v : circles) {
        std::cout << "(" << v[0] << ", " << v[1] << ", " << v[2] << ")" << std::endl;
    }

    // Average x values of circles within threshold
    int threshold = 10;
    float prev = -1;
    std::vector<double> averages;
    double columnSum = 0;
    int columnCount = 0;

    for (const auto& c : circles) {
        float value = c[0];
        if(prev == -1 || (value - prev) < threshold) {
            columnSum += value;
            columnCount += 1;
        } else {
            if(columnCount == 0) {
                prev = -1;
                continue;
            }
            double average = columnSum / columnCount;
            std::cout << average << std::endl;
            averages.push_back(average);
            columnSum = 0;
            columnCount = 0;
        }
        prev = value;
    }

    // Draw vertical lines between the averages
    for(int i = 0; i < averages.size() - 1; i++) {
        double lineX = (averages[i] + averages[i + 1]) / 2.0;
        cv::Point startPoint(lineX, 0);
        cv::Point endPoint(lineX, image.rows);

        cv::line(image, startPoint, endPoint, cv::Scalar(0, 255, 0), 2);
    }

}
void graphicsViewHelper(QGraphicsView *view, FlameProcessing *fp, cv::Mat f) {

    //TOD: rm fp


    //std::string filePathSTD = filePath.toStdString();
    //ui->FileLabel->setText(filePath);
    //videoFilePath = filePathSTD;
    //view = new QGraphicsView();
    QGraphicsScene *scene = new QGraphicsScene();
    view->setScene(scene);
    //cv::VideoCapture cap(videoFilePath);
    //cv::Mat frame1;
    //cap >> frame1;
    view->scene()->clear();
    QImage qimg = matToQImage(f);
    QPixmap pixmap = QPixmap::fromImage(qimg);
    QGraphicsPixmapItem *item = scene->addPixmap(pixmap);
    view->fitInView(item, Qt::KeepAspectRatio);

}

// Fast Fourier Transform for frequency detection
void computeFFT(const std::vector<double> &inputSignal, std::vector<double> &amplitudeSpectrum) {
    int N = inputSignal.size();

    // Allocate input and output arrays for FFTW
    fftw_complex *in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_plan plan = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

    // Copy input signal into the input array
    for (int i = 0; i < N; ++i) {
        in[i][0] = inputSignal[i]; // Real part
        in[i][1] = 0.0;            // Imaginary part (0 since we're starting with real data)
    }

    // Execute FFT
    fftw_execute(plan);

    // Compute the amplitude spectrum (magnitude of FFT output)
    amplitudeSpectrum.resize(N);
    for (int i = 0; i < N; ++i) {
        amplitudeSpectrum[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]); // sqrt(Re^2 + Im^2)
    }

    // Cleanup
    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
}


void updateNumericalLabel(QLabel *label, int val) {

    QString valSTR = QString::number(val);
    label->setText(valSTR);
}
