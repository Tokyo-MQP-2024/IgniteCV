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


    //setupHistogram();
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

    // // Resize the window for displaying the image
    // cv::namedWindow("Select ROI", cv::WINDOW_NORMAL);
    // cv::resizeWindow("Select ROI", 800, 600); // Resize the window (example: 800x600)

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
    //detectCircles(croppedFrame, circles);

    cv::imshow("cirles", croppedFrame);
}


// Open circle detect edit panel
void ScalingTool::on_pushButton_4_clicked()
{
    cv::Mat newImg;
    levelsIMG.copyTo(newImg);

    std::cout << "DETETCING CIRCLES\n";
    std::vector<cv::Vec3f> circles;
    detectCircles(newImg, circles, minRad, maxRad, canny, accumulator);

    createGridlines(newImg, circles);
    graphicsViewHelper(ui->FileViewWindow, flame_process, newImg);

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

    clipBlack = value;
    QString valSTR = QString::number(value);
    ui->BlackVal->setText(valSTR);
    if(!croppedFrame.empty()) {
        adjustLevels(croppedFrame);
        //graphicsViewHelper(ui->FileViewWindow, flame_process, croppedFrame);
    }


}


void ScalingTool::on_WhiteSlider_valueChanged(int value)
{
    clipWhite = value;
    updateNumericalLabel(ui->WhiteVal, value);
    if(!croppedFrame.empty()) {
        adjustLevels(croppedFrame);
        //graphicsViewHelper(ui->FileViewWindow, flame_process, croppedFrame);
    }
    //graphicsViewHelper(ui->FileViewWindow, flame_process, croppedFrame);
}


// void ScalingTool::setupHistogram()
// {
//     // Example data for the histogram
//     QVector<double> bins = {1, 2, 3, 4, 5};      // X-axis bins
//     QVector<double> frequencies = {5, 15, 25, 10, 8}; // Y-axis values

//     // Create a QCPBars object
//     QCPBars *histogram = new QCPBars(ui->customPlot->xAxis, ui->customPlot->yAxis);
//     histogram->setWidth(0.5);                   // Bar width
//     histogram->setBrush(QBrush(Qt::blue));     // Bar color
//     histogram->setPen(QPen(Qt::black));        // Border color

//     // Set data
//     histogram->setData(bins, frequencies);

//     // Customize axes
//     ui->customPlot->xAxis->setLabel("Bins");
//     ui->customPlot->yAxis->setLabel("Frequency");

//     // Automatically scale axes to fit the histogram
//     ui->customPlot->xAxis->setRange(bins.first() - 1, bins.last() + 1); // Add padding
//     ui->customPlot->yAxis->setRange(0, *std::max_element(frequencies.begin(), frequencies.end()) * 1.1); // Add 10% padding

//     // Enable dynamic rescaling when data changes
//     ui->customPlot->rescaleAxes();

//     // Replot
//     ui->customPlot->replot();
// }


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

