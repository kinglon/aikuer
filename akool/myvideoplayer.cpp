#include "myvideoplayer.h"
#include "ffmpegutil.h"

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

MyVideoPlayer::MyVideoPlayer(QObject *parent)
    : QThread{parent}
{

}

void MyVideoPlayer::setVideoFilePath(QString newVideoFilePath)
{
    m_mutex.lock();
    if (m_videoFilePath != newVideoFilePath)
    {
        m_videoFilePath = newVideoFilePath;
        m_needReload = true;
    }
    m_mutex.unlock();
}

QImage MyVideoPlayer::getCurrentVideoFrame()
{
    QImage image;
    m_mutex.lock();
    if (m_currentVideoFrame)
    {
        image = *m_currentVideoFrame;
    }
    m_mutex.unlock();
    return image;
}

void MyVideoPlayer::run()
{
    qInfo("begin to run the thread of video player");

    // Initialize FFmpeg
    avformat_network_init();

    while (true)
    {
        QThread::msleep(100);
        if (!m_isPlay)
        {
            continue;
        }

        QString videoFilePath;
        m_mutex.lock();
        videoFilePath = m_videoFilePath;
        m_mutex.unlock();
        if (videoFilePath.isEmpty())
        {
            continue;
        }

        if (!playVideo(videoFilePath))
        {
            // 播放失败，先暂停
            m_isPlay = 0;
        }
    }

    qInfo("finish to run the thread of video player");
}

bool MyVideoPlayer::playVideo(const QString& videoFilePath)
{
    // Open the video file
    AVFormatContext* videoFormatCtx = avformat_alloc_context();
    std::string filePath = videoFilePath.toStdString();
    int result = avformat_open_input(&videoFormatCtx, filePath.c_str(), nullptr, nullptr);
    if (result != 0)
    {
        qCritical("failed to open video file, error is %d, path is %s", result, filePath.c_str());
        return false;
    }

    // Find the video stream information
    result = avformat_find_stream_info(videoFormatCtx, NULL);
    if (result < 0)
    {
        qCritical("failed to find stream, error is %d", result);
        avformat_close_input(&videoFormatCtx);
        return false;
    }

    // Find the video stream
    unsigned int videoStream = -1;
    for (unsigned int i = 0; i < videoFormatCtx->nb_streams; i++)
    {
        if (videoFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            break;
        }
    }

    if (videoStream == -1)
    {
        qCritical("failed to find video stream");
        avformat_close_input(&videoFormatCtx);
        return false;
    }

    // Allocate the codec context for the video stream
    AVCodecContext* pDecoderCtx = avcodec_alloc_context3(NULL);
    AVCodecParameters* codecPar = videoFormatCtx->streams[videoStream]->codecpar;
    result = avcodec_parameters_to_context(pDecoderCtx, codecPar);
    if (result < 0)
    {
        qCritical("failed to set codec parameters, error is %d", result);
        avcodec_free_context(&pDecoderCtx);
        avformat_close_input(&videoFormatCtx);
        return false;
    }

    // Find the codec
    const AVCodec* pCodec = avcodec_find_decoder(pDecoderCtx->codec_id);
    if (!pCodec)
    {
        qCritical("failed to find decoder");
        avcodec_free_context(&pDecoderCtx);
        avformat_close_input(&videoFormatCtx);
        return false;
    }

    // Open the codec
    result = avcodec_open2(pDecoderCtx, pCodec, nullptr);
    if (result < 0)
    {
        qCritical("failed to open decoder, error is %d", result);
        avcodec_free_context(&pDecoderCtx);
        avformat_close_input(&videoFormatCtx);
        return false;
    }

    AVPacket avPacket;
    m_needReload = 0;
    while (!m_needReload)
    {
        // 判断是否暂停
        if (!m_isPlay)
        {
            QThread::msleep(100);
            continue;
        }

        // Read the frame from the camera
        result = av_read_frame(videoFormatCtx, &avPacket);
        if (result == AVERROR_EOF)
        {
            // 播放完后重新开始
            av_seek_frame(videoFormatCtx, -1, 0, AVSEEK_FLAG_BACKWARD);
            continue;
        }

        if (result < 0)
        {
            qCritical("failed to read frame");
            break;
        }

        // Decode the frame
        avcodec_send_packet(pDecoderCtx, &avPacket);
        AVFrame* avFrame = av_frame_alloc();
        result = avcodec_receive_frame(pDecoderCtx, avFrame);
        if (result < 0)
        {            
            av_packet_unref(&avPacket);
            av_frame_free(&avFrame);
            continue;
        }

        AVFrame* rgbFrame = FfmpegUtil::convertToRGB24Format(avFrame);
        QImage* image = FfmpegUtil::convertToQImage(rgbFrame);
        av_frame_free(&rgbFrame);

        m_mutex.lock();
        if (m_currentVideoFrame)
        {
            delete m_currentVideoFrame;
        }
        m_currentVideoFrame = image;
        m_mutex.unlock();

        av_frame_free(&avFrame);

        // Free the packet
        av_packet_unref(&avPacket);
    }

    // Clean up
    avcodec_free_context(&pDecoderCtx);
    avformat_close_input(&videoFormatCtx);

    return true;
}
