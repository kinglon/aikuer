#include "meetingcontroller.h"
#include "settingmanager.h"
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include "virtualcameramanager.h"

#define URI_GET_AGORA_CONFIG "/meeting/agoraConfig"

MeetingController::MeetingController(QObject *parent)
    : HttpClientBase{parent}
{

}

void MeetingController::run()
{
    emit printLog(QString::fromWCharArray(L"开始获取频道信息"));
    sendGetAgoraConfigRequest();
}

void MeetingController::requestStop()
{
    if (m_requestStop)
    {
        return;
    }

    m_requestStop = true;
    unInitAgoraSdk();
    QTimer::singleShot(500, [this](){
        emit runFinish();
    });
}

void MeetingController::sendGetAgoraConfigRequest()
{
    QNetworkRequest request;
    QUrl url(SettingManager::getInstance()->m_meetingHost + URI_GET_AGORA_CONFIG);
    request.setUrl(url);
    addCommonHeader(request);    
    m_networkAccessManager.get(request);
}

void MeetingController::onHttpResponse(QNetworkReply *reply)
{
    QString url = reply->request().url().toString();
    if (url.indexOf(URI_GET_AGORA_CONFIG) >= 0)
    {
        if (!handleGetAgoraConfigResponse(reply))
        {
            // 请求失败，重试
            QTimer* timer = new QTimer(this);
            connect(timer, &QTimer::timeout, [this, timer]() {
                sendGetAgoraConfigRequest();
                timer->stop();
                timer->deleteLater();
            });
            timer->start(2000);
        }
    }
}

bool MeetingController::handleGetAgoraConfigResponse(QNetworkReply *reply)
{    
    if (reply->error() != QNetworkReply::NoError)
    {
        qCritical("failed to send request of getting agora configuration, error: %d", reply->error());
        emit printLog(QString::fromWCharArray(L"获取频道信息失败，错误是%1").arg(reply->error()));
        return false;
    }

    QByteArray data = reply->readAll();
    QJsonDocument jsonDocument = QJsonDocument::fromJson(data);
    if (jsonDocument.isNull() || jsonDocument.isEmpty())
    {
        qCritical("failed to parse the json data");
        return false;
    }

    QJsonObject root = jsonDocument.object();
    if (!root.contains("code"))
    {
        qCritical("failed to parse the json data, code field is missing");
        return false;
    }

    int code = root["code"].toInt();
    if (code != 1000)
    {
        qCritical("failed to get agora configuration, error is %d", code);
        emit printLog(QString::fromWCharArray(L"获取频道信息失败，错误是%1").arg(code));
        return false;
    }

    if (!root.contains("data")
            || !root["data"].toObject().contains("agora_app_id")
            || !root["data"].toObject().contains("agora_token")
            || !root["data"].toObject().contains("agora_channel"))
    {
        qCritical("failed to parse the json data, some field is missing");
        return false;
    }


    QString appId = root["data"].toObject()["agora_app_id"].toString();
    QString token = root["data"].toObject()["agora_token"].toString();
    QString channel = root["data"].toObject()["agora_channel"].toString();
    emit printLog(QString::fromWCharArray(L"获取频道信息成功，频道是%1").arg(channel));    

    if (initAgoraSdk(appId))
    {
        ChannelMediaOptions options;
        options.channelProfile = CHANNEL_PROFILE_COMMUNICATION;
        options.clientRoleType = CLIENT_ROLE_BROADCASTER;
        options.autoSubscribeAudio = true;
        options.autoSubscribeVideo = true;
        int ret = m_rtcEngine->joinChannel(token.toStdString().c_str(),
                                 channel.toStdString().c_str(),
                                 100010, options);
        if (ret != 0)
        {
            emit printLog(QString::fromWCharArray(L"加入频道失败"));
            qCritical("failed to call joinChannel, error: %d", ret);
        }
    }

    return true;
}

