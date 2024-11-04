#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui.hpp"
#include "contours.hpp"




void parseVideoContours() {
    auto const MASK_WINDOW = "Mask Settings";
    cv::namedWindow(MASK_WINDOW, cv::WINDOW_AUTOSIZE);
    // Open the video file
    std::string videoFilePath = "C:/Users/chris/WPI/JapanMQPClone/JapanMQP/data/FireSafetyVideo.mp4";
    cv::VideoCapture cap(videoFilePath);

  
    // Check if the video file was opened successfully
    if (!cap.isOpened()) {
        std::cerr << "Error: FAILED OPENING FILE" << std::endl;
        
    } else {

        // Get video frame rate
        double fps = cap.get(cv::CAP_PROP_FPS);
        std::cout << "Frames per second: " << fps << std::endl;

        // Frame-by-frame analysis
        cv::Mat frame, grayScaleFrame, binaryMask, contourMask;
        int frameCount = 0;

        // INITAIL MASK
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error: Could not read the first frame." << std::endl;
            return;
        }

        // Convert the first frame to grayscale and create the mask
        cv::cvtColor(frame, grayScaleFrame, cv::COLOR_BGR2GRAY);
        cv::threshold(grayScaleFrame, binaryMask, 200, 210, cv::THRESH_BINARY);

        // Optional: Apply morphological operations to clean the mask
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::morphologyEx(binaryMask, binaryMask, cv::MORPH_CLOSE, kernel);
        cv::morphologyEx(binaryMask, binaryMask, cv::MORPH_OPEN, kernel);

        // Create a color mask from the binary mask
        cv::Mat colorMask;
        cv::cvtColor(binaryMask, colorMask, cv::COLOR_GRAY2BGR);


        while (true) {
            // Capture each frame
            cap >> frame;

            // If the frame is empty, break the loop (end of video)
            if (frame.empty()) {
                break;
            }

            // Use the color mask to black out areas outside the white paper
            cv::Mat maskedFrame;
            frame.copyTo(maskedFrame, binaryMask);  // Copy only the area defined by the mask

            // // frame processing code goes here vvvv
            //  // Convert to grayscale
            cv::Mat grayScale_Contour, binaryMask_Contour;
            cv::cvtColor(maskedFrame, grayScale_Contour, cv::COLOR_BGR2GRAY);

            // // Apply binary thresholding to make it black and white
            cv::threshold(grayScale_Contour, binaryMask_Contour, 200, 255, cv::THRESH_BINARY);

            // // Find contours on the binary mask
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(binaryMask_Contour, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);


            // // Print information about the contours
            // std::cout << "Number of contours found: " << contours.size() << std::endl;
            // for (size_t i = 0; i < contours.size(); i++) {
            //     // Get the bounding box for the current contour
            //     cv::Rect boundingBox = cv::boundingRect(contours[i]);

            //     std::cout << "Contour " << i << ":" << std::endl;
            //     std::cout << "  Number of points: " << contours[i].size() << std::endl;
            //     std::cout << "  Bounding box: [x: " << boundingBox.x
            //             << ", y: " << boundingBox.y
            //             << ", width: " << boundingBox.width
            //             << ", height: " << boundingBox.height << "]" << std::endl;

            
            // }

            // // Create a blank mask with the same size as the frame
            contourMask = cv::Mat::zeros(frame.size(), CV_8UC1);

            // // Draw contours on the mask
            cv::drawContours(contourMask, contours, -1, cv::Scalar(170), cv::LINE_8);

             

       

            // Display the original frame and the mask
            cv::imshow("Frame", contourMask);
            

            

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

 