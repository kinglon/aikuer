#ifndef ECHOREMOVER_H
#define ECHOREMOVER_H

#include <QThread>
#include <QObject>
#include <Windows.h>
#include <mmreg.h>
#include <mmsystem.h>

#include <IAgoraRtcEngine.h>
#include <IAgoraMediaEngine.h>
#include <AgoraMediaBase.h>

using namespace agora;
using namespace agora::rtc;
using namespace agora::media;


class EchoRemover : public QThread
{
    Q_OBJECT

public:
    explicit EchoRemover(QObject *parent = nullptr);
    ~EchoRemover();

public:
    void setRtcEngine(IRtcEngine* engine) { m_rtcEngine = engine; }

    void setMicrophoneName(QString microphoneName) { m_microphoneName = microphoneName; }

    void setCustomAudioTrackId(track_id_t trackId) { m_audioTrackId = trackId; }

    void setEnabled(bool enabled) { m_enabled = enabled; }

protected:
    virtual void run() override;

private:
    // 回调函数：播放音频
    static void CALLBACK waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

private:
    IRtcEngine* m_rtcEngine = nullptr;

    QString m_microphoneName;

    rtc::track_id_t m_audioTrackId = INVALID_TRACK_ID;

    bool m_enabled = false;

    // 正在播放的声音
    QVector<LPWAVEHDR> m_playingWaves;

    // 已经播放完的声音
    QVector<LPWAVEHDR> m_finishPalyWaves;

    // 临界区，同步使用
    CRITICAL_SECTION m_cs;
};

#endif // ECHOREMOVER_H
