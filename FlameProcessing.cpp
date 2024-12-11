// need to calc the burned area

// #include <stdio.h>
// #include <fstream>
// #include <limits>
// #include <string>
// #include <opencv2/opencv.hpp>
// #include "opencv2/highgui.hpp"

#include "utils.h"
#include <qgraphicsview.h>
#include <unordered_set>




// FlameProcessing Constructor definition
FlameProcessing::FlameProcessing() {

    stopProcess = false;
    scaleClicks = 0;

    currX = 0;
    currY = 0;
    pixelsX = 0;
    pixelsY = 0;
    irlScaleX = 0.0;
    irlScaleY = 0.0;
    cmPerPixel = 0.0;
    areaPerPixel = 0.0;

    fist_point_selected = false;



    maskX = 0;
    maskY = 0;
    maskH = 0;
    maskW = 0;

    unitPerPixelX = 0;
    unitPerPixelY = 0;

}

void FlameProcessing::setIRLScale(double x, double y){
    irlScaleX = x;
    irlScaleY = y;
    //cmPerPixel =
}

void FlameProcessing::setScale() {
    cmPerPixel = irlScaleY / pixelsY;
    double cmPerPixelX = irlScaleX / pixelsX;
    areaPerPixel = cmPerPixel * cmPerPixelX;
}

void FlameProcessing::scalingMouse(int event, int x, int y, int flags) {
    if(event == 1){
        scaleClicks = scaleClicks + 1;
        currPos.x = x;
        currPos.y = y;
        //std::cout<<event<<" "<<x<<" "<<y<<" clicks:"<<scaleClicks<<"\n";
    }

    else if(event == 0) {
        currPos.x = x;
        currPos.y = y;
    }
}

// class function to set the cancel button state
void FlameProcessing::setStopProcess(bool state) {
    stopProcess = state;
}

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

// check if the file is .mp4
bool FlameProcessing::checkMP4(std::string newFile) {
    // TODO: Move to utils.cpp
    size_t dotPosition = newFile.find_last_of('.');
    if (dotPosition == std::string::npos) {
        return false;
    }
    std::string extension = newFile.substr(dotPosition);
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   ::tolower);
    return extension == ".mp4";
}

void FlameProcessing::imageROISelect(std::string videoFilePath) {
    cv::VideoCapture cap(videoFilePath);
    cv::Mat image;
    if (!cap.read(image)) {
        std::cerr << "Error: Could not read first frame!\n";
    }
    cv::resize(image, image, cv::Size(), 0.5, 0.5); // Scale down by half
    cv::Rect roi;
    roi = cv::selectROI("Selected ROI", image);
    cv::Mat imCropped;
    try {
        imCropped = image(roi);
    } catch (const cv::Exception &e) {
        return;
    }
    image = imCropped;
    maskX = roi.x;
    maskY = roi.y;
    maskH = roi.height;
    maskW = roi.width;

    cv::destroyWindow("Selected ROI");
}

void FlameProcessing::imageScaling(std::string videoFilePath, char axis) {

    cv::VideoCapture cap(videoFilePath);
    cv::Mat firstframe;
    if (!cap.read(firstframe)) {
        std::cerr << "Error: Could not read first frame!\n";
        return; // Exit if the frame cannot be read
    }

    if (firstframe.empty()) {
        std::cerr << "Error: Retrieved frame is empty!\n";
        return;
    }

    cv::resize(firstframe, firstframe, cv::Size(), 0.5, 0.5); // Scale down by half
    cv::imshow("Manual Scaling", firstframe);
    cv::setMouseCallback("Manual Scaling", FlameProcessing::mouseCallback, this);

    cv::Point startingPos(-1,-1);
    cv::Point endingPos(-1,-1);

    // Clone the image for dynamic updates
    cv::Mat tempImage;
    tempImage = firstframe.clone();

    // Wait for two points to be clicked
    while (true) {

        //tempImage = firstframe.clone();
        firstframe.copyTo(tempImage);

        if(scaleClicks == 1 && fist_point_selected == false) {
            startingPos.x = currPos.x;
            startingPos.y = currPos.y;
            fist_point_selected = true;
        }
        if (scaleClicks == 1) {
            cv::line(tempImage, startingPos, currPos, cv::Scalar(0, 255, 0), 2);
        }

        if (scaleClicks == 2) {
            endingPos.x = currPos.x;
            endingPos.y = currPos.y;

            if(axis == 'x') {
                pixelsX = abs((startingPos.x*2)-(endingPos.x*2));
            }
            else if(axis == 'y') {
                pixelsY = abs((startingPos.y*2)-(endingPos.y*2));
            }

            break;
        }
        if (cv::waitKey(1) == 27) { // Press 'ESC' to exit early
            //std::cout << "Exiting before two points were selected.\n";
            break; // Exit the loop gracefully
        }



        cv::imshow("Manual Scaling", tempImage);
    }

    scaleClicks = 0;
    fist_point_selected = false;

    cv::destroyWindow("Manual Scaling"); // Destroy the window to release resources

}


