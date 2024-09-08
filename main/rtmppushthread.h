#ifndef RTMPPUSHTHREAD_H
#define RTMPPUSHTHREAD_H

#include <QThread>
#include <QObject>
#include <QQueue>
#include <QMutex>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/rational.h>
}


class RtmpPushThread : public QThread
{
    Q_OBJECT
public:
    explicit RtmpPushThread(QObject *parent = nullptr);

    ~RtmpPushThread();

public:
    void setExit() { m_exit = true; }

    void setRtmpPushUrl(QString rtmpPushUrl) { m_rtmpPushUrl = rtmpPushUrl; }

    void setSize(int width, int height) { m_width = width; m_height = height; }

    void setFrameCacheSize(int frameCacheSize) { m_frameCacheSize = frameCacheSize; }

    void setFormat(AVPixelFormat format) { m_format = format; }

    void pushFrame(AVFrame* newFrame);

signals:
    void runFinish();

protected:
    void run() override;

private:
    void run2();

    AVFrame* popFrame();

private:
    bool m_exit = false;

    QString m_rtmpPushUrl;

    int m_width = 0;

    int m_height = 0;

    AVPixelFormat m_format = AV_PIX_FMT_YUV420P;

    QQueue<AVFrame*> m_frames;

    int m_frameCacheSize = 10;

    QMutex m_mutex;
};

#endif // RTMPPUSHTHREAD_H
