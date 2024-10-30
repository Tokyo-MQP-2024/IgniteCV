#ifndef UTILS_H
#define UTILS_H

#include <opencv2/core/mat.hpp>
#include <qimage.h>

QImage matToQImage(const cv::Mat &mat);

#endif // UTILS_H
