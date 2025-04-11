#ifndef VIRTUALCAMERAMANAGER_H
#define VIRTUALCAMERAMANAGER_H

#include <QObject>

class VirtualCameraManager : public QObject
{
    Q_OBJECT
public:
    explicit VirtualCameraManager(QObject *parent = nullptr);

public:
    static VirtualCameraManager* getInstance();

    void sendFrame(int width, int height, void* data);    

    void enableChangeCameraSize() { m_enableChangeCameraSize = true; }

private:
    bool m_enableChangeCameraSize = false;
};

#endif // VIRTUALCAMERAMANAGER_H
