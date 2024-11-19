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

    void imageScaling(cv::VideoCapture cap);

private:
    bool stopProcess = false;
    QGraphicsScene *scene = new QGraphicsScene();
    int scaleClicks = 0;

    int maskX = 0;
    int maskY = 0;
    int maskH = 0;
    int maskW = 0;

    static void mouseCallback(int event, int x, int y, int flags, void* userdata);



};


