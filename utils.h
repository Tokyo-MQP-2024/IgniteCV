#ifndef UTILS_H
#define UTILS_H

#include <opencv2/core/mat.hpp>
#include <qimage.h>
#include <QGraphicsView>
#include "FlameProcessing.h"
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <fftw3.h>
#include <QLabel>

QImage matToQImage(const cv::Mat &mat);
cv::Mat QImageToCvMat(const QImage& image);
cv::Mat averageImagesFromFolder(const QString& folderPath);
void imageWidthOverlay(cv::Mat &image);
void detectCircles(cv::Mat &image, std::vector<cv::Vec3f> &circles);
void createGridlines(cv::Mat &image, std::vector<cv::Vec3f> &circles);
void graphicsViewHelper(QGraphicsView *view, FlameProcessing *fp, cv::Mat f);
void computeFFT(const std::vector<double> &inputSignal, std::vector<double> &amplitudeSpectrum);
void detectCircles(cv::Mat &image, std::vector<cv::Vec3f> &circles, int min, int max, int canny, int accum);
void updateNumericalLabel(QLabel *label, int val);

#endif // UTILS_H
