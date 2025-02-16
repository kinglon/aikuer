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
    if (camera == nullptr)
    {
        width = imgWidth;
        height = imgHeight;
        camera = scCreateCamera(0, width, height, 60);
        if (camera == nullptr)
        {
            return;
        }
    }

    if (imgWidth != width || imgHeight != height)
    {
        if (SettingManager::getInstance()->m_nLogLevel >= (int)ELogLevel::LOG_LEVEL_DEBUG)
        {
            qDebug("frame size is (%d, %d), desired size is (%d, %d)", imgWidth, imgHeight, width, height);
        }
        return;
    }

    int dataSize = imgWidth * imgHeight * 4;
    scSendFrame(camera, data, dataSize);
}
