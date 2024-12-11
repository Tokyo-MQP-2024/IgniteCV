#include "scalingtool.h"
#include "ui_scalingtool.h"
#include "mainwindow.h"





ScalingTool::ScalingTool(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScalingTool)
{
    ui->setupUi(this);
    manual_scaleX = 0.0;
    manual_scaleY = 0.0;
    ui->ROIButton->setDisabled(false); //change later
    ui->BeginButton->setDisabled(true);
    ui->stackedWidget->setCurrentIndex(0);

    //other init stuff
    ui->BlackSlider->setMinimum(0);
    ui->BlackSlider->setMaximum(255);
    ui->WhiteSlider->setMinimum(0);
    ui->WhiteSlider->setMaximum(255);
    ui->WhiteSlider->setValue(255);
    updateNumericalLabel(ui->WhiteVal, 255);

    ui->Accumulator->setMaximum(15);
    ui->CannySlider->setValue(canny);
    ui->CannySlider->setMaximum(255);
    ui->MinRadSlider->setValue(minRad);
    ui->MaxRadSlider->setValue(maxRad);

    updateNumericalLabel(ui->CannyLabel, canny);
    updateNumericalLabel(ui->minLabel, minRad);
    updateNumericalLabel(ui->MaxLabel, maxRad);


    std::cout<<maxHue;
    ui->HMax->setMaximum(179);
    ui->HMin->setMaximum(278);
    ui->HMax->setValue(maxHue);

    ui->SMax->setMaximum(255);
    ui->SMax->setValue(maxSat);
    ui->SMin->setValue(minSat);
    ui->SMin->setMaximum(254);

    ui->VMax->setMaximum(255);
    ui->VMin->setMaximum(254);
    ui->VMin->setValue(minVal);
    ui->VMax->setValue(maxVal);



    angleThreshold = 40;

    areaFrameThreshold = 50;
    ui->FrameThreshSlider->setMaximum(400);
    ui->FrameThreshSlider->setValue(areaFrameThreshold);
    updateNumericalLabel(ui->FrameThreshLabel, areaFrameThreshold);

    //setupHistogram();
    //ui->WhiteVal->setText();


}

void ScalingTool::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        // Update the UI elements to the new language
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

ScalingTool::~ScalingTool()
{
    delete ui;
}

void ScalingTool::on_manualButton_clicked()
{
    // int index = ui->stackedWidget->indexOf(scalingTool);
    // ui->stackedWidget->setCurrentIndex(index);
    ui->stackedWidget->setCurrentIndex(0);
}


void ScalingTool::on_AutoDetectButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}




void ScalingTool::on_pushButton_clicked()
{
    flame_process->imageScaling(videoFilePath, 'x');
    scaleXDone = true;

    if (scaleXDone && scaleYDone && editXDone && editYDone) {
        ui->ROIButton->setDisabled(false);
    }
}
void ScalingTool::on_scaleYButton_clicked()
{
    flame_process->imageScaling(videoFilePath, 'y');
    scaleYDone = true;
    if (scaleXDone && scaleYDone && editXDone && editYDone) {
        ui->ROIButton->setDisabled(false);
    }

}


void ScalingTool::on_scaleX_textEdited(const QString &arg1)
{
    //QString input = ui->ScaleXEdit->text();
    //manual_scaleX = ui->ScaleXEdit->text().toDouble();
}

void ScalingTool::on_ScaleYEdit_textEdited(const QString &arg1)
{
    //QString input = ui->ScaleYEdit->text();
    //manual_scaleY = ui->ScaleYEdit->text().toDouble();
}


void ScalingTool::on_comboBox_currentIndexChanged(int index)
{
    std::cout << "cuurent index:" << index << "\n";
}


void ScalingTool::on_ROIButton_clicked()
{
    manual_scaleX = ui->ScaleXEdit->text().toDouble();
    manual_scaleY = ui->ScaleYEdit->text().toDouble();



    flame_process->setIRLScale(manual_scaleX, manual_scaleY);

    imageROISelect(videoFilePath);

    ui->BeginButton->setDisabled(false);

}

