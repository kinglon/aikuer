﻿#ifndef MEETINGCONTROLLER_H
#define MEETINGCONTROLLER_H

#include "../Utility/httpclientbase.h"
#include <IAgoraRtcEngine.h>
#include <IAgoraMediaEngine.h>
#include <QMutex>
#include "avatarcontroller.h"

using namespace agora;
using namespace agora::rtc;
using namespace agora::media;

#define MEETING_STATE_INIT 1
#define MEETING_STATE_CREATE_SESSION 2
#define MEETING_STATE_JOIN_MEETING 3
#define MEETING_STATE_IN_MEETING 4

class MeetingController : public HttpClientBase, public IRtcEngineEventHandler, public IVideoFrameObserver
{
    Q_OBJECT
public:
    explicit MeetingController(QObject *parent = nullptr);

public:    
    void run();

    bool isRun() { return m_isRunning; }

    void setAvatar(const Avatar& avatar);

    void requestStop();

    // 用完要释放
    QImage* popImage();    

protected:
    virtual void onHttpResponse(QNetworkReply *reply) override;

    virtual void onJoinChannelSuccess(const char* channel, uid_t uid, int elapsed) override;

    virtual void onError(int err, const char* msg) override;

    virtual agora::media::base::VIDEO_PIXEL_FORMAT getVideoFormatPreference() override { return agora::media::base::VIDEO_PIXEL_BGRA; }

    virtual bool onRenderVideoFrame(const char* channelId, rtc::uid_t remoteUid, VideoFrame& videoFrame) override;
    virtual bool onTranscodedVideoFrame(VideoFrame& )override { return true; }
    virtual bool onMediaPlayerVideoFrame(VideoFrame& , int ) override { return true; }
    virtual bool onPreEncodeVideoFrame(agora::rtc::VIDEO_SOURCE_TYPE , VideoFrame& ) override { return true; }
    virtual bool onCaptureVideoFrame(agora::rtc::VIDEO_SOURCE_TYPE , VideoFrame& ) override { return true; }

private slots:
    void onMainTimer();

private:
    void createSession();

    void endSession();

    bool handleCreateSessionResponse(QNetworkReply *reply);

    bool initAgoraSdk(QString appId);

    void unInitAgoraSdk();

    void restartMeeting();

    bool joinChannel(const QString& appId, const QString& token, const QString& channel, int uid);

    void leaveChannel();

private:
    QMutex m_mutex;

    bool m_isRunning = false;

    int m_currentState = MEETING_STATE_INIT;

    QImage* m_lastImage = nullptr;

    IRtcEngine* m_rtcEngine = nullptr;

    AAudioDeviceManager* m_audioDeviceManager = nullptr;

    bool m_requestStop = false;

    Avatar m_avatar;

    bool m_creatingSession = false;
    bool m_createSessionSuccess = false;

    QString m_meetingSessionId;
};

#endif // MEETINGCONTROLLER_H
