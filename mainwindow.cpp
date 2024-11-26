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
    // ui->ProcessVideoButton->setEnabled(false);
    // ui->stackedWidget->setCurrentIndex(3);
    int index = ui->stackedWidget->indexOf(scalingTool);
    ui->stackedWidget->setCurrentIndex(index);
}

// IMAGE AVERAGING PAGE FUNCTIONS

void MainWindow::on_pushButton_2_clicked() {
    QString folderPath = QFileDialog::getExistingDirectory(this, tr("Open a Folder"), "C://");
    ui->label_4->setText(folderPath);
}

// Average images in selected folder
void MainWindow::on_pushButton_4_clicked() {
    // Helper function from utils
    //cv::Mat averageImage = averageImagesFromFolder(ui->label_4->text());
    ui->pushButton_4->setDisabled(true);
    QFuture<cv::Mat> future = QtConcurrent::run(averageImagesFromFolder, ui->label_4->text());
    cv::Mat averageImage = future.result();
    cv::imshow("test", averageImage);
    ui->pushButton_4->setDisabled(false);
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





