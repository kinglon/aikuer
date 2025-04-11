#ifndef MYVIDEOPLAYER_H
#define MYVIDEOPLAYER_H

#include <QThread>
#include <QObject>
#include <QAtomicInteger>
#include <QMutex>
#include <QImage>

class MyVideoPlayer : public QThread
{
    Q_OBJECT
public:
    explicit MyVideoPlayer(QObject *parent = nullptr);

    void play() { m_isPlay = 1; }
    void pause() { m_isPlay = 0; }

    void setVideoFilePath(QString newVideoFilePath);

    QImage getCurrentVideoFrame();

protected:
    virtual void run() override;

private:
    bool playVideo(const QString& videoFilePath);

private:
    QMutex m_mutex;

    QString m_videoFilePath;

    // 更新视频文件后，需要重新加载
    QAtomicInteger<int> m_needReload;

    QImage* m_currentVideoFrame = nullptr;

    QAtomicInteger<int> m_isPlay = 0;
};

#endif // MYVIDEOPLAYER_H
