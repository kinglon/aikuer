#ifndef RTMPPULLTHREAD_H
#define RTMPPULLTHREAD_H

#include <QThread>
#include <QObject>
#include <QImage>
#include <QMutex>

extern "C"
{
    #include "libavdevice/avdevice.h"
}

class IRtmpFrameArriveCallback
{
public:
    // 处理函数需要释放frame
    virtual void onRtmpFrameArrive(AVFrame* frame) = 0;
};


class RtmpPullThread : public QThread
{
    Q_OBJECT

public:
    RtmpPullThread();
    ~RtmpPullThread();

public:
    void setExit() { m_exit = true; }

    void setRtmpPullUrl(QString rtmpPullUrl) { m_rtmpPullUrl = rtmpPullUrl; }

    void enableGenerateQImage() { m_enableGenerateQImage = true; }

    // 用完要释放
    QImage* popImage();

    void setRtmpFrameArriveCallback(IRtmpFrameArriveCallback* callback) { m_rtmpFrameArriveCallback = callback; }

signals:
    void runFinish();

protected:
    void run() override;

private:
    void run2();

private:
    bool m_exit = false;

    bool m_enableGenerateQImage = false;

    QString m_rtmpPullUrl;

    IRtmpFrameArriveCallback* m_rtmpFrameArriveCallback = nullptr;

    QImage* m_lastImage = nullptr;

    QMutex m_mutex;
};

#endif // RTMPPULLTHREAD_H
