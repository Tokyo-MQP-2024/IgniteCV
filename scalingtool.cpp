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
    ui->ROIButton->setDisabled(true);
    ui->BeginButton->setDisabled(true);
    ui->stackedWidget->setCurrentIndex(0);


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

    flame_process->imageROISelect(videoFilePath);

    ui->BeginButton->setDisabled(false);

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
        // ui->FileViewWindow->scene()->clear();
        // QImage qimg = matToQImage(frame1);
        // QPixmap pixmap = QPixmap::fromImage(qimg);


        // Fit the pixmap inside the view window
        //view.fitInView(item, Qt::KeepAspectRatio);
        //QGraphicsPixmapItem *item = scene->addPixmap(pixmap);


        //ui->FileViewWindow->fitInView(item, Qt::KeepAspectRatio);
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
    currSelectFrame = frame;
    if (globalCap.read(frame)) {
        graphicsViewHelper(ui->FileViewWindow, flame_process, frame);
    }

}



// Circle detection button
void ScalingTool::on_pushButton_7_clicked()
{
    std::cout << "DETETCING CIRCLES\n";
    std::vector<cv::Vec3f> circles;
    detectCircles(currSelectFrame, circles);
    cv::imshow("cirles", currSelectFrame);
}


// Open circle detect edit panel
void ScalingTool::on_pushButton_4_clicked()
{
    std::cout << "DETETCING CIRCLES\n";
    std::vector<cv::Vec3f> circles;
    detectCircles(currSelectFrame, circles);
    std::cout << "did it make it here\n";
    cv::imshow("cirles", currSelectFrame);
}

