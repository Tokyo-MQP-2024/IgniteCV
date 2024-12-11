#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFuture>
#include <QThreadPool>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <QActionGroup>
#include <qtconcurrentrun.h>
#include <qtranslator.h>
#include <QtUiTools/QUiLoader>
#include "utils.h"
#include <QFutureWatcher>
#include <QTConcurrent/QtConcurrentMap>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    calculateWidth(new CalculateWidth),
    scalingTool(new ScalingTool),
    frequencyDetection(new FrequencyDetection)
{
    ui->setupUi(this);

    // Disable bad buttons
    ui->ProcessVideoButton->setEnabled(false);

    ui->stackedWidget->addWidget(calculateWidth);
    ui->stackedWidget->addWidget(scalingTool);
    ui->stackedWidget->addWidget(frequencyDetection);

    //int index = ui->stackedWidget->indexOf(this);
    ui->stackedWidget->setCurrentIndex(0);

    createLanguageMenu();

}

MainWindow::~MainWindow()
{
    delete ui;
}

// Language
// we create the menu entries dynamically, dependent on the existing translations.
void MainWindow::createLanguageMenu(void) {
    QActionGroup* langGroup = new QActionGroup(ui->menuLanguage);
    langGroup->setExclusive(true);

    connect(langGroup, SIGNAL (triggered(QAction *)), this, SLOT (slotLanguageChanged(QAction *)));

    // format systems language
    QString defaultLocale = QLocale::system().name(); // e.g. "de_DE"
    defaultLocale.truncate(defaultLocale.lastIndexOf('_')); // e.g. "de"

    m_langPath = QApplication::applicationDirPath();
    //m_langPath.append("/translations");
    QDir dir(m_langPath);
    QStringList fileNames = dir.entryList(QStringList("OpenCV_Integration_*.qm"));

    for (int i = 0; i < fileNames.size(); ++i) {
        // get locale extracted by filename
        QString locale;
        locale = fileNames[i]; // "TranslationExample_de.qm"
        locale.truncate(locale.lastIndexOf('.')); // "TranslationExample_de"
        locale.remove(0, locale.lastIndexOf('_') + 1); // "de"

        QString lang = QLocale::languageToString(QLocale(locale).language());
        QIcon ico(QString("%1/%2.png").arg(m_langPath).arg(locale));

        QAction *action = new QAction(ico, lang, this);
        action->setCheckable(true);
        action->setData(locale);

        ui->menuLanguage->addAction(action);
        langGroup->addAction(action);

        // set default translators and language checked
        if (defaultLocale == locale) {
            action->setChecked(true);
        }
    }
}

// Called every time, when a menu entry of the language menu is called
void MainWindow::slotLanguageChanged(QAction* action)
{
    if(0 != action) {
        // load the language dependant on the action content
        loadLanguage(action->data().toString());
        setWindowIcon(action->icon());
    }
}

void switchTranslator(QTranslator& translator, const QString& filename) {
    // remove the old translator
    qApp->removeTranslator(&translator);

    // load the new translator
    QString path = QApplication::applicationDirPath();
    path.append("/");
    if(translator.load(path + filename)) //Here Path and Filename has to be entered because the system didn't find the QM Files else
        qApp->installTranslator(&translator);
}

void MainWindow::loadLanguage(const QString& rLanguage) {
    if(m_currLang != rLanguage) {
        m_currLang = rLanguage;
        QLocale locale = QLocale(m_currLang);
        QLocale::setDefault(locale);
        QString languageName = QLocale::languageToString(locale.language());
        switchTranslator(m_translator, QString("OpenCV_Integration_%1.qm").arg(rLanguage));
        switchTranslator(m_translatorQt, QString("qt_%1.qm").arg(rLanguage));
        ui->statusbar->showMessage(tr("Current Language changed to %1").arg(languageName));
    }
}

void MainWindow::changeEvent(QEvent* event) {
    if(0 != event) {
        switch(event->type()) {
        // this event is send if a translator is loaded
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;

            // this event is send, if the system, language changes
        case QEvent::LocaleChange:
        {
            QString locale = QLocale::system().name();
            locale.truncate(locale.lastIndexOf('_'));
            loadLanguage(locale);
        }
        default:
            break;
        }
    }
    QMainWindow::changeEvent(event);
}

// Page Navigation

