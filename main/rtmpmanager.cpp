#include "rtmpmanager.h"
#include "liveswapmanager.h"

RtmpManager::RtmpManager(QObject *parent)
    : QObject{parent}
{

}

RtmpManager* RtmpManager::getInstance()
{
    static RtmpManager* instance = new RtmpManager();
    return instance;
}

void RtmpManager::startPush()
{
    if (m_rtmpPushThread)
    {
        qInfo("the thread of rtmp push is in running status");
        return;
    }

    QSize cameraFrameSize = CameraManager::getInstance()->getCameraFrameSize();
    if (cameraFrameSize.width() == 0 || cameraFrameSize.height() == 0)
    {
        qCritical("the size of camera frame is (0, 0)");
        return;
    }

    AVPixelFormat format = CameraManager::getInstance()->getCameraFrameFormat();

    m_rtmpPushThread = new RtmpPushThread();
    m_rtmpPushThread->setRtmpPushUrl(LiveSwapManager::getInstance()->getPushUrl());
    m_rtmpPushThread->setSize(cameraFrameSize.width(), cameraFrameSize.height());
    m_rtmpPushThread->setFormat(format);
    connect(m_rtmpPushThread, &RtmpPushThread::runFinish, this, &RtmpManager::rtmpPushThreadFinish);
    connect(m_rtmpPushThread, &RtmpPushThread::finished, m_rtmpPushThread, &QObject::deleteLater);
    m_rtmpPushThread->start();

    CameraManager::getInstance()->setFrameArriveCallback(this);
}

void RtmpManager::stopPush()
{
    if (m_rtmpPushThread == nullptr)
    {
        return;
    }

    m_rtmpPushThread->setExit();
    disconnect(m_rtmpPushThread, &RtmpPushThread::runFinish, this, &RtmpManager::rtmpPushThreadFinish);

    m_rtmpPushThread = nullptr;
    CameraManager::getInstance()->setFrameArriveCallback(nullptr);
}

void RtmpManager::startPull()
{
    if (m_rtmpPullThread)
    {
        qInfo("the thread of rtmp pull is in running status");
        return;
    }

    m_rtmpPullThread = new RtmpPullThread();
    m_rtmpPullThread->setRtmpPullUrl(LiveSwapManager::getInstance()->getPushUrl());
    m_rtmpPullThread->enableGenerateQImage();
    m_rtmpPullThread->setRtmpFrameArriveCallback(m_rtmpFrameArriveCallback);
    connect(m_rtmpPullThread, &RtmpPullThread::runFinish, this, &RtmpManager::rtmpPullThreadFinish);
    connect(m_rtmpPullThread, &RtmpPullThread::finished, m_rtmpPullThread, &QObject::deleteLater);
    m_rtmpPullThread->start();
}

void RtmpManager::stopPull()
{
    if (m_rtmpPullThread == nullptr)
    {
        return;
    }

    m_rtmpPullThread->setExit();
    m_rtmpPullThread->setRtmpFrameArriveCallback(nullptr);
    disconnect(m_rtmpPullThread, &RtmpPullThread::runFinish, this, &RtmpManager::rtmpPullThreadFinish);

    m_rtmpPullThread = nullptr;
}

void RtmpManager::setRtmpFrameArriveCallback(IRtmpFrameArriveCallback* callback)
{
    m_rtmpFrameArriveCallback = callback;
    if (m_rtmpPullThread)
    {
        m_rtmpPullThread->setRtmpFrameArriveCallback(m_rtmpFrameArriveCallback);
    }
}


QImage* RtmpManager::getRtmpPullImage()
{
    if (m_rtmpPullThread)
    {
        return m_rtmpPullThread->popImage();
    }

    return nullptr;
}

void RtmpManager::onFrameArrive(AVFrame* frame)
{
    if (m_rtmpPushThread)
    {
        m_rtmpPushThread->pushFrame(frame);
    }
    else
    {
        av_frame_free(&frame);
    }
}

void RtmpManager::rtmpPullThreadFinish()
{
    m_rtmpPullThread = nullptr;
}

void RtmpManager::rtmpPushThreadFinish()
{
    m_rtmpPushThread = nullptr;
}
