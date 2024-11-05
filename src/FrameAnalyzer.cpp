#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui.hpp"
#include "FrameAnalyzer.hpp"




void parseVideo() {
    // auto const MASK_WINDOW = "Mask Settings";
    // cv::namedWindow(MASK_WINDOW, cv::WINDOW_AUTOSIZE);
    // Open the video file
    std::string videoFilePath = "C:/Users/chris/WPI/JapanMQPClone/JapanMQP/data/FireSafetyVideo.mp4";
    cv::VideoCapture cap(videoFilePath);

    // HSV range to detect blue color
    int minHue = 0, maxHue = 50;
    int minSat = 50, maxSat = 255;
    int minVal =  150, maxVal = 255;

    int minThresh = 200, maxThresh = 255;

    int blurAmount = 2;

    // Create trackbars in mask settings window
    // cv::createTrackbar("Min Hue", MASK_WINDOW, &minHue, 179);
    // cv::createTrackbar("Max Hue", MASK_WINDOW, &maxHue, 179);
    // cv::createTrackbar("Min Sat", MASK_WINDOW, &minSat, 255);
    // cv::createTrackbar("Max Sat", MASK_WINDOW, &maxSat, 255);
    // cv::createTrackbar("Min Val", MASK_WINDOW, &minVal, 255);
    // cv::createTrackbar("Max Val", MASK_WINDOW, &maxVal, 255);

    // cv::createTrackbar("Min THRESHOLD", MASK_WINDOW, &minThresh, 255);
    // cv::createTrackbar("Max THRESHOLD", MASK_WINDOW, &maxThresh, 255);

    // Check if the video file was opened successfully
    if (!cap.isOpened()) {
        std::cerr << "Error: FAILED OPENING FILE" << std::endl;
        
    } else {

        // Get video frame rate
        double fps = cap.get(cv::CAP_PROP_FPS);
        std::cout << "Frames per second: " << fps << std::endl;

        // FRAME VARIABES
        cv::Mat frame, HSVFrame, foreground, mask, hsvMask, resultImage, dilateErodeMask;
        cv::Mat dMask, eMask;

        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3));
        int frameCount = 0;
        cv::Ptr<cv::BackgroundSubtractor> bg_sub = cv::createBackgroundSubtractorMOG2();


        while (true) {
            // Capture each frame
            cap >> frame;

            // If the frame is empty, break the loop (end of video)
            if (frame.empty()) {
                break;
            }

            cv::resize(frame, frame, cv::Size(), 0.5, 0.5); // Scale down by half

            

           
            
            // Example: Display the frame
            // cv::Mat mask;
            // int adjustedBlurAmount = blurAmount * 2 + 1;
            // cv::GaussianBlur(frame, frame, cv::Size(adjustedBlurAmount, adjustedBlurAmount), 0);
            bg_sub->apply(frame, mask);
           

            //cv::Mat fg;
            foreground.setTo(cv::Scalar(0, 0, 0));
            HSVFrame.setTo(cv::Scalar(0, 0, 0));
            dilateErodeMask.setTo(cv::Scalar(0,0,0));
            frame.copyTo(foreground, mask);
            

            
            cv::cvtColor(foreground, HSVFrame, cv::COLOR_BGR2HSV);
            

            // Create an HSV mask for flame colors
            cv::inRange(HSVFrame, cv::Scalar(minHue, minSat, minVal), cv::Scalar(maxHue, maxSat, maxVal), hsvMask);


            // Detect contours
            std::vector<std::vector<cv::Point>> contours;
            std::vector<cv::Vec4i> hierarchy;
            cv::findContours(mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

            // Display contours on a copy of the mask image
            cv::Mat contourImage = cv::Mat::zeros(mask.size(), CV_8UC3); // Create a blank image for displaying contours
            cv::drawContours(contourImage, contours, -1, cv::Scalar(0, 255, 0), 2); // Draw contours in green color


           
            // cv::erode(hsvMask, dilateErodeMask, kernel, cv::Point(-1, -1), 1);
            // cv::dilate(dilateErodeMask, dilateErodeMask, kernel, cv::Point(-1, -1), 1);


            
            //if (frameCount % 5 == 0) { // Show every 5th frame
                cv::imshow("Frame", hsvMask);
                cv::imshow("cleaned mask", frame);
                // cv::imshow("erode", eMask);
                // cv::imshow("dilate", dMask);
            //}
            

            

            // Wait for 1 ms between frames (to simulate real-time playback)
            if (cv::waitKey(1) == 27) { // Exit if 'Esc' is pressed
                break;
            }

            frameCount++;
        }

        // Release video capture object and close display window
        cap.release();
        cv::destroyAllWindows();
    }
   
}

 