void MainWindow::on_actionHome_triggered() {
    ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::on_averageImages_clicked() {
    ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::on_pushButton_5_clicked() {
    ui->stackedWidget->setCurrentWidget(calculateWidth);
}

void MainWindow::on_pushButton_clicked() {
    ui->stackedWidget->setCurrentWidget(frequencyDetection);
}

void MainWindow::on_FlameToolButton_clicked() {
    ui->stackedWidget->setCurrentWidget(scalingTool);
}

// IMAGE AVERAGING PAGE FUNCTIONS

void MainWindow::on_pushButton_2_clicked() {
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Open a Folder"), "C://");
    ui->label_4->setText(folderPath);
}

// Average images in selected folder
// void MainWindow::on_pushButton_4_clicked() {
//     // Make sure file path is valid
//     if(!QDir(ui->label_4->text()).exists() || ui->label_4->text() == "") {
//         QMessageBox::critical(this, tr("Error"), tr("Invalid folder path"));
//         return;
//     }
//     // Helper function from utils
//     ui->pushButton_4->setDisabled(true);


//     QFuture<cv::Mat> future = QtConcurrent::run(averageImagesFromFolder, ui->label_4->text());
//     cv::Mat averageImage = future.result();
//     cv::imshow("test", averageImage);
//     ui->pushButton_4->setDisabled(false);
// }

// Map function to average a batch of images
cv::Mat MainWindow::averageBatch(const QStringList& batchPaths) {
    QFileInfoList fileList;

    // Convert each QString in batchPaths to a QFileInfo and add it to fileList
    for (const QString& path : batchPaths) {
        fileList.append(QFileInfo(path));
    }

    // Initialize an accumulator matrix with the size of the first valid image and float precision
    cv::Mat accumulator;

    int imageCount = 0;
    for (const QFileInfo& fileInfo : fileList) {
        // Return early if cancelled
        if(QThread::currentThread()->isInterruptionRequested()) {
            return cv::Mat();
        }
        QString filePath = fileInfo.absoluteFilePath();

        QFile file(filePath);
        file.open(QFile::ReadOnly);
        qint64 sz = file.size();
        std::vector<uchar> buf(sz);
        file.read((char*) buf.data(), sz);

        cv::Mat img = cv::imdecode(buf, cv::IMREAD_COLOR); // For japanese filepath compatibility

        // Check if the image is valid
        if (img.empty()) {
            qWarning() << QFileDevice::tr("Skipping invalid or incompatible file:") << filePath;
            continue;
        }

        // Convert the image to float format for accumulation
        cv::Mat floatImg;
        img.convertTo(floatImg, CV_32FC3);

        // Initialize accumulator on the first valid image
        if (accumulator.empty()) {
            accumulator = cv::Mat::zeros(floatImg.size(), CV_32FC3);
        }

        // Add the current image to the accumulator
        accumulator += floatImg;
        ++imageCount;
    }

    // Ensure we have at least one valid image to average
    if (imageCount == 0) {
        qWarning() << QFileDevice::tr("No valid images could be processed for averaging.");
        return cv::Mat();
    }

    // Divide the accumulator by the number of images to get the average
    accumulator /= static_cast<float>(imageCount);

    // Convert the result back to 8-bit for display or saving - DONT DO THIS
    //cv::Mat averageImage;
    //accumulator.convertTo(averageImage, CV_8UC3);
    return accumulator;
}

void MainWindow::on_pushButton_4_clicked() {
    // Check for empty files
    QString folderPath = ui->label_4->text();
    if(folderPath == "") {
        QMessageBox::warning(this, tr("Warning"), tr("Invalid file path. Please try again"));
        return;
    }
    QDir dir(folderPath);

    // Return if folder is invalid
    if(!dir.exists()) {
        QMessageBox::warning(this, tr("Warning"), tr("Invalid file path. Please try again"));
        return;
    }


    dir.setFilter(QDir::Files);
    dir.setNameFilters({"*.jpg", "*.jpeg", "*.png", "*.bmp", "*.tif", "*.tiff"});
    QFileInfoList fileList = dir.entryInfoList();

    if (fileList.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No valid images found in the directory"));
        return;
    }

    // Convert QFileInfoList to QStringList of file paths
    QStringList filePaths;
    for (const QFileInfo& fileInfo : fileList) {
        filePaths << fileInfo.absoluteFilePath();
    }

    // Calculate optimized batch size with a minimum of 1
    int idealThreadCount = QThread::idealThreadCount();
    int batchSize = std::max(1, (int)(filePaths.size() / idealThreadCount));

    // Divide file paths into batches, ensuring all files are included
    QList<QStringList> batches;
    for (int i = 0; i < filePaths.size(); i += batchSize) {
        batches.append(filePaths.mid(i, batchSize));
    }

    // Create the progress dialog with a cancel button
    progressDialog = new QProgressDialog(tr("Processing images"), tr("Cancel"), 0, filePaths.size(), this);
    progressDialog->setWindowModality(Qt::WindowModal); // Modal dialog to block interaction with the main window
    progressDialog->setMinimumDuration(0); // Show instantly
    progressDialog->setAutoClose(true);    // Close automatically when done
    progressDialog->setAutoReset(true);    // Reset automatically when done


    // Create FutureWatcher to monitor progress
    QFutureWatcher<cv::Mat> *watcher = new QFutureWatcher<cv::Mat>(this);

    // Connect progress dialog cancel button to stop processing
    connect(progressDialog, &QProgressDialog::canceled, watcher, &QFutureWatcher<cv::Mat>::cancel);

    // Connect progress updates
    connect(watcher, &QFutureWatcher<cv::Mat>::progressValueChanged, progressDialog, &QProgressDialog::setValue);

    // Connect when processing is finished
    connect(watcher, &QFutureWatcher<cv::Mat>::finished, this, [this, watcher, batches](){
        if(watcher->isCanceled()) {
            QMessageBox::information(this, tr("Canceled"), tr("The operation was canceled by the user."));
        } else {
            // Collect results when finished
            QFuture<cv::Mat> future = watcher->future();
            cv::Mat accumulator = future.result();

            // Final averaging
            int totalBatchCount = batches.size();
            if (totalBatchCount > 0 && !accumulator.empty()) {
                accumulator /= static_cast<float>(totalBatchCount);
            } else {
                QMessageBox::warning(this, tr("Warning"), tr("No images found"));
                return 0;
            }

            // Convert to 8-bit for display or saving
            cv::Mat averageImage;
            accumulator.convertTo(averageImage, CV_8UC3);

            cv::imshow(tr("Result").toStdString(), averageImage);
        }
        watcher->deleteLater(); // Clean up the watcher
        progressDialog->deleteLater(); // Clean up progress dialog
        return 0;
    });

    // Cool and chill little lambda function to replace std::bind
    QFuture<cv::Mat> future = QtConcurrent::mappedReduced<cv::Mat>(batches, [this](const QStringList& batchPaths){
        // Return early if cancelled
        if(QThread::currentThread()->isInterruptionRequested()) {
            return cv::Mat();
        }
        cv::Mat result = averageBatch(batchPaths);
        QMetaObject::invokeMethod(progressDialog, "setValue", Q_ARG(int, progressDialog->value() + 1));
        return result;
    }, accumulateBatch);
    watcher->setFuture(future);
    progressDialog->setMaximum(batches.size());
}




// button for running video processing program
void MainWindow::on_ProcessVideoButton_clicked()
{
    //FlameProcessing::scaleClicks = 0;
    //flame_process->scaleClicks = 0;

    int index = ui->stackedWidget->indexOf(scalingTool);
    ui->stackedWidget->setCurrentIndex(index);


    //flame_process->parseVideo(videoFilePath.toStdString(), ui->VideoView);//ru the process func
}

// button to uplaod video for processing
void MainWindow::on_VideoSelectButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open a File"), "C://");
    std::string filePathSTD = filePath.toStdString();
    flame_process = new FlameProcessing(); // create new instance of flame process
    if(flame_process->checkMP4(filePathSTD)) {
        ui->VideoLabel->setText(filePath);
        ui->ProcessVideoButton->setEnabled(true);
        // TODO: display video on the graphics viewer
        // TODO: pass the file into parseVideo
        videoFilePath = filePath;
        scalingTool->videoFilePath = filePathSTD;
        scalingTool->flame_process = flame_process;
        //ui->stackedWidget->addWidget(scalingTool);

    } else {
        ui->VideoLabel->setText("File must be .mp4");
    }




}

void MainWindow::on_CancelButton_clicked()
{
    //TODO: stop button for processing
    if(flame_process != nullptr) {
        flame_process->setStopProcess(true);
    }
}


void MainWindow::on_VideoView_rubberBandChanged(const QRect &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint)
{

}





