#include "memoryimageprovider.h"

QImage MemoryImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    (void)requestedSize;

    if (m_mainController == nullptr)
    {
        return QImage();
    }

    QImage* retImage = nullptr;
    if (id == "videoPlayer")
    {
        QImage remoteImage = m_mainController->getMeetingController().getRemoteImage();
        if (!remoteImage.isNull())
        {
            retImage = &remoteImage;
        }
        else
        {
            // todo by yejinlong, 有选择avatar就播放avatar视频

            // 默认等待播放图片
            static QImage waitingImage;
            if (waitingImage.isNull())
            {
                waitingImage.load(":/content/res/waiting_play.png");
            }
            retImage = &waitingImage;
        }
    }
    else if (id == "cameraImage")
    {
        QImage localImage = m_mainController->getMeetingController().getLocalImage();
        if (!localImage.isNull())
        {
            retImage = &localImage;
        }
        else
        {
            // 默认无摄像头画面
            static QImage noCameraImage;
            if (noCameraImage.isNull())
            {
                noCameraImage.load(":/content/res/no_camera_bg.png");
            }
            retImage = &noCameraImage;
        }
    }

    if (retImage)
    {
        if (size)
        {
            *size = retImage->size();
        }
        return *retImage;
    }

    return QImage();
}
