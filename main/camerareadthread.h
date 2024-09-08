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

class IFrameArriveCallback
{
public:
    // 处理函数负责释放frame
    virtual void onFrameArrive(AVFrame* frame) = 0;
};


class CameraReadThread : public QThread
{
    Q_OBJECT

public:
    CameraReadThread();
    ~CameraReadThread();

public:
    void setExit() { m_exit = true; }

    void setCamera(const AVInputFormat* camera) { m_camera = camera; }    

    void setFrameCount(int frameCount) { m_frameCount = frameCount; }

    void enableGenerateQImage(bool enable) { m_enableGenerateQImage = enable; }

    // 用完要释放
    QImage* popImage();

    void setFrameArriveCallback(IFrameArriveCallback* callback) { m_frameArriveCallback = callback; }

    QSize getCameraFrameSize() { return m_cameraSize; }

    AVPixelFormat getCameraFrameFormat() { return m_format; }

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

    int m_frameCount = 30;

    QImage* m_lastImage = nullptr;

    QMutex m_mutex;

    IFrameArriveCallback* m_frameArriveCallback = nullptr;

    QSize m_cameraSize;

    AVPixelFormat m_format = AV_PIX_FMT_YUV420P;
};

#endif // CAMERAREADTHREAD_H
