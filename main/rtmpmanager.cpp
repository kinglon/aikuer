#include "rtmpmanager.h"

RtmpManager::RtmpManager(QObject *parent)
    : QObject{parent}
{

}

RtmpManager* RtmpManager::getInstance()
{
    static RtmpManager* instance = new RtmpManager();
    return instance;
}

void RtmpManager::startPull(QString pullUrl)
{
    if (m_rtmpPullThread)
    {
        qInfo("the thread of rtmp pull is in running status");
        return;
    }

    m_rtmpPullThread = new RtmpPullThread();
    m_rtmpPullThread->setRtmpPullUrl(pullUrl);
    m_rtmpPullThread->setEnableImageArriveSignal();
    m_rtmpPullThread->setRtmpFrameArriveCallback(m_rtmpFrameArriveCallback);
    connect(m_rtmpPullThread, &RtmpPullThread::imageArrive, this, &RtmpManager::imageArrive);
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
    m_rtmpPullThread = nullptr;
}

void RtmpManager::imageArrive(QImage* image)
{
    emit receiveCameraImage(image);
    delete image;
}

void RtmpManager::rtmpPullThreadFinish()
{
    m_rtmpPullThread = nullptr;
}
