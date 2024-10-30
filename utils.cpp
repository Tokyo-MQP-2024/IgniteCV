#include "utils.h"

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
