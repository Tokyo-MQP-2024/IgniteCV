#include "imagesubtraction.h"
#include <QMutexLocker>
#include <iostream>

ImageSubtraction::ImageSubtraction(const cv::Mat &im1, const cv::Mat &im2, cv::Mat *dest, QMutex *mutex) {
    m_im1 = im1;
    m_im2 = im2;
    m_dest = dest;
    m_mutex = mutex;
    m_runflag = RUNFLAG::SINGLE_IMAGE;
}

ImageSubtraction::ImageSubtraction(QString path, const cv::Mat &im2, QString dest) {
    m_path = path;
    m_im2 = im2;
    m_dest_folder = dest;
    m_runflag = RUNFLAG::VIDEO;
}

void ImageSubtraction::run() {
    QMutexLocker locker(m_mutex);
    switch(m_runflag) {
    case RUNFLAG::SINGLE_IMAGE:
        cv::absdiff(m_im1, m_im2, *m_dest);
        break;
    case RUNFLAG::VIDEO:
        break;
    case RUNFLAG::UNDEFINED:
    default:
        std::cerr << "ImageSubtraction Error: RUNFLAG is UNDEFINED." << std::endl;
        break;
    }
}
