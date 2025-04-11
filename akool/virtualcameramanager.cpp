#include "virtualcameramanager.h"
#include <QFile>
#include "softcam.h"
#include "settingmanager.h"
#include "../Utility/LogMacro.h"

VirtualCameraManager::VirtualCameraManager(QObject *parent)
    : QObject{parent}
{

}

VirtualCameraManager* VirtualCameraManager::getInstance()
{
    static VirtualCameraManager* instance = new VirtualCameraManager();
    return instance;
}

void VirtualCameraManager::sendFrame(int imgWidth, int imgHeight, void* data)
{
    static scCamera camera = nullptr;
    static int width = 0;
    static int height = 0;
    bool needCreateCamera = false;
    if (camera == nullptr)
    {
        needCreateCamera = true;
    }
    else
    {
        if ((imgWidth != width || imgHeight != height) && m_enableChangeCameraSize)
        {
            needCreateCamera = true;
        }
    }

    if (needCreateCamera)
    {
        if (camera)
        {
            scDeleteCamera(camera);
        }
        camera = scCreateCamera(0, imgWidth, imgHeight, 60);
        if (camera == nullptr)
        {
            qCritical("failed to create virtual camera");
            return;
        }
        qInfo("create virtual camera (%d*%d)", imgWidth, imgHeight);

        width = imgWidth;
        height = imgHeight;
        m_enableChangeCameraSize = false;
    }

    if (imgWidth != width || imgHeight != height)
    {
        if (SettingManager::getInstance()->m_nLogLevel >= (int)ELogLevel::LOG_LEVEL_DEBUG)
        {
            qDebug("virtual camera, recv frame size is (%d, %d), desired size is (%d, %d)", imgWidth, imgHeight, width, height);
        }
        return;
    }

    int dataSize = imgWidth * imgHeight * 4;
    scSendFrame(camera, data, dataSize);
}