void FlameProcessing::parseVideo(std::string videoFilePath, QGraphicsView *view) {


    auto const MASK_WINDOW = "Mask Settings";
    cv::namedWindow(MASK_WINDOW, cv::WINDOW_AUTOSIZE);

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

        //std::cout << "Frames per second: " << fps << std::endl;






        int currY = -1;
        int lastY = -1;

        imageScaling(videoFilePath, 'x');
        imageROISelect(videoFilePath);
        int frameCount = 0;


        while (true) {

            if(stopProcess) {
                break;
            }

            accumulatedTime = accumulatedTime + timePerFrame;
            // Capture each frame
            cap >> frame;
            // If the frame is empty, break the loop (end of video)
            if (frame.empty()) {
                break;
            }
            cv::resize(frame, frame, cv::Size(), 0.5, 0.5); // Scale down by half
            // Create a black mask
            cv::Mat newFrame = cv::Mat::zeros(frame.size(), frame.type());
            // Define the rectangle for the mask
            cv::Rect box(maskX, maskY, maskW, maskH);
            // Copy the contents of the rectangle from the frame to the mask
            frame(box).copyTo(newFrame(box));
            // apply mask
            bg_sub->apply(newFrame, mask);

            // reinit image processing masks
            foreground.setTo(cv::Scalar(0, 0, 0));
            HSVFrame.setTo(cv::Scalar(0, 0, 0));
            dilateErodeMask.setTo(cv::Scalar(0,0,0));
            newFrame.copyTo(foreground, mask);

            // convert foreground to HSV
            cv::cvtColor(foreground, HSVFrame, cv::COLOR_BGR2HSV);

            // Create an HSV mask for flame colors
            cv::inRange(HSVFrame, cv::Scalar(minHue, minSat, minVal), cv::Scalar(maxHue, maxSat, maxVal), hsvMask);

            // dilate the flame
            cv::dilate(hsvMask, dMask, kernel, cv::Point(-1, -1), 2);

            std::vector<std::vector<cv::Point>> contours;
            std::vector<std::vector<cv::Point>> filteredContours;
            cv::findContours(dMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

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

                ////std::cout << bottom_y << " ";
                if (bottom_y > currY && frameCount > 10) {
                    currY = bottom_y;
                }

                if (top_y < highestY) {
                    highestY = top_y;
                    lastY = top_y;
                }
            }

            ////std::cout << "NEXT" << "\n";

            if(accumulatedTime > sampleInterval) {
                //std::cout << frameCount << "\n";

                fireVerticalPositions.emplace_back(seconds, currY, lastY);
                //fireLastPositions.emplace_back(seconds, lastY);

                ////std::cout << " " << fireVerticalPositions() "\n";


                accumulatedTime = accumulatedTime - sampleInterval;
                seconds = seconds +1;
            }



            // Define the point you want to draw
            cv::Point tl(80, currY-5);  // Coordinates of the point
            cv::Point br(800, currY+5);  // Coordinates of the point

            cv::Point tl2(80, highestY-5);  // Coordinates of the point
            cv::Point br2(800, highestY+5);  // Coordinates of the point


            // Draw the point on the image
            cv::rectangle(newFrame, tl,br,cv::Scalar(0,255,0),cv::FILLED);
            cv::rectangle(newFrame, tl2,br2,cv::Scalar(0,200,0),cv::FILLED);

            cv::drawContours(newFrame, contours, -1, cv::Scalar(255, 0, 0), cv::LINE_AA);

            if (frameCount % 5 == 0) { // Show every 5th frame
                // //cv::imshow("cleaned mask", frame);
                // view->scene()->clear();
                // QImage qimg = matToQImage(newFrame);

                // // Create a QPixmap from the QImage and add it to the scene

                // scene->addPixmap(QPixmap::fromImage(qimg));
                //view->setSceneRect(0, 0, qimg.width(), qimg.height());

                // Optionally set the view size to match the image size
                //view->setSceneRect(0, 0, qimg.width(), qimg.height());
                //graphicsViewHelper(view, newFrame, scene);

            }

            // Wait for 1 ms between frames (to simulate real-time playback)
            if (cv::waitKey(1) == 27) { // Exit if 'Esc' is pressed
                break;
            }

            frameCount++;
        }

        setStopProcess(false);

        // Release video capture object and close display window
        cap.release();
        cv::destroyAllWindows();

        std::vector<std::string> cols = {"Time", "Flame Front", "Flame Back"};

        writeToCSV("FirePositions.csv", cols, fireVerticalPositions);
    }

}

