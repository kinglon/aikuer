#include "memoryimageprovider.h"

QImage MemoryImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    (void)requestedSize;
    QImage* retImage = nullptr;
    if (id == "videoPlayer")
    {
        if (m_mainController)
        {
            QImage* image = m_mainController->getPlayerImage();
            if (image && image != m_lastImage)
            {
                delete m_lastImage;
                m_lastImage = image;
            }
        }

        if (m_lastImage)
        {
            retImage = m_lastImage;
        }
        else
        {
            static QImage waitingImage;
            if (waitingImage.isNull())
            {
                waitingImage.load(":/content/res/waiting_play.png");
            }
            retImage = &waitingImage;
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
