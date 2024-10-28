#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui.hpp"
#include "FrameAnalyzer.hpp"




void parseVideo() {
    auto const MASK_WINDOW = "Mask Settings";
    cv::namedWindow(MASK_WINDOW, cv::WINDOW_AUTOSIZE);
    // Open the video file
    std::string videoFilePath = "C:/Users/chris/WPI/JapanMQPClone/JapanMQP/data/FireSafetyVideo.mp4";
    cv::VideoCapture cap(videoFilePath);

    // HSV range to detect blue color
    int minHue = 0, maxHue = 32;
    int minSat = 50, maxSat = 255;
    int minVal =  180, maxVal = 255;

    // Create trackbars in mask settings window
    cv::createTrackbar("Min Hue", MASK_WINDOW, &minHue, 179);
    cv::createTrackbar("Max Hue", MASK_WINDOW, &maxHue, 179);
    cv::createTrackbar("Min Sat", MASK_WINDOW, &minSat, 255);
    cv::createTrackbar("Max Sat", MASK_WINDOW, &maxSat, 255);
    cv::createTrackbar("Min Val", MASK_WINDOW, &minVal, 255);
    cv::createTrackbar("Max Val", MASK_WINDOW, &maxVal, 255);

    // Check if the video file was opened successfully
    if (!cap.isOpened()) {
        std::cerr << "Error: FAILED OPENING FILE" << std::endl;
        
    } else {

        // Get video frame rate
        double fps = cap.get(cv::CAP_PROP_FPS);
        std::cout << "Frames per second: " << fps << std::endl;

        // Frame-by-frame analysis
        cv::Mat frame, HSVFrame;
        int frameCount = 0;

        while (true) {
            // Capture each frame
            cap >> frame;

            // If the frame is empty, break the loop (end of video)
            if (frame.empty()) {
                break;
            }

            // Process the frame (you can add your analysis here)
            std::cout << "Processing frame #" << frameCount << std::endl;

           //// 3. Capture and convert frame to HSV color space
            cv::cvtColor(frame, HSVFrame, cv::COLOR_BGR2HSV);
            //cv::cvtColor(frame, grayScaleFrame, cv::COLOR_RGB2GRAY);

             //// 3. Create mask and result (masked) image
            cv::Mat mask;
            // params: input array, lower boundary array, upper boundary array, output array
            cv::inRange(
                HSVFrame, 
                cv::Scalar(minHue, minSat, minVal), 
                cv::Scalar(maxHue, maxSat, maxVal), 
                mask
            );
            cv::Mat resultImage;
            // params: src1	array, src2 array, output array, mask
            cv::bitwise_and(HSVFrame, HSVFrame, resultImage, mask);
            
            // Example: Display the frame
            cv::imshow("Frame", resultImage);

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

 