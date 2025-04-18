﻿#ifndef VIRTUALCAMERAMANAGER_H
#define VIRTUALCAMERAMANAGER_H

#include <QObject>
#include "rtmpmanager.h"

class VirtualCameraManager : public QObject, public IRtmpFrameArriveCallback
{
    Q_OBJECT
public:
    explicit VirtualCameraManager(QObject *parent = nullptr);

public:
    static VirtualCameraManager* getInstance();

    bool enableVirtualCamera(bool enable);

    virtual void onRtmpFrameArrive(AVFrame* frame) override;

    void sendFrame(int width, int height, void* data);

private:
    void sendFrame(const AVFrame* frame);

private:
    bool m_enableVirtualCamera = false;
};

#endif // VIRTUALCAMERAMANAGER_H
