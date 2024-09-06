#ifndef RTMPPULLTHREAD_H
#define RTMPPULLTHREAD_H

#include <QThread>
#include <QObject>
#include <QImage>

extern "C"
{
    #include "libavdevice/avdevice.h"
}


class RtmpPullThread : public QThread
{
    Q_OBJECT

public:
    RtmpPullThread();

public:
    void setExit() { m_exit = true; }

    void setRtmpPullUrl(QString rtmpPullUrl) { m_rtmpPullUrl = rtmpPullUrl; }

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

    QString m_rtmpPullUrl;
};

#endif // RTMPPULLTHREAD_H
