// need to calc the burned area

#include <stdio.h>
#include <fstream>
#include <limits>
#include <string>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui.hpp"
#include "FrameAnalyzer.hpp"



void writeToCSV(std::string filename, std::vector<std::string> cols, std::vector<std::tuple<int,int,int>> &data) {
    std::ofstream myFile(filename);

    for (int i = 0; i < cols.size(); i++) {
        myFile << cols[i];
        if(i != cols.size() - 1) {
            myFile << ",";
        }else {
            myFile << "\n";
        }
    }

    for (const auto &entry : data) {
        myFile << std::get<0>(entry) << ", " << std::get<1>(entry) << ", " << std::get<2>(entry) << "\n";
    }
   
    
    // Close the file
    myFile.close();
    
    
}


void parseVideo() {

    auto const MASK_WINDOW = "Mask Settings";
    cv::namedWindow(MASK_WINDOW, cv::WINDOW_AUTOSIZE);
    // Open the video file
    std::string videoFilePath = "C:/Users/chris/WPI/JapanMQPClone/JapanMQP/data/FireSafetyVideo.mp4";
    cv::VideoCapture cap(videoFilePath);

    // HSV range to detect blue color
    int minHue = 0, maxHue = 50;
    int minSat = 50, maxSat = 255;
    int minVal =  150, maxVal = 255;

    int minThresh = 200, maxThresh = 255;
    int blurAmount = 2;
    int minBBArea = 40;

    std::vector<std::tuple<int, int, int>> fireVerticalPositions;
    std::vector<std::pair<int, int>> fireLastPositions;
    
    // Check if the video file was opened successfully
    if (!cap.isOpened()) {
        std::cerr << "Error: FAILED OPENING FILE" << std::endl;
        
    } else {

        // Get video frame rate
        double fps = cap.get(cv::CAP_PROP_FPS);
        double timePerFrame = 1.0/fps; // in seconds
        double accumulatedTime = 0.0;
        double sampleInterval = 1.0;
        int seconds = 0;

        std::cout << "Frames per second: " << fps << std::endl;

        // FRAME VARIABES
        cv::Mat frame, HSVFrame, foreground, mask, hsvMask, resultImage, dilateErodeMask;
        cv::Mat dMask, eMask;

        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
        int frameCount = 0;
        cv::Ptr<cv::BackgroundSubtractor> bg_sub = cv::createBackgroundSubtractorKNN();

        int currY = -1;
        int lastY = -1;

        while (true) {
            accumulatedTime = accumulatedTime + timePerFrame;

            // Capture each frame
            cap >> frame;

            // If the frame is empty, break the loop (end of video)
            if (frame.empty()) {
                break;
            }

            cv::resize(frame, frame, cv::Size(), 0.5, 0.5); // Scale down by half


            bg_sub->apply(frame, mask);
           

            //cv::Mat fg;
            foreground.setTo(cv::Scalar(0, 0, 0));
            HSVFrame.setTo(cv::Scalar(0, 0, 0));
            dilateErodeMask.setTo(cv::Scalar(0,0,0));
            frame.copyTo(foreground, mask);
            

            
            cv::cvtColor(foreground, HSVFrame, cv::COLOR_BGR2HSV);
            

            // Create an HSV mask for flame colors
            cv::inRange(HSVFrame, cv::Scalar(minHue, minSat, minVal), cv::Scalar(maxHue, maxSat, maxVal), hsvMask);


           
            // cv::erode(hsvMask, dilateErodeMask, kernel, cv::Point(-1, -1), 1);
            cv::dilate(hsvMask, dMask, kernel, cv::Point(-1, -1), 2);


            std::vector<std::vector<cv::Point>> contours;
            std::vector<std::vector<cv::Point>> filteredContours;
            cv::findContours(dMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            
            //std::cout << "Number of contours found: " << contours.size() << std::endl;


            for (const auto& contour : contours) {

                int area = cv::contourArea(contour);
                
                if (area > minBBArea) {
                    filteredContours.push_back(contour);
                }

            }

            //Replace the original contours with the filtered ones
            contours = filteredContours;

            int highestY = std::numeric_limits<int>::max();

            for (const auto& contour : contours) {
                // Get the bounding box for the current contour
                cv::Rect boundingBox = cv::boundingRect(contour);
                int bottom_y = boundingBox.y + boundingBox.height;
                int top_y = boundingBox.y;

                //std::cout << bottom_y << " ";
                if (bottom_y > currY && frameCount > 10) {
                    currY = bottom_y;
                }

                if (top_y < highestY) {
                    highestY = top_y;
                    lastY = top_y;
                }

        
                

             }

            //std::cout << "NEXT" << "\n";

            if(accumulatedTime > sampleInterval) {
                std::cout << frameCount << "\n";

                fireVerticalPositions.emplace_back(seconds, currY, lastY);
                //fireLastPositions.emplace_back(seconds, lastY);

                //std::cout << " " << fireVerticalPositions() "\n";


                accumulatedTime = accumulatedTime - sampleInterval;
                seconds = seconds +1;
            }
           


            // Define the point you want to draw
            cv::Point tl(80, currY-5);  // Coordinates of the point
            cv::Point br(800, currY+5);  // Coordinates of the point

            cv::Point tl2(80, highestY-5);  // Coordinates of the point
            cv::Point br2(800, highestY+5);  // Coordinates of the point
         

            // Draw the point on the image
            cv::rectangle(frame, tl,br,cv::Scalar(0,255,0),cv::FILLED);
            cv::rectangle(frame, tl2,br2,cv::Scalar(0,200,0),cv::FILLED);

            cv::drawContours(frame, contours, -1, cv::Scalar(255, 0, 0), cv::FILLED);
            
            if (frameCount % 5 == 0) { // Show every 5th frame
                cv::imshow("cleaned mask", frame);
            }

            // Wait for 1 ms between frames (to simulate real-time playback)
            if (cv::waitKey(1) == 27) { // Exit if 'Esc' is pressed
                break;
            }

            frameCount++;
        }

        // Release video capture object and close display window
        cap.release();
        cv::destroyAllWindows();

        std::vector<std::string> cols = {"Time", "Flame Front", "Flame Back"};

        writeToCSV("FirePositions.csv", cols, fireVerticalPositions);
    }
   
}

 