void ScalingTool::imageROISelect(std::string vf) {
    cv::VideoCapture cap(vf);
    cv::Mat image;
    if (!cap.read(image)) {
        std::cerr << "Error: Could not read first frame!\n";
    }
    //cv::resize(image, image, cv::Size(), 0.5, 0.5); // Scale down by half
    cv::Rect roi;


    cv::Mat imgCopy;
    image.copyTo(imgCopy);
    cv::resize(imgCopy,  imgCopy, cv::Size(), 0.5, 0.5); // Scale down by half

    roi = cv::selectROI("Selected ROI", imgCopy);
    cv::Mat imCropped;
    roi.x = roi.x*2;
    roi.y = roi.y*2;
    roi.height = roi.height*2;
    roi.width = roi.width*2;
    try {
        imCropped = image(roi);
    } catch (const cv::Exception &e) {
        return;
    }
    image = imCropped;
    croppedFrame = imCropped;

    // maskX = roi.x*2;
    // maskY = roi.y*2;
    // maskH = roi.height*2;
    // maskW = roi.width*2;
    maskX = roi.x;
    maskY = roi.y;
    maskH = roi.height;
    maskW = roi.width;

    flame_process->setROIBox(maskX, maskY, maskH, maskW);

    roiSelected = true;

    cv::Mat frame;
    if (globalCap.read(frame)) {
        applyROIMask(frame);
        frame.copyTo(currSelectFrame);
    }
    graphicsViewHelper(ui->FileViewWindow, frame, scene);



    cv::destroyWindow("Selected ROI");
}

void ScalingTool::applyROIMask(cv::Mat &frame) {
    // Define the rectangle for the mask
    cv::Rect box(maskX, maskY, maskW, maskH);
    // Create a temporary frame to hold the masked region
    cv::Mat tempFrame = cv::Mat::zeros(frame.size(), frame.type());
    // Copy the contents of the rectangle from the original frame to the temporary frame
    frame(box).copyTo(tempFrame(box));
    // Update the original frame with the masked frame
    tempFrame.copyTo(frame);
}




void ScalingTool::on_ScaleXEdit_editingFinished()
{
    editXDone = true;
    if (scaleXDone && scaleYDone && editXDone && editYDone) {
        ui->BeginButton->setDisabled(false);
    }
}


void ScalingTool::on_ScaleYEdit_editingFinished()
{
    editYDone = true;
    if (scaleXDone && scaleYDone && editXDone && editYDone) {
        ui->BeginButton->setDisabled(false);
    }
}


void ScalingTool::on_BeginButton_clicked()
{
    //flame_process->parseVideo()
    flame_process->setScale();
    ui->EditPanel->setCurrentIndex(1);
}


void ScalingTool::on_manualButton2_clicked()
{
    //ui->EditingStack
    ui->EditPanel->setCurrentIndex(0);
}


void ScalingTool::on_AutoButton2_clicked()
{
    ui->EditPanel->setCurrentIndex(1);
}



// FILE UPLOAD FUNCTION
void ScalingTool::on_pushButton_2_clicked()
{
    stopProcess = false;
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open a File"), "C://");
    std::string filePathSTD = filePath.toStdString();
    flame_process = new FlameProcessing(); // create new instance of flame process
    if(flame_process->checkMP4(filePathSTD)) {
        ui->FileLabel->setText(filePath);
        videoFilePath = filePathSTD;
        // QGraphicsScene *scene = new QGraphicsScene();
        // ui->FileViewWindow->setScene(scene);
        cv::VideoCapture cap(videoFilePath);
        cv::Mat frame1;
        cap >> frame1;

        frame1.copyTo(currSelectFrame);

        globalCap.open(videoFilePath);

        graphicsViewHelper(ui->FileViewWindow, frame1, scene);


        int totalFrames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
        ui->VideoScroll->setRange(0, totalFrames - 1);

    } else {
        ui->FileLabel->setText("File must be .mp4");
    }
}


void ScalingTool::on_VideoScroll_actionTriggered(int action)
{
    //std::cout << action <<"\n";
}


void ScalingTool::on_VideoScroll_valueChanged(int value)
{
    globalCap.set(cv::CAP_PROP_POS_FRAMES, value);
    cv::Mat frame;

    if (globalCap.read(frame)) {
        if(roiSelected) {
            applyROIMask(frame);
            graphicsViewHelper(ui->FileViewWindow, frame, scene);

        } else {
            graphicsViewHelper(ui->FileViewWindow, frame, scene);
        }

        frame.copyTo(currSelectFrame);
    }

}



