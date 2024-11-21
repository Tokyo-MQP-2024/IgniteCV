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

    void setStopProcess(bool state);

    void scalingMouse(int event, int x, int y, int flags);

    void imageScaling(std::string videoFilePath, char axis);

    void imageROISelect(std::string videoFilePath);

    void setIRLScale(double x, double y);

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

    bool fist_point_selected = false;

    cv::Point currPos;



    int maskX = 0;
    int maskY = 0;
    int maskH = 0;
    int maskW = 0;

    double unitPerPixelX = 0;
    double unitPerPixelY = 0;
    static void mouseCallback(int event, int x, int y, int flags, void* userdata);
};


