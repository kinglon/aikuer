#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <QObject>
#include <QVector>
#include "camerareadthread.h"

extern "C"
{
    #include "libavdevice/avdevice.h"
}

class CameraManager : public QObject
{
    Q_OBJECT
public:
    explicit CameraManager(QObject *parent = nullptr);

public:
    static CameraManager* getInstance();

    void refreshCameras();

    QVector<const AVInputFormat*> getCameras() { return m_cameras; }

    void setCurrentCamera(const AVInputFormat* camera) { m_currentCamera = camera; }

    void startReadCamera();

    void stopReadCamera();

private:
    // 直接启动读摄像头，无需等待退出
    bool startReadCameraDirectly();

signals:
    void startReadCameraResult(bool ok);

    void receiveCameraImage(const QImage* image);

private slots:
    void imageArrive(QImage* image);

    void cameraReadThreadFinish();

private:
    QVector<const AVInputFormat*> m_cameras;

    const AVInputFormat* m_currentCamera = nullptr;

    CameraReadThread* m_cameraReadThread = nullptr;

    // 标志摄像头读线程是否正在停止中
    bool m_cameraReadThreadStopping = false;
};

#endif // CAMERAMANAGER_H
