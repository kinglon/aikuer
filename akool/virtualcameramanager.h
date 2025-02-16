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
};

#endif // VIRTUALCAMERAMANAGER_H