// Circle detection button (Deprecated function)
void ScalingTool::on_pushButton_7_clicked()
{
    std::cout << "DETETCING CIRCLES\n";
    std::vector<cv::Vec3f> circles;
    cv::imshow("TEST", croppedFrame);
    //detectCircles(croppedFrame, circles);

    cv::imshow("cirles", croppedFrame);
}


// Open circle detect edit panel
void ScalingTool::on_pushButton_4_clicked()
{
    if(!levelsIMG.empty()) {
        cv::Mat newImg;
        levelsIMG.copyTo(newImg);
        std::cout << "DETETCING CIRCLES\n";
        std::vector<cv::Vec3f> circles;
        detectCircles(newImg, circles, minRad, maxRad, canny, accumulator);
        dataSegments = createGridlines(newImg, circles);
        graphicsViewHelper(ui->FileViewWindow, newImg, scene);
    }

    // if(!levelsIMG.empty()) {
    //     cv::Mat newImg;
    //     currSelectFrame.copyTo(levelsIMG);

    //     std::cout << "DETETCING CIRCLES\n";
    //     std::vector<cv::Vec3f> circles;
    //     detectCircles(newImg, circles, minRad, maxRad, canny, accumulator);

    //     createGridlines(newImg, circles);
    //     graphicsViewHelper(ui->FileViewWindow, newImg, scene);
    // }

}



void ScalingTool::adjustLevels(cv::Mat image) {
    double gamma = 1.0;
    image.copyTo(levelsIMG);
    if (clipBlack >= clipWhite) {
        std::cerr << "Error: Black level must be less than white level!" << std::endl;
    } else {
        // Loop through each pixel
        for (int row = 0; row < levelsIMG.rows; ++row) {
            for (int col = 0; col < levelsIMG.cols; ++col) {
                // Access each color channel for the pixel (BGR format)
                cv::Vec3b& pixel = levelsIMG.at<cv::Vec3b>(row, col);
                // Adjust each channel: Blue, Green, Red
                for (int channel = 0; channel < 3; ++channel) {
                    uchar& colorValue = pixel[channel];
                    // Normalize to [0, 1]
                    float normalized = (colorValue - clipBlack) / static_cast<float>(clipWhite - clipBlack);
                    normalized = std::clamp(normalized, 0.0f, 1.0f); // Clamp to [0, 1]
                    // Apply gamma correction
                    float corrected = std::pow(normalized, 1.0 / gamma);
                    // Scale back to [0, 255]
                    colorValue = static_cast<uchar>(corrected * 255.0f);
                }
            }
        }
        graphicsViewHelper(ui->FileViewWindow, levelsIMG, scene);
    }
}




void ScalingTool::on_BlackSlider_valueChanged(int value)
{

    clipBlack = value;
    QString valSTR = QString::number(value);
    ui->BlackVal->setText(valSTR);
    if(!currSelectFrame.empty()) {
        adjustLevels(currSelectFrame);
        //graphicsViewHelper(ui->FileViewWindow, flame_process, croppedFrame);
    }


}


void ScalingTool::on_WhiteSlider_valueChanged(int value)
{
    clipWhite = value;
    updateNumericalLabel(ui->WhiteVal, value);
    if(!currSelectFrame.empty()) {
        adjustLevels(currSelectFrame);
        //graphicsViewHelper(ui->FileViewWindow, flame_process, croppedFrame);
    }
    //graphicsViewHelper(ui->FileViewWindow, flame_process, croppedFrame);
}


void ScalingTool::on_MinRadSlider_valueChanged(int value)
{
    minRad = value;
    updateNumericalLabel(ui->minLabel, value);
}


void ScalingTool::on_MaxRadSlider_valueChanged(int value)
{
    maxRad = value;
    updateNumericalLabel(ui->MaxLabel, value);
}


void ScalingTool::on_CannySlider_valueChanged(int value)
{
    canny = value;
    updateNumericalLabel(ui->CannyLabel, value);
}


void ScalingTool::on_Accumulator_valueChanged(int value)
{
    accumulator = value;
    //updateNumericalLabel(ui, value);

}