// Mouse callback function
void FlameProcessing::mouseCallback(int event, int x, int y, int flags, void* userdata) {
    FlameProcessing* self = static_cast<FlameProcessing*>(userdata);
    self->scalingMouse(event, x, y, flags);
}



void FlameProcessing::setROIBox(int x, int y, int h, int w) {
    maskX = x;
    maskY = y;
    maskH = h;
    maskW = w;
}

cv::Mat FlameProcessing::findContourImage(cv::Mat original_frame) {
    int minThresh = 200, maxThresh = 255;
    int blurAmount = 2;
    int minBBArea = 60;

    int minHue = 0, maxHue = 50;
    int minSat = 50, maxSat = 255;
    int minVal =  150, maxVal = 255;
    // Create a black mask
    cv::Mat newFrame = cv::Mat::zeros(original_frame.size(), original_frame.type());
    // Define the rectangle for the mask
    cv::Rect box(maskX, maskY, maskW, maskH);
    // Copy the contents of the rectangle from the frame to the mask
    original_frame(box).copyTo(newFrame(box));
    // apply mask
    bg_sub->apply(newFrame, mask);
    // reinit image processing masks
    foreground.setTo(cv::Scalar(0, 0, 0));
    HSVFrame.setTo(cv::Scalar(0, 0, 0));
    dilateErodeMask.setTo(cv::Scalar(0,0,0));
    newFrame.copyTo(foreground, mask);
    // convert foreground to HSV
    cv::cvtColor(foreground, HSVFrame, cv::COLOR_BGR2HSV);
    // Create an HSV mask for flame colors
    cv::inRange(HSVFrame, cv::Scalar(minHue, minSat, minVal), cv::Scalar(maxHue, maxSat, maxVal), hsvMask);
    // dilate the flame
    cv::dilate(hsvMask, dMask, kernel, cv::Point(-1, -1), 2);
    std::vector<std::vector<cv::Point>> contours;

    cv::findContours(dMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    for (const auto& contour : contours) {
        int area = cv::contourArea(contour);
        if (area > minBBArea) {
            filteredContours.push_back(contour);
            ////std::cout<<"removing contour\n";
        }
    }
    //Replace the original contours with the filtered ones
    contours = filteredContours;
    cv::drawContours(newFrame, filteredContours, -1, cv::Scalar(255, 0, 0), cv::FILLED);
    return newFrame;
}

// find lowest contour for each segment so that there is only one angle being found
std::vector<cv::Point> FlameProcessing::selectLowestContour(int threshold, int currentX) {
    std::vector<cv::Point> lowestC;
    int lowest = -1;
    for (const auto& contour : filteredContours) {
        cv::Rect boundingBox = cv::boundingRect(contour);
        int bottom_y = boundingBox.y + boundingBox.height;
        // loop through contour points
        for(const auto& point : contour) {
            // if contour points are in the threshold
            if(point.x > (currentX-threshold) && point.x < (currentX+threshold)) {
                if (bottom_y > lowest) {
                    lowestC = contour;
                    lowest = bottom_y;
                }
                break;
            }
        }
    }
    return lowestC;
}

// loop through the lines, for each draw a line for the angle of direction
std::vector<double> FlameProcessing::recordAngle(std::vector<double> segments, cv::Mat &image, int threshold){

    std::vector<double> angleData;
    for(int i = 0; i < segments.size(); i++) {
        double currentX = segments[i];
        cv::Point gridStart(currentX, 0);
        cv::Point gridEnd(currentX, image.rows);
        cv::line(image, gridStart, gridEnd, cv::Scalar(255, 115, 0), 2);
        int leadingY = -1;
        // selectLowestContour(threshold, filteredContours);
        // instead of filtering through each contor, find the lowest one withing the threshold
        std::vector<cv::Point> bestContour = selectLowestContour(threshold, currentX);
        //for (const auto& contour : filteredContours) {
        if(!bestContour.empty()) {
            std::vector<cv::Point> contourInfo;

            //before grouping, find the actual contour pixels
            std::vector<cv::Point> contourPixels = findContourPixels(bestContour, image);

            for(const auto& point : contourPixels) {
                if(point.x > (currentX-threshold) && point.x < (currentX+threshold)) {
                    contourInfo.emplace_back(point);
                }

            }
            // group the pixels by x value
            // find lowest y, add to edge array
            std::vector<cv::Point> leadingEdge = findLowestEdges(contourInfo);
            drawLowestEdges(image, leadingEdge, cv::Scalar(0,0,255), 3, -1);
            // do fitLine on that array
            // draw line
            if(!contourInfo.empty()) {
                cv::Vec4f line;
                cv::fitLine(leadingEdge, line, cv::DIST_L2, 0, 0.01, 0.01);
                float vx = line[0];//vector x component
                float vy = line[1];//vector y component
                float x0 = line[2];
                float y0 = line[3];
                //std::cout << "Contour Angle: ";
                // calc angle here, use x = 1 and y = 0 for regular x axis
                double angle = calcLOBFAngle(vx, vy, 1.0, 0.0);
                //std::cout<<angle<<std::endl;
                angleData.push_back(angle);

                // Calculate the endpoints of the line (20px long)
                float halfLength = threshold; // Half of the line length (20px total)
                cv::Point start(x0 - vx * halfLength, y0 - vy * halfLength);
                cv::Point end(x0 + vx * halfLength, y0 + vy * halfLength);
                // Draw the line on the image
                cv::line(image, start, end, cv::Scalar(0, 255, 0), 2);
            }

        } else {
            angleData.push_back(0);
        }

    }
    filteredContours.clear();
    return angleData;

}

//calc angle from horizontal axis to line of best fit
double FlameProcessing::findAngle(double vx, double vy, double refVx, double refVy) {
    double magnitude = std::sqrt(vx * vx + vy * vy);
    double normVx = vx / magnitude;
    double normVy = vy / magnitude;
    double dotProduct = normVx * refVx + normVy * refVy;
    double crossProduct = normVx * refVy - normVy * refVx;
    double angleRad = std::atan2(crossProduct, dotProduct);
    double angleDeg = angleRad * 180.0 / CV_PI;
    return angleDeg;
}


// loop through lines, find lowest and closest point to each line between threshold
std::vector<double> FlameProcessing::recordPositions(std::vector<double> segments){
    int threshold = 5;
    std::vector<double> posData; // array of each line
    //loop through all segments
    for(int i = 0; i < segments.size(); i++) {
        double currentX = segments[i];
        int leadingY = -1;
        // loop through all contours
        for (const auto& contour : filteredContours) {
            //loop through each contour pixel
            for (const auto& point : contour) {
                if(point.x > (currentX-threshold) && point.x < (currentX+threshold) && point.y > leadingY) {
                    leadingY = point.y;
                }
            }
        }
        posData.push_back(leadingY);
        std::cout<<leadingY<<" ";
    }
    filteredContours.clear();
    std::cout<<""<<std::endl;
    return posData;
}

std::vector<std::vector<double>> FlameProcessing::cleanData(std::vector<std::vector<double>> positions) {
    int numOfLines = positions[0].size();
    std::vector<std::vector<double>> allLines;
    //std::cout<<"CM PER PIXEL:" << cmPerPixel << std::endl;
    for (int line = 0; line < numOfLines; line++) {
        std::vector<double> cleanedPositions;
        bool firstDataEncountered = false;
        double last_known_position = 0.0; // Initialize position at 0 cm
        double initialPx = 0;
        //loop through all frames
        for (int frame = 0; frame < positions.size(); frame++) {
            int curr = positions[frame][line];
            if (curr == -1) {
                // If no new data, keep the last position
                cleanedPositions.push_back(last_known_position);
            } else if (curr != -1 && !firstDataEncountered) {
                initialPx = curr;
                cleanedPositions.push_back(last_known_position); // keep as 0 for the first one
                firstDataEncountered = true;
            } else if (curr != -1 && firstDataEncountered) {
                last_known_position = (curr - initialPx) * cmPerPixel;
                cleanedPositions.push_back(last_known_position);
            }
            std::cout<<"Line[" << line << "]:" << cleanedPositions[frame]<< ", with px:" << positions[frame][line]<<std::endl;
        }
        std::cout << "NEXT DATA SEGMENT" << std::endl;
        allLines.emplace_back(cleanedPositions);
    }
    return allLines;
}


std::vector<cv::Point> FlameProcessing::findLowestEdges(std::vector<cv::Point> contourInfo) {
    std::vector<cv::Point> edgePoints;
    std::map<int, std::vector<cv::Point>> xMap;

    for (const cv::Point& pt : contourInfo) {
        xMap[pt.x].push_back(pt); // Group points with the same x
    }

    for (const auto& [x, points] : xMap) {
        cv::Point edgePt(-1,-1);
        for (const cv::Point& pt : points) {
            if(pt.y > edgePt.y) {
                edgePt.y = pt.y;
                edgePt.x = pt.x;
            }
        }

        edgePoints.push_back(edgePt);

    }

    return edgePoints;


}

// std::vector<cv::Point> FlameProcessing::findLowestEdges(std::vector<cv::Point> contourInfo) {
//     std::vector<cv::Point> edgePoints;
//     std::map<int, std::vector<cv::Point>> xMap;

//     // Group points by x-coordinate
//     for (const cv::Point& pt : contourInfo) {
//         xMap[pt.x].push_back(pt); // Group points with the same x
//     }

//     // Process each group
//     for (const auto& [x, points] : xMap) {
//         //std::cout << "Processing x = " << x << ":\n";

//         cv::Point edgePt(-1, -1); // Initialize with invalid point

//         for (const cv::Point& pt : points) {
//             //std::cout << "  Considering point (" << pt.x << ", " << pt.y << ") ";
//             if (pt.y > edgePt.y) {
//                 edgePt = pt; // Update to the point with the maximum y
//                 //std::cout << "-> New edge point selected.\n";
//             } else {
//                 //std::cout << "-> Not selected.\n";
//             }
//         }

//         if (edgePt.x != -1) { // Valid point found
//             edgePoints.push_back(edgePt);
//             //std::cout << "  Final edge point for x = " << x << ": ("
//                       << edgePt.x << ", " << edgePt.y << ")\n";
//         } else {
//             ////std::cout << "  No valid points found for x = " << x << ".\n";
//         }
//     }

//     return edgePoints;
// }

void FlameProcessing::drawLowestEdges(cv::Mat& image,const std::vector<cv::Point>& edgePoints, const cv::Scalar& color, int radius, int thickness) {
    for (const cv::Point& pt : edgePoints) {
        if (pt.x >= 0 && pt.y >= 0 && pt.x < image.cols && pt.y < image.rows) {
            cv::circle(image, pt, radius, color, thickness);
        } else {
            std::cerr << "Point (" << pt.x << ", " << pt.y << ") is outside the image bounds!" << std::endl;
        }
    }
}


// takes a contour and returns all pixels found within the contour
std::vector<cv::Point> FlameProcessing::findContourPixels(std::vector<cv::Point> contour, cv::Mat image) {
    cv::Mat mask = cv::Mat::zeros(image.size(), CV_8UC1);
    cv::drawContours(mask, std::vector<std::vector<cv::Point>>{contour}, -1, cv::Scalar(255), cv::FILLED);
    //cv::imshow("LOOK", mask);
    std::vector<cv::Point> whitePixels;
    CV_Assert(mask.type() == CV_8UC1);
    for (int y = 0; y < mask.rows; ++y) {
        for (int x = 0; x < mask.cols; ++x) {
            if (mask.at<uchar>(y, x) == 255) {
                whitePixels.emplace_back(x, y);
            }
        }
    }
    return whitePixels;
}



cv::Mat FlameProcessing::findContourMask(cv::Mat original_frame, cv::Mat newFrame, cv::Mat contourMask) {

    //std::cout << "HERE!!" << std::endl;
    int minThresh = 200, maxThresh = 255;
    int blurAmount = 2;
    int minBBArea = 60;

    int minHue = 0, maxHue = 50;
    int minSat = 50, maxSat = 255;
    int minVal =  150, maxVal = 255;
    burnedAreaPixels.clear();
    // Create a black mask

    // Define the rectangle for the mask
    cv::Rect box(maskX, maskY, maskW, maskH);
    // Copy the contents of the rectangle from the frame to the mask
    original_frame(box).copyTo(newFrame(box));
    // apply mask
    //bg_sub->apply(newFrame, mask);
    // reinit image processing masks
    foreground.setTo(cv::Scalar(0, 0, 0));
    HSVFrame.setTo(cv::Scalar(0, 0, 0));
    dilateErodeMask.setTo(cv::Scalar(0,0,0));
    newFrame.copyTo(foreground);
    // convert foreground to HSV
    cv::cvtColor(foreground, HSVFrame, cv::COLOR_BGR2HSV);
    // Create an HSV mask for flame colors
    cv::inRange(HSVFrame, cv::Scalar(minHue, minSat, minVal), cv::Scalar(maxHue, maxSat, maxVal), hsvMask);
    // dilate the flame
    cv::dilate(hsvMask, dMask, kernel, cv::Point(-1, -1), 2);
    std::vector<std::vector<cv::Point>> contours;

    cv::findContours(dMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    // for (const auto& contour : contours) {
    //     int area = cv::contourArea(contour);

    //     //std::vector<cv::Point> contourPixels = findContourPixels(contour, original_frame);
    //     //addNewPixels(contourPixels);



    //     // if (area > minBBArea) {
    //     //     filteredContours.push_back(contour);

    //     // }
    // }

    // if(!burnedAreaPixels.empty()) {
    //     if (contourMask.channels() == 1) {
    //         for (const auto& pixel : burnedAreaPixels) {
    //             contourMask.at<uchar>(pixel.y, pixel.x) = 255; // Set pixel to white
    //         }
    //     }

    //     else if (contourMask.channels() == 3) {
    //         for (const auto& pixel : burnedAreaPixels) {
    //             contourMask.at<cv::Vec3b>(pixel.y, pixel.x) = cv::Vec3b(255, 255, 255); // Set pixel to white
    //         }
    //     } else {
    //         std::cerr << "Unsupported number of channels in the image!" << std::endl;
    //     }
    // }

    cv::drawContours(contourMask, contours, -1, cv::Scalar(255,255,255), cv::FILLED);


    return contourMask;
}




// Define a hash function for cv::Point to use with std::unordered_set
struct PointHash {
    std::size_t operator()(const cv::Point& point) const {
        return std::hash<int>()(point.x) ^ (std::hash<int>()(point.y) << 1);
    }
};

void FlameProcessing::addNewPixels(std::vector<cv::Point> contourPixels) {
    // Use a static unordered_set to track unique points efficiently
    static std::unordered_set<cv::Point, PointHash> burnedAreaSet;

    for (const auto& pixel : contourPixels) {
        burnedAreaSet.insert(pixel); // Add pixel to the set (duplicates are ignored)
    }

    // Clear burnedAreaPixels and update it with the unique points
    burnedAreaPixels.assign(burnedAreaSet.begin(), burnedAreaSet.end());
}


double FlameProcessing::calculateArea(cv::Mat areaImg) {
    if (areaImg.empty() || areaImg.channels() != 3) {
        std::cerr << "Invalid input image! Ensure it's a non-empty 3-channel image." << std::endl;
        //return;
    }

    int whitePixelCount = 0;
    for (int row = 0; row < areaImg.rows; row++) {
        for (int col = 0; col < areaImg.cols; col++) {
            cv::Vec3b pixel = areaImg.at<cv::Vec3b>(row, col);
            if (pixel[0] == 255 && pixel[1] == 255 && pixel[2] == 255) {
                whitePixelCount++;
            }
        }
    }
    double realArea = areaPerPixel * whitePixelCount;
    return realArea;
}
