#ifndef RTMPMANAGER_H
#define RTMPMANAGER_H

#include <QObject>
#include "rtmppullthread.h"
#include "rtmppushthread.h"
#include "cameramanager.h"

class RtmpManager : public QObject, public IFrameArriveCallback
{
    Q_OBJECT
public:
    explicit RtmpManager(QObject *parent = nullptr);

public:
    static RtmpManager* getInstance();

    void startPush();

    void stopPush();

    void startPull();

    void stopPull();

    void setRtmpFrameArriveCallback(IRtmpFrameArriveCallback* callback);

    // 用完要释放
    QImage* getRtmpPullImage();

    virtual void onFrameArrive(AVFrame* frame) override;

private slots:
    void rtmpPushThreadFinish();

    void rtmpPullThreadFinish();

private:
    RtmpPullThread* m_rtmpPullThread = nullptr;

    RtmpPushThread* m_rtmpPushThread = nullptr;

    IRtmpFrameArriveCallback* m_rtmpFrameArriveCallback = nullptr;
};

#endif // RTMPMANAGER_H