void ScalingTool::on_HSVbutton_clicked()
{
    cv::Mat hsvFrame, hsvMask, resultFrame;
    cv::cvtColor(currSelectFrame, hsvFrame, cv::COLOR_BGR2HSV); // Convert to HSV
    // Create an HSV mask for flame colors
    cv::inRange(hsvFrame, cv::Scalar(minHue, minSat, minVal), cv::Scalar(maxHue, maxSat, maxVal), hsvMask);
    currSelectFrame.copyTo(resultFrame, hsvMask);
    graphicsViewHelper(ui->FileViewWindow, resultFrame, scene);
}




void ScalingTool::on_HMin_valueChanged(int arg1)
{
    if(minHue < maxHue && !(videoFilePath.empty())) {
        minHue = arg1;
        on_HSVbutton_clicked();
    }
}


void ScalingTool::on_HMax_valueChanged(int arg1)
{
    if(maxHue > minHue && !(videoFilePath.empty())) {
        maxHue = arg1;
        on_HSVbutton_clicked();
    }
}


void ScalingTool::on_SMin_valueChanged(int arg1)
{
    if(minSat < maxSat && !(videoFilePath.empty())) {
        minSat = arg1;
        on_HSVbutton_clicked();
    }
}


void ScalingTool::on_SMax_valueChanged(int arg1)
{
    if(maxSat > minSat && !(videoFilePath.empty())) {
        maxSat = arg1;
        on_HSVbutton_clicked();
    }
}


void ScalingTool::on_VMin_valueChanged(int arg1)
{
    if(minVal < maxVal && !(videoFilePath.empty())) {
        minVal = arg1;
        on_HSVbutton_clicked();
    }
}


void ScalingTool::on_VMax_valueChanged(int arg1)
{
    if(maxVal > minVal && !(videoFilePath.empty())) {
        maxVal = arg1;
        on_HSVbutton_clicked();
    }
}


// IMPORTANT FUNCTION TO PREFORM SPEED TRACKING AND ANALYZING
// get filtered flame contour
// loop through lines, find lowest and closest point to each line between threshold
// record position for each line every second

void ScalingTool::on_pushButton_8_clicked() {
    cv::VideoCapture cap(videoFilePath);
    // Check if the video file was opened successfully
    if (!cap.isOpened()) {
        std::cerr << "Error: FAILED OPENING FILE" << std::endl;
    } else {
        // Get video frame rate
        cap.set(cv::CAP_PROP_POS_FRAMES, 0);
        double fps = cap.get(cv::CAP_PROP_FPS);
        double timePerFrame = 1.0/fps; // in seconds
        double accumulatedTime = 0.0;
        double sampleInterval = 1.0;
        int seconds = 0;
        int frameCount = 0;
        cv::Mat frame;
        std::vector<std::vector<double>> totalPixelData;
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
            if(accumulatedTime > sampleInterval) {
                //std::cout << frameCount << "\n";
                accumulatedTime = accumulatedTime - sampleInterval;
                seconds = seconds+1;
                cv::Mat incoming = flame_process->findContourImage(frame);
                //flame_process->recordAngle(dataSegments, incoming, angleThreshold);
                std::vector<double> posData = flame_process->recordPositions(dataSegments);
                totalPixelData.emplace_back(posData);
                graphicsViewHelper(ui->FileViewWindow, incoming, scene);
                int out = 0 + ((100/cap.get(cv::CAP_PROP_FRAME_COUNT)) * (frameCount));
                ui->progressBar->setValue(out);
                QCoreApplication::processEvents();
            }
            frameCount++;
        }
        cap.release();
        ui->progressBar->setValue(100);
        totalPosData = flame_process->cleanData(totalPixelData);
    }
}




void ScalingTool::on_pushButton_9_clicked()
{
    //stopProcess = true;
    dataSegments.clear();
    totalAngleData.clear();
    totalPosData.clear();
    stopProcess = true;
    delete flame_process;
    ui->EditPanel->setCurrentIndex(0);
    ui->BeginButton->setDisabled(true);
}





void ScalingTool::on_SavePosDataButton_clicked()
{
    // Open a file dialog to allow the user to select a location and name
    QString fileName = QFileDialog::getSaveFileName(nullptr,"Save File","","CSV Files (*.csv);;All Files (*)");
    if (fileName.isEmpty()) {
        // User canceled the save dialog
        return;
    }
    std::string filePathSTD = fileName.toStdString();
    writePosDataToCSV(totalPosData, filePathSTD);
}


