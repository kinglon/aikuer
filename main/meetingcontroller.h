#ifndef MEETINGCONTROLLER_H
#define MEETINGCONTROLLER_H

#include "httpclientbase.h"
#include <IAgoraRtcEngine.h>
#include <IAgoraMediaEngine.h>
#include <QMutex>

using namespace agora;
using namespace agora::rtc;
using namespace agora::media;

class MeetingController : public HttpClientBase, public IRtcEngineEventHandler, public IVideoFrameObserver
{
    Q_OBJECT
public:
    explicit MeetingController(QObject *parent = nullptr);

public:
    void run();

    void requestStop();

    // 用完要释放
    QImage* popImage();

    void enableGenerateQImage() { m_enableGenerateQImage = true; }

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

signals:
    void printLog(QString content);

    void runFinish();

private:
    void sendGetAgoraConfigRequest();

    bool handleGetAgoraConfigResponse(QNetworkReply *reply);

    bool initAgoraSdk(QString appId);

    void unInitAgoraSdk();

private:
    QMutex m_mutex;

    QImage* m_lastImage = nullptr;

    bool m_enableGenerateQImage = false;

    IRtcEngine* m_rtcEngine = nullptr;

    AAudioDeviceManager* m_audioDeviceManager = nullptr;

    bool m_requestStop = false;
};

#endif // MEETINGCONTROLLER_H
