#include "virtualcameramanager.h"
#include "Utility/ImPath.h"
#include "rtmpmanager.h"
#include <QFile>
#include "softcam.h"
#include "settingmanager.h"
#include "Utility/LogMacro.h"

VirtualCameraManager::VirtualCameraManager(QObject *parent)
    : QObject{parent}
{

}

VirtualCameraManager* VirtualCameraManager::getInstance()
{
    static VirtualCameraManager* instance = new VirtualCameraManager();
    return instance;
}

bool VirtualCameraManager::enableVirtualCamera(bool enable)
{
    m_enableVirtualCamera = enable;
    if (!enable)
    {        
        RtmpManager::getInstance()->setRtmpFrameArriveCallback(nullptr);        
    }
    else
    {
        RtmpManager::getInstance()->setRtmpFrameArriveCallback(this);
    }
    return true;
}

void VirtualCameraManager::onRtmpFrameArrive(AVFrame* frame)
{
    sendFrame(frame);
    av_frame_free(&frame);
}

void VirtualCameraManager::sendFrame(const AVFrame* frame)
{
    if (!m_enableVirtualCamera)
    {        
        return;
    }

    static scCamera camera = nullptr;
    static int width = 0;
    static int height = 0;
    if (camera == nullptr)
    {
        width = frame->width;
        height = frame->height;
        camera = scCreateCamera(0, width, height, 60);
        if (camera == nullptr)
        {            
            return;
        }
    }

    if (frame->width != width || frame->height != height)
    {
        if (SettingManager::getInstance()->m_nLogLevel >= (int)ELogLevel::LOG_LEVEL_DEBUG)
        {
            qDebug("frame size is (%d, %d), desired size is (%d, %d)", frame->width, frame->height, width, height);
        }
        return;
    }

    // rgb24转成bgra后推给虚拟摄像头
    AVFrame *bgraFrame = av_frame_alloc();
    bgraFrame->width = frame->width;
    bgraFrame->height = frame->height;
    bgraFrame->format = AV_PIX_FMT_BGRA;
    av_frame_get_buffer(bgraFrame, 0);
    SwsContext *swsContext = sws_getContext(frame->width, frame->height, AV_PIX_FMT_RGB24,
                                                bgraFrame->width, bgraFrame->height, AV_PIX_FMT_BGRA,
                                                SWS_BILINEAR, NULL, NULL, NULL);
    sws_scale(swsContext, frame->data, frame->linesize, 0, frame->height,
              bgraFrame->data, bgraFrame->linesize);

    uint8_t *bgraData = bgraFrame->data[0];
    int dataSize = frame->width * frame->height * 4; // 4 bytes per pixel for BGRA format
    scSendFrame(camera, bgraData, dataSize);

    av_frame_free(&bgraFrame);
    sws_freeContext(swsContext);
}

void VirtualCameraManager::sendFrame(int imgWidth, int imgHeight, void* data)
{
    if (!m_enableVirtualCamera)
    {
        return;
    }

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
