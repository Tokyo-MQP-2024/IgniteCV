#include <stdio.h>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui.hpp"
#include "FrameAnalyzer.hpp"



void writeToCSV(std::string col1, std::string col2, std::vector<std::pair<int,int>> &data) {
      // Create an output filestream object

   // std::cout << "HERE????????????";
    std::ofstream myFile("FirePositions.csv");
    
    // Send data to the stream
    myFile << col1 << "," << col2 << "\n";

    for (const auto &entry : data) {
        myFile << entry.first << ", " << entry.second << "\n";
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

    std::vector<std::pair<int, int>> fireVerticalPositions;

    // Create trackbars in mask settings window
    // cv::createTrackbar("Min Hue", MASK_WINDOW, &minHue, 179);
    // cv::createTrackbar("Max Hue", MASK_WINDOW, &maxHue, 179);
    // cv::createTrackbar("Min Sat", MASK_WINDOW, &minSat, 255);
    // cv::createTrackbar("Max Sat", MASK_WINDOW, &maxSat, 255);
    // cv::createTrackbar("Min Val", MASK_WINDOW, &minVal, 255);
    // cv::createTrackbar("Max Val", MASK_WINDOW, &maxVal, 255);

    // cv::createTrackbar("Min THRESHOLD", MASK_WINDOW, &minThresh, 255);
    cv::createTrackbar("Contr", MASK_WINDOW, &minBBArea, 300);

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
        //cv::Ptr<cv::BackgroundSubtractor> bg_sub = cv::createBackgroundSubtractorMOG2();
        cv::Ptr<cv::BackgroundSubtractor> bg_sub = cv::createBackgroundSubtractorKNN();




        int currY = -1;

        while (true) {

            accumulatedTime = accumulatedTime + timePerFrame;


            // if(accumulatedTime > sampleInterval) {
            //     std::cout << frameCount << "\n";
            //     accumulatedTime = accumulatedTime - sampleInterval;
            //     seconds++;
            // }


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

             for (const auto& contour : contours) {
                // Get the bounding box for the current contour
                cv::Rect boundingBox = cv::boundingRect(contour);
                int bottom_y = boundingBox.y + boundingBox.height;
                //std::cout << bottom_y << " ";
                if (bottom_y > currY && frameCount > 10) {
                    currY = bottom_y;
                }

             }

            //std::cout << "NEXT" << "\n";

            if(accumulatedTime > sampleInterval) {
                std::cout << frameCount << "\n";

                fireVerticalPositions.emplace_back(seconds, currY);

                //std::cout << " " << fireVerticalPositions() "\n";


                accumulatedTime = accumulatedTime - sampleInterval;
                seconds = seconds +1;
            }
           


            // Define the point you want to draw
            cv::Point tl(80, currY-5);  // Coordinates of the point
            cv::Point br(800, currY+5);  // Coordinates of the point
         

            // Draw the point on the image
            //cv::circle(frame, point, 2, cv::Scalar(0, 255, 0), cv::FILLED);  // Green point with radius 2
            cv::rectangle(frame, tl,br,cv::Scalar(0,255,0),cv::FILLED);

            


            cv::drawContours(frame, contours, -1, cv::Scalar(255, 0, 0), cv::FILLED);




            
            if (frameCount % 5 == 0) { // Show every 5th frame
                //cv::imshow("Frame", dMask);
                cv::imshow("cleaned mask", frame);
                // cv::imshow("erode", eMask);
                // cv::imshow("dilate", dMask);
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

        writeToCSV("Time", "Position", fireVerticalPositions);
    }
   
}

 














// #include <stdio.h>
// #include <opencv2/opencv.hpp>
// #include "opencv2/highgui.hpp"
// #include "FrameAnalyzer.hpp"




// void parseVideo() {
//     auto const MASK_WINDOW = "Mask Settings";
//     cv::namedWindow(MASK_WINDOW, cv::WINDOW_AUTOSIZE);
//     // Open the video file
//     std::string videoFilePath = "C:/Users/chris/WPI/JapanMQPClone/JapanMQP/data/FireSafetyVideo.mp4";
//     cv::VideoCapture cap(videoFilePath);

//     // HSV range to detect flame
//     int minHue = 5, maxHue = 35;
//     int minSat = 50, maxSat = 255;
//     int minVal =  200, maxVal = 255;

//     // Create trackbars in mask settings window
//     cv::createTrackbar("Min Hue", MASK_WINDOW, &minHue, 179);
//     cv::createTrackbar("Max Hue", MASK_WINDOW, &maxHue, 179);
//     cv::createTrackbar("Min Sat", MASK_WINDOW, &minSat, 255);
//     cv::createTrackbar("Max Sat", MASK_WINDOW, &maxSat, 255);
//     cv::createTrackbar("Min Val", MASK_WINDOW, &minVal, 255);
//     cv::createTrackbar("Max Val", MASK_WINDOW, &maxVal, 255);

//     // Check if the video file was opened successfully
//     if (!cap.isOpened()) {
//         std::cerr << "Error: FAILED OPENING FILE" << std::endl;
        
//     } else {

//         // Get video frame rate
//         double fps = cap.get(cv::CAP_PROP_FPS);
//         std::cout << "Frames per second: " << fps << std::endl;

//         // Frame-by-frame analysis
//         cv::Mat frame, grayScaleFrame, HSVFrame;
//         int frameCount = 0;

//         while (true) {
//             // Capture each frame
//             cap >> frame;

//             // If the frame is empty, break the loop (end of video)
//             if (frame.empty()) {
//                 break;
//             }

//             // Process the frame (you can add your analysis here)
//             std::cout << "Processing frame #" << frameCount << std::endl;

//            //// 3. Capture and convert frame to HSV color space
//             cv::cvtColor(frame, HSVFrame, cv::COLOR_BGR2HSV);
//             //cv::cvtColor(frame, grayScaleFrame, cv::COLOR_RGB2GRAY);

//              //// 3. Create mask and result (masked) image
//             cv::Mat mask;
//             // params: input array, lower boundary array, upper boundary array, output array
//             cv::inRange(
//                 HSVFrame, 
//                 cv::Scalar(minHue, minSat, minVal), 
//                 cv::Scalar(maxHue, maxSat, maxVal), 
//                 mask
//             );
//             cv::Mat resultImage;
//             // params: src1	array, src2 array, output array, mask
//             cv::bitwise_and(HSVFrame, HSVFrame, resultImage, mask);
            
//             // Example: Display the frame
//             cv::imshow("Frame", resultImage);

//             // Wait for 1 ms between frames (to simulate real-time playback)
//             if (cv::waitKey(1) == 27) { // Exit if 'Esc' is pressed
//                 break;
//             }

//             frameCount++;
//         }

//         // Release video capture object and close display window
//         cap.release();
//         cv::destroyAllWindows();
//     }
   
// }

 