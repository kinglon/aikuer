#include "memoryimageprovider.h"
#include "statusmanager.h"

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
            if (StatusManager::getInstance()->m_currentMeetingMode == MEETING_MODE_SA)
            {
                QImage avatarImage = m_mainController->getAvatarVideoPlayer().getCurrentVideoFrame();
                if (!avatarImage.isNull())
                {
                    retImage = &avatarImage;
                }
            }

            if (retImage == nullptr)
            {
                // 默认使用完全透明的图片
                static QImage transparentImg;
                if (transparentImg.isNull())
                {
                    transparentImg = QImage(20, 20, QImage::Format_ARGB32);
                    transparentImg.fill(Qt::transparent);
                }
                retImage = &transparentImg;
            }
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
