#ifndef FLAMEPROCESSING_H
#define FLAMEPROCESSING_H

#include <qgraphicsscene.h>
#endif // FLAMEPROCESSING_H

#include <stdio.h>
#include <fstream>
#include <limits>
#include <string>

#include <opencv2/opencv.hpp>
#include "opencv2/highgui.hpp"


class FlameProcessing {
public:
    //functions
    FlameProcessing();

    void parseVideo(std::string videoFilePath, QGraphicsView *scene);

    bool checkMP4(std::string newFile);

    void setScale();

    void setStopProcess(bool state);

    void scalingMouse(int event, int x, int y, int flags);

    void imageScaling(std::string videoFilePath, char axis);

    void imageROISelect(std::string videoFilePath);

    void setIRLScale(double x, double y);

    cv::Mat findContourImage(cv::Mat original_frame);

    void setROIBox(int x, int y, int h, int w);

    std::vector<double> recordPositions(std::vector<double> segments);

    std::vector<std::vector<double>> cleanData(std::vector<std::vector<double>> positions);

    std::vector<double> recordAngle(std::vector<double> segments, cv::Mat &image, int threshold);

    std::vector<cv::Point> findLowestEdges(std::vector<cv::Point> contourInfo);

    void drawLowestEdges(cv::Mat& image,const std::vector<cv::Point>& edgePoints,const cv::Scalar& color,int radius,int thickness);

    std::vector<cv::Point> findContourPixels(std::vector<cv::Point> contour, cv::Mat image);

    std::vector<cv::Point> selectLowestContour(int threshold, int currentX);

    double findAngle(double vx, double vy, double refVx, double refVy);

    cv::Mat findContourMask(cv::Mat original_frame);



private:

    bool stopProcess = false;
    QGraphicsScene *scene = new QGraphicsScene();
    int scaleClicks = 0;

    int currX = 0;
    int currY = 0;
    int pixelsX = 0;
    int pixelsY = 0;
    double irlScaleX = 0.0;
    double irlScaleY = 0.0;
    double cmPerPixel = 0.0;

    bool fist_point_selected = false;

    cv::Point currPos;

    int maskX = 0;
    int maskY = 0;
    int maskH = 0;
    int maskW = 0;

    double unitPerPixelX = 0;
    double unitPerPixelY = 0;
    static void mouseCallback(int event, int x, int y, int flags, void* userdata);

    cv::Mat frame, HSVFrame, foreground, mask, hsvMask, resultImage, dilateErodeMask;
    cv::Mat dMask, eMask;
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
    cv::Ptr<cv::BackgroundSubtractor> bg_sub = cv::createBackgroundSubtractorKNN();

    std::vector<std::vector<cv::Point>> filteredContours;

    std::vector<std::vector<double>> flamePositionData;

};


