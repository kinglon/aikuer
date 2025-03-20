#include "meetingcontroller.h"
#include "settingmanager.h"
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QImage>
#include "virtualcameramanager.h"

#define URI_CREATE_SESSION "/api/v7/content/liveAvatar/session/create"
#define URI_END_SESSION "/api/v7/content/liveAvatar/session/close"

MeetingController::MeetingController(QObject *parent)
    : HttpClientBase{parent}
{

}

void MeetingController::run()
{
    if (!m_avatar.isValid())
    {
        qCritical("avatar id is empty");
        return;
    }

    m_isRunning = true;

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MeetingController::onMainTimer);
    timer->start(1000);
    onMainTimer();
}

void MeetingController::setAvatar(const Avatar& avatar)
{
    if (!avatar.isValid() || avatar.m_avatarId == m_avatar.m_avatarId)
    {
        return;
    }

    m_avatar = avatar;
    if (m_currentState != MEETING_STATE_INIT)
    {
        restartMeeting();
    }
}

void MeetingController::onMainTimer() {
    if (m_currentState == MEETING_STATE_INIT)
    {
        createSession();
    }
    else if (m_currentState == MEETING_STATE_CREATE_SESSION)
    {
        if (!m_createSessionSuccess && !m_creatingSession)
        {
            createSession();
        }
    }
}

void MeetingController::requestStop()
{
    if (m_requestStop)
    {
        return;
    }

    m_requestStop = true;
    leaveChannel();
    unInitAgoraSdk();
}

void MeetingController::createSession()
{
    QNetworkRequest request;
    QUrl url(SettingManager::getInstance()->m_host + URI_CREATE_SESSION);
    request.setUrl(url);
    addCommonHeader(request);
    QString bearerToken = "Bearer ";
    bearerToken += SettingManager::getInstance()->m_loginToken;
    request.setRawHeader("Authorization", bearerToken.toUtf8());

    QJsonObject bodyJson;
    bodyJson["avatar_id"] = m_avatar.m_avatarIdForService;
    bodyJson["scene_mode"] = "meeting";
    m_networkAccessManager.post(request, QJsonDocument(bodyJson).toJson());

    qInfo("send the request of creating session");
    m_currentState = MEETING_STATE_CREATE_SESSION;
    m_creatingSession = true;
}

void MeetingController::endSession()
{
    if (m_meetingSessionId.isEmpty())
    {
        return;
    }

    QNetworkRequest request;
    QUrl url(SettingManager::getInstance()->m_host + URI_END_SESSION);
    request.setUrl(url);
    addCommonHeader(request);
    QString bearerToken = "Bearer ";
    bearerToken += SettingManager::getInstance()->m_loginToken;
    request.setRawHeader("Authorization", bearerToken.toUtf8());

    QJsonObject bodyJson;
    bodyJson["id"] = m_meetingSessionId;
    m_networkAccessManager.post(request, QJsonDocument(bodyJson).toJson());
}

void MeetingController::onHttpResponse(QNetworkReply *reply)
{
    QString url = reply->request().url().toString();
    if (url.indexOf(URI_CREATE_SESSION) >= 0)
    {
        m_creatingSession = false;
        m_createSessionSuccess = handleCreateSessionResponse(reply);
    }
}

bool MeetingController::handleCreateSessionResponse(QNetworkReply *reply)
{    
    if (reply->error() != QNetworkReply::NoError)
    {
        qCritical("failed to send request of creating session, error: %d", reply->error());
        return false;
    }

    QByteArray data = reply->readAll();
    if (SettingManager::getInstance()->enableDebugLog())
    {
        qDebug("create session response: %s", QString::fromUtf8(data).toStdString().c_str());
    }
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
        if (root.contains("msg"))
        {
            qCritical("msg: %s", root["msg"].toString().toStdString().c_str());
        }
        return false;
    }

    int code = root["code"].toInt();
    if (code != 1000)
    {
        qCritical("failed to create session, error is %d", code);
        if (root.contains("msg"))
        {
            qCritical("msg: %s", root["msg"].toString().toStdString().c_str());
            emit hasError(root["msg"].toString());
        }
        m_currentState = MEETING_STATE_JOIN_MEETING_FAILED;
        return false;
    }

    if (!root.contains("data") || !root["data"].toObject().contains("credentials"))
    {
        qCritical("failed to parse the json data, credentials field is missing");
        return false;
    }

    m_meetingSessionId = root["data"].toObject()["_id"].toString();

    QJsonObject credentialsJson = root["data"].toObject()["credentials"].toObject();
    int uid = credentialsJson["agora_uid"].toInt();
    QString appId = credentialsJson["agora_app_id"].toString();
    QString token = credentialsJson["agora_token"].toString();
    QString channel = credentialsJson["agora_channel"].toString();

    qInfo("agora uid is %d", uid);
    qInfo("agora app id is %s", appId.toStdString().c_str());
    qInfo("agora token is %s", token.toStdString().c_str());
    qInfo("agora channel is %s", channel.toStdString().c_str());

    m_currentState = MEETING_STATE_JOIN_MEETING;
    joinChannel(appId, token, channel, uid);

    return true;
}

bool MeetingController::joinChannel(const QString& appId, const QString& token, const QString& channel, int uid)
{
    if (!initAgoraSdk(appId))
    {
        return false;
    }

    ChannelMediaOptions options;
    options.channelProfile = CHANNEL_PROFILE_COMMUNICATION;
    options.clientRoleType = CLIENT_ROLE_BROADCASTER;
    options.autoSubscribeAudio = true;
    options.autoSubscribeVideo = true;
    options.enableAudioRecordingOrPlayout = true;
    options.publishMicrophoneTrack = true;
    int ret = m_rtcEngine->joinChannel(token.toStdString().c_str(),
                             channel.toStdString().c_str(),
                             uid, options);
    if (ret != 0)
    {
        qCritical("failed to call joinChannel, error: %d", ret);
        return false;
    }

    return true;
}

void MeetingController::onJoinChannelSuccess(const char* channel, uid_t uid, int )
{
    (void)channel;
    (void)uid;
    qInfo("join channel successfully");
    m_currentState = MEETING_STATE_IN_MEETING;

    // 把音频播放设备设置为VAC的虚拟麦克风Line 1
    bool useVirtualMicrophoneSuccess = false;
    if (m_audioDeviceManager)
    {
        m_audioDeviceManager->get()->followSystemPlaybackDevice(false);

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
        qInfo("successful to use VAC");
    }
    else
    {
        qInfo("failed to use VAC");
    }
}

void MeetingController::onError(int err, const char* msg)
{
    (void)err;
    qCritical("agora sdk has error: %d, msg: %s", err, msg);
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
        qCritical("failed to call initialize, error: %d", ret);
        return false;
    }

    m_audioDeviceManager = new AAudioDeviceManager(m_rtcEngine);

    ret = m_rtcEngine->enableAudio();
    if (ret != 0)
    {
        qCritical("failed to call enableAudio, error: %d", ret);
    }

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
    qInfo("init agora successfully");
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

void MeetingController::leaveChannel()
{
    if (m_currentState > MEETING_STATE_CREATE_SESSION)
    {
        if (m_rtcEngine)
        {
            m_rtcEngine->leaveChannel();
        }
    }

    endSession();
}

void MeetingController::restartMeeting()
{
    leaveChannel();

    m_creatingSession = false;
    m_createSessionSuccess = false;
    m_currentState = MEETING_STATE_CREATE_SESSION;
    onMainTimer();
}
