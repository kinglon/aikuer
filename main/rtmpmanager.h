#ifndef RTMPMANAGER_H
#define RTMPMANAGER_H

#include <QObject>
#include "rtmppullthread.h"

class RtmpManager : public QObject
{
    Q_OBJECT
public:
    explicit RtmpManager(QObject *parent = nullptr);

public:
    static RtmpManager* getInstance();

    void startPull(QString pullUrl);

    void stopPull();

    void setRtmpFrameArriveCallback(IRtmpFrameArriveCallback* callback) { m_rtmpFrameArriveCallback = callback; }

signals:
    void receiveCameraImage(const QImage* image);

private slots:
    void imageArrive(QImage* image);

    void rtmpPullThreadFinish();

private:
    RtmpPullThread* m_rtmpPullThread = nullptr;

    IRtmpFrameArriveCallback* m_rtmpFrameArriveCallback = nullptr;
};

#endif // RTMPMANAGER_H