void ScalingTool::on_SegmentSpinBox_valueChanged(int arg1)
{
    segmentLines = arg1;
    cv::Mat newImg;
    currSelectFrame.copyTo(newImg);
    //std::cout << "DETETCING CIRCLES\n";
    //std::vector<cv::Vec3f> circles;
    //detectCircles(newImg, circles, minRad, maxRad, canny, accumulator);
    int maskWCoord = maskX + maskW;
    dataSegments = createManualGridlines(newImg, segmentLines, maskX, maskWCoord);
    graphicsViewHelper(ui->FileViewWindow, newImg, scene);

}







void ScalingTool::on_TrackAnglesButton_clicked()
{
    totalAngleData.clear();
    cv::VideoCapture cap(videoFilePath);
    // Check if the video file was opened successfully
    if (!cap.isOpened()) {
        std::cerr << "Error: FAILED OPENING FILE" << std::endl;
    } else {
        // Get video frame rate
        cap.set(cv::CAP_PROP_POS_FRAMES, 0);
        double fps = cap.get(cv::CAP_PROP_FPS);
        double timePerFrame = 1.0/fps; // in seconds
        double accumulatedTime = 0.0;
        double sampleInterval = 1.0;
        int seconds = 0;
        int frameCount = 0;
        cv::Mat frame;
        std::vector<std::vector<double>> angleData;
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
            if(accumulatedTime > sampleInterval) {
                //std::cout << frameCount << "\n";
                accumulatedTime = accumulatedTime - sampleInterval;
                seconds = seconds+1;
                cv::Mat incoming = flame_process->findContourImage(frame);
                std::vector<double> angleData = flame_process->recordAngle(dataSegments, incoming, angleThreshold);
                totalAngleData.emplace_back(angleData);
                graphicsViewHelper(ui->FileViewWindow, incoming, scene);
                int out = 0 + ((100/cap.get(cv::CAP_PROP_FRAME_COUNT)) * (frameCount));
                ui->progressBar->setValue(out);
                QCoreApplication::processEvents();
            }
            frameCount++;
        }
        cap.release();
        ui->progressBar->setValue(100);
        //totalPosData = flame_process->cleanData(totalPixelData);
    }

}


void ScalingTool::on_AngleSaveButton_clicked()
{
    // Open a file dialog to allow the user to select a location and name
    QString fileName = QFileDialog::getSaveFileName(nullptr,"Save File","","CSV Files (*.csv);;All Files (*)");
    if (fileName.isEmpty()) {
        return;
    }
    std::string filePathSTD = fileName.toStdString();
    writeAngleDataToCSV(totalAngleData, filePathSTD);
}


void ScalingTool::on_AngleThresholdWheel_valueChanged(int arg1)
{
    angleThreshold = arg1;
}


void ScalingTool::on_AreaTrackButton_clicked()
{
    cv::VideoCapture cap(videoFilePath);
    // Check if the video file was opened successfully
    if (!cap.isOpened()) {
        std::cerr << "Error: FAILED OPENING FILE" << std::endl;
    } else {
        // Get video frame rate

    int frameCount = 0;
    cv::Mat frame;
    cap >> frame;
    cv::Mat contourMask = cv::Mat::zeros(frame.size(), frame.type());
    cv::Mat newFrame = cv::Mat::zeros(frame.size(), frame.type());

    while (true) {
        if(stopProcess) {
            break;
        }

        // Capture each frame
        cap >> frame;

        // If the frame is empty, break the loop (end of video)
        if (frame.empty()) {
            break;
        }

            //std::cout << frameCount << "\n";
        if(frameCount % areaFrameThreshold == 0) {
            cv::Mat incoming = flame_process->findContourMask(frame, newFrame, contourMask);
            double realArea = flame_process->calculateArea(incoming);
            updateDoubleLabel(ui->AreaLabel, realArea);
            graphicsViewHelper(ui->FileViewWindow, incoming, scene);
            int out = 0 + ((100/cap.get(cv::CAP_PROP_FRAME_COUNT)) * (frameCount));
            ui->AreaProgress->setValue(out);
            QCoreApplication::processEvents();
        }
        frameCount++;
        }

    }
    cap.release();
    ui->AreaProgress->setValue(100);

}


void ScalingTool::on_FrameThreshSlider_valueChanged(int value)
{
    areaFrameThreshold = value;
    updateNumericalLabel(ui->FrameThreshLabel, value);
}


void ScalingTool::on_startFrame_valueChanged(int arg1)
{

}

