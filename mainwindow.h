#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QPixmap>
#include <QCloseEvent>
#include <QMessageBox>

// For file dialog
#include <QFileDialog>

#include "frequencydetection.h"
#include <QTranslator>

#include "calculateWidth.h"
#include "scalingtool.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    // this event is called, when a new translator is loaded or the system language is changed
    void changeEvent(QEvent*);
protected slots:
    // this slot is called by the language menu actions
    void slotLanguageChanged(QAction* action);
private slots:
    void on_actionHome_triggered();
    void on_pushButton_2_clicked();
    void on_averageImages_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_FlameToolButton_clicked();
    void on_ProcessVideoButton_clicked();
    void on_VideoSelectButton_clicked();
    void on_CancelButton_clicked();
    void on_VideoView_rubberBandChanged(const QRect &viewportRect, const QPointF &fromScenePoint, const QPointF &toScenePoint);
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    CalculateWidth *calculateWidth;
    ScalingTool *scalingTool;
    FrequencyDetection *frequencyDetection;
    QGraphicsPixmapItem pixmap;
    cv::VideoCapture video;
    QString videoFilePath;
    FlameProcessing *flame_process;

    QProgressDialog *progressDialog = nullptr;

    // Language
    // loads a language by the given language shortcur (e.g. de, en)
    void loadLanguage(const QString& rLanguage);

    // creates the language menu dynamically from the content of m_langPath
    void createLanguageMenu(void);
    QTranslator m_translator; // contains the translations for this application
    QTranslator m_translatorQt; // contains the translations for qt
    QString m_currLang; // contains the currently loaded language
    QString m_langPath; // Path of language files. This is always fixed to /translations.

    cv::Mat averageBatch(const QStringList& batchPaths);
};
#endif // MAINWINDOW_H
