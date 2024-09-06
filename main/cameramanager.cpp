#include "cameramanager.h"
#include <QTimer>
#include "liveswapmanager.h"

CameraManager::CameraManager(QObject *parent)
    : QObject{parent}
{

}

CameraManager* CameraManager::getInstance()
{
    static CameraManager* instance = new CameraManager();
    return instance;
}

void CameraManager::refreshCameras()
{
    m_cameras.clear();
    avdevice_register_all();
    const AVInputFormat *ifmt = av_find_input_format("vfwcap");
    if (ifmt != nullptr)
    {
        m_cameras.append(ifmt);
    }
}

void CameraManager::startReadCamera()
{
    if (m_cameraReadThread)
    {
        if (!m_cameraReadThreadStopping)
        {
            qInfo("the thread of reading camera is in running status");
            emit startReadCameraResult(true);
            return;
        }
        else
        {
            qInfo("the thread of reading camera is in stopping status");
            QTimer* timer = new QTimer();
            timer->setInterval(100);
            connect(timer, &QTimer::timeout, [this, timer]() {
                if (m_cameraReadThread == nullptr)
                {
                    bool result = startReadCameraDirectly();
                    emit startReadCameraResult(result);
                    timer->stop();
                    timer->deleteLater();
                }
            });
            timer->start();
            return;
        }
    }
    else
    {
        bool result = startReadCameraDirectly();
        emit startReadCameraResult(result);
    }
}

void CameraManager::stopReadCamera()
{
    if (m_cameraReadThread == nullptr)
    {
        return;
    }

    if (m_cameraReadThreadStopping)
    {
        qInfo("the camera reading thread is stopping");
        return;
    }

    m_cameraReadThreadStopping = true;
    m_cameraReadThread->setExit();
}

bool CameraManager::startReadCameraDirectly()
{
    if (m_currentCamera == nullptr)
    {
        qCritical("camera is not selected");
        return false;
    }

    m_cameraReadThread = new CameraReadThread();
    m_cameraReadThread->setCamera(m_currentCamera);
    m_cameraReadThread->setRtmpPushUrl(LiveSwapManager::getInstance()->getPushUrl());
    m_cameraReadThread->setEnableImageArriveSignal();
    connect(m_cameraReadThread, &CameraReadThread::imageArrive, this, &CameraManager::imageArrive);
    connect(m_cameraReadThread, &CameraReadThread::runFinish, this, &CameraManager::cameraReadThreadFinish);
    connect(m_cameraReadThread, &CameraReadThread::finished, m_cameraReadThread, &QObject::deleteLater);
    m_cameraReadThread->start();
    return true;
}

void CameraManager::imageArrive(QImage* image)
{
    emit receiveCameraImage(image);
    delete image;
}

void CameraManager::cameraReadThreadFinish()
{
    m_cameraReadThread = nullptr;
    m_cameraReadThreadStopping = false;
}
