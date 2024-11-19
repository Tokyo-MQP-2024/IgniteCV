#ifndef IMAGESUBTRACTION_H
#define IMAGESUBTRACTION_H

#include <QRunnable>
#include <opencv2/core.hpp>
#include <QMutex>

enum class RUNFLAG {
    UNDEFINED=-1,
    SINGLE_IMAGE,
    VIDEO,
};

class ImageSubtraction : public QRunnable
{
public:
    // Subtracts im2 from im1 and stores result in dest
    // TODO - implement mass subtraction, with one background image and a folder of subtractors.
    ImageSubtraction(const cv::Mat &im1, const cv::Mat &im2, cv::Mat *dest, QMutex *mutex);

    // First string can be file path or video file. Mass subtraction with single background image.
    ImageSubtraction(QString path, const cv::Mat &im2, QString dest);
    void run() override;
private:
    cv::Mat m_im1;
    cv::Mat m_im2;
    cv::Mat *m_dest;
    QMutex* m_mutex;
    RUNFLAG m_runflag;

    QString m_path;
    QString m_dest_folder;
};

#endif // IMAGESUBTRACTION_H
