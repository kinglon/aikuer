#ifndef CAMERAREADTHREAD_H
#define CAMERAREADTHREAD_H

#include <QThread>
#include <QObject>
#include <QImage>

extern "C"
{
    #include "libavdevice/avdevice.h"
}


class CameraReadThread : public QThread
{
    Q_OBJECT

public:
    CameraReadThread();

public:
    void setExit() { m_exit = true; }

    void setCamera(const AVInputFormat* camera) { m_camera = camera; }

    void setRtmpPushUrl(QString rtmpPushUrl) { m_rtmpPushUrl = rtmpPushUrl; }

    void setFrameCount(int frameCount) { m_frameCount = frameCount; }

    // 启用后，需要处理imageArrive信号
    void setEnableImageArriveSignal() { m_enableImageArriveSignal = true; }

signals:
    // image需要释放
    void imageArrive(QImage* image);

    void runFinish();

protected:
    void run() override;

private:
    void run2();

private:
    bool m_exit = false;

    bool m_enableImageArriveSignal = false;

    const AVInputFormat* m_camera = nullptr;

    QString m_rtmpPushUrl;

    int m_frameCount = 30;
};

#endif // CAMERAREADTHREAD_H
