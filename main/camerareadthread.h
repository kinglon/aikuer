#ifndef CAMERAREADTHREAD_H
#define CAMERAREADTHREAD_H

#include <QThread>
#include <QObject>
#include <QImage>
#include <QMutex>

extern "C"
{
    #include "libavdevice/avdevice.h"
}


class CameraReadThread : public QThread
{
    Q_OBJECT

public:
    CameraReadThread();
    ~CameraReadThread();

public:
    void setExit() { m_exit = true; }

    void setCamera(const AVInputFormat* camera) { m_camera = camera; }

    void setRtmpPushUrl(QString rtmpPushUrl) { m_rtmpPushUrl = rtmpPushUrl; }

    void setFrameCount(int frameCount) { m_frameCount = frameCount; }

    void enableGenerateQImage() { m_enableGenerateQImage = true; }

    // 用完要释放
    QImage* popImage();

signals:
    void runFinish();

protected:
    void run() override;

private:
    void run2();

private:
    bool m_exit = false;

    bool m_enableGenerateQImage = false;

    const AVInputFormat* m_camera = nullptr;

    QString m_rtmpPushUrl;

    int m_frameCount = 30;

    QImage* m_lastImage = nullptr;

    QMutex m_mutex;
};

#endif // CAMERAREADTHREAD_H
