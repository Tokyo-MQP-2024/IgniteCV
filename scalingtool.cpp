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
    //ui->WhiteVal->setText();


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

    // if index = 0: keep scale as is
    // if(ui->xComboBox->currentIndex() == 1) {
    //     manual_scaleX = manual_scaleX/10;
    //     manual_scaleY = manual_scaleY/10;
    // }

    // else if(ui->xComboBox->currentIndex() == 2) {
    //     manual_scaleX = manual_scaleX*2.54;
    //     manual_scaleY = manual_scaleY*2.54;
    // }


    // QString sx = QString::number(manual_scaleX);
    // QString sy = QString::number(manual_scaleY);

    // ui->ScaleXEdit->setText(sx);
    // ui->ScaleYEdit->setText(sy);

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
    croppedFrame = imCropped;

    cv::imshow("NEW IMG", croppedFrame);
    maskX = roi.x*2;
    maskY = roi.y*2;
    maskH = roi.height*2;
    maskW = roi.width*2;

    roiSelected = true;

    cv::Mat frame;
    if (globalCap.read(frame)) {
        applyROIMask(frame);
        frame.copyTo(currSelectFrame);
    }
    graphicsViewHelper(ui->FileViewWindow, flame_process, frame);



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
        ui->ROIButton->setDisabled(false);
    }
}


void ScalingTool::on_ScaleYEdit_editingFinished()
{
    editYDone = true;
    if (scaleXDone && scaleYDone && editXDone && editYDone) {
        ui->ROIButton->setDisabled(false);
    }
}


void ScalingTool::on_BeginButton_clicked()
{
    //flame_process->parseVideo()
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

        graphicsViewHelper(ui->FileViewWindow, flame_process, frame1);


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
            graphicsViewHelper(ui->FileViewWindow, flame_process, frame);

        } else {
            graphicsViewHelper(ui->FileViewWindow, flame_process, frame);
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
    detectCircles(croppedFrame, circles);

    cv::imshow("cirles", croppedFrame);
}


// Open circle detect edit panel
void ScalingTool::on_pushButton_4_clicked()
{
    cv::Mat newImg;
    levelsIMG.copyTo(newImg);

    std::cout << "DETETCING CIRCLES\n";
    std::vector<cv::Vec3f> circles;
    detectCircles(newImg, circles);
    cv::imshow("cirles", newImg);
}



void ScalingTool::adjustLevels(cv::Mat image) {

    double gamma = 1.0;

    image.copyTo(levelsIMG);


    // Create a chart
    //QChart *chart = new QChart();

    // Set the title for the chart
    //chart->setTitle("RGB Histogram");


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

        graphicsViewHelper(ui->FileViewWindow, flame_process, levelsIMG);
    }






}




void ScalingTool::on_BlackSlider_valueChanged(int value)
{
    std::cout << "herehehrhhf\n";

    clipBlack = value;
    if(!croppedFrame.empty()) {
        adjustLevels(croppedFrame);
        //graphicsViewHelper(ui->FileViewWindow, flame_process, croppedFrame);
    }


}


void ScalingTool::on_WhiteSlider_valueChanged(int value)
{
    clipWhite = value;
    if(!croppedFrame.empty()) {
        adjustLevels(croppedFrame);
        //graphicsViewHelper(ui->FileViewWindow, flame_process, croppedFrame);
    }
    //graphicsViewHelper(ui->FileViewWindow, flame_process, croppedFrame);
}