void MeetingController::onJoinChannelSuccess(const char* channel, uid_t uid, int )
{
    (void)channel;
    (void)uid;
    emit printLog(QString::fromWCharArray(L"加入频道成功"));

    // 把音频播放设备设置为VAC的虚拟麦克风Line 1
    bool useVirtualMicrophoneSuccess = false;
    if (m_audioDeviceManager)
    {
        IAudioDeviceCollection *audioPlaybackDevices = (*m_audioDeviceManager)->enumeratePlaybackDevices();
        if (audioPlaybackDevices)
        {
            char szDeviceName[1024] = {0};
            char szDeviceId[1024] = {0};
            for (int i = 0; i < audioPlaybackDevices->getCount(); i++)
            {
                int result = audioPlaybackDevices->getDevice(i, szDeviceName, szDeviceId);
                if (result == 0)
                {
                    qInfo("virtual microphone, name=%s, device id=%s", szDeviceName, szDeviceId);
                    if (strstr(szDeviceName, "Line 1"))
                    {
                        if ((*m_audioDeviceManager)->setPlaybackDevice(szDeviceId) == 0)
                        {
                            useVirtualMicrophoneSuccess = true;
                        }
                        break;
                    }
                }
            }
            audioPlaybackDevices->release();
        }
    }

    if (useVirtualMicrophoneSuccess)
    {
        emit printLog(QString::fromWCharArray(L"使用虚拟麦克风作为音频播放设备成功"));
    }
    else
    {
        emit printLog(QString::fromWCharArray(L"使用虚拟麦克风作为音频播放设备失败"));
    }
}

void MeetingController::onError(int err, const char* msg)
{
    (void)err;
    qCritical("agora sdk has error: %s", msg);
}

bool MeetingController::initAgoraSdk(QString appId)
{
    if (m_rtcEngine)
    {
        return true;
    }

    //create Agora RTC engine
    m_rtcEngine = createAgoraRtcEngine();
    if (!m_rtcEngine)
    {
        emit printLog(QString::fromWCharArray(L"初始化声网SDK失败"));
        qCritical("failed to call createAgoraRtcEngine");
        return false;
    }

    RtcEngineContext context;
    std::string appIdString = appId.toStdString();
    context.appId = appIdString.c_str();
    context.eventHandler = this;
    context.channelProfile = CHANNEL_PROFILE_COMMUNICATION;
    int ret = m_rtcEngine->initialize(context);
    if (ret != 0)
    {
        emit printLog(QString::fromWCharArray(L"初始化声网SDK失败"));
        qCritical("failed to call initialize, error: %d", ret);
        return false;
    }

    m_audioDeviceManager = new AAudioDeviceManager(m_rtcEngine);

    m_rtcEngine->enableAudio();
    m_rtcEngine->setAudioProfile(AUDIO_PROFILE_IOT);
    agora::util::AutoPtr<agora::media::IMediaEngine> mediaEngine;
    mediaEngine.queryInterface(m_rtcEngine, AGORA_IID_MEDIA_ENGINE);
    if (mediaEngine.get())
    {
        ret = mediaEngine->registerVideoFrameObserver(this);
        if (ret != 0)
        {
            qCritical("failed to call registerVideoFrameObserver, error: %d", ret);
        }
    }
    emit printLog(QString::fromWCharArray(L"初始化声网SDK成功"));
    return true;
}

void MeetingController::unInitAgoraSdk()
{
    if (m_audioDeviceManager)
    {
        m_audioDeviceManager->release();
        m_audioDeviceManager = nullptr;
    }

    if (m_rtcEngine)
    {
        m_rtcEngine->leaveChannel();
        m_rtcEngine->release(true);
        m_rtcEngine = NULL;
    }
}

bool MeetingController::onRenderVideoFrame(const char* channelId, rtc::uid_t remoteUid, VideoFrame& videoFrame)
{
    (void)channelId;
    (void)remoteUid;

    if (videoFrame.type != agora::media::base::VIDEO_PIXEL_BGRA)
    {
        qCritical("the type of video frame is not VIDEO_PIXEL_BGRA");
        return false;
    }

    VirtualCameraManager::getInstance()->sendFrame(videoFrame.width, videoFrame.height, videoFrame.yBuffer);

    if (m_enableGenerateQImage)
    {
        QImage* image = new QImage(videoFrame.width, videoFrame.height, QImage::Format_RGB888);
        for (int y = 0; y < videoFrame.height; y++)
        {
            for (int x = 0; x < videoFrame.width; x++)
            {
                uint8_t *ptr = image->scanLine(y) + x * 3;
                ptr[0] = videoFrame.yBuffer[y * videoFrame.width*4 + x * 4 + 2];
                ptr[1] = videoFrame.yBuffer[y * videoFrame.width*4 + x * 4 + 1];
                ptr[2] = videoFrame.yBuffer[y * videoFrame.width*4 + x * 4];
            }
        }
        m_mutex.lock();
        if (m_lastImage)
        {
            delete m_lastImage;
        }
        m_lastImage = image;
        m_mutex.unlock();
    }

    return false;
}

QImage* MeetingController::popImage()
{
    QImage* image = nullptr;
    m_mutex.lock();
    image = m_lastImage;
    m_lastImage = nullptr;
    m_mutex.unlock();
    return image;
}
