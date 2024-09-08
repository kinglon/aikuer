#include "rtmppushthread.h"
#include <Windows.h>

RtmpPushThread::RtmpPushThread(QObject *parent)
    : QThread{parent}
{

}

RtmpPushThread::~RtmpPushThread()
{
    m_mutex.lock();
    while(!m_frames.empty())
    {
        AVFrame* frame = m_frames.front();
        av_frame_free(&frame);
        m_frames.pop_front();
    }
    m_mutex.unlock();
}

void RtmpPushThread::run()
{
    run2();
    emit runFinish();
}

void RtmpPushThread::run2()
{
    // 查找H264编码器
    const AVCodec *h264Codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!h264Codec)
    {
        qCritical("failed to find h264 encoder");
        return;
    }

    // 分配编码器上下文
    AVCodecContext *h264CodecCtx = avcodec_alloc_context3(h264Codec);
    if (!h264CodecCtx)
    {
        qCritical("failed to create h264 encoder context");
        return;
    }

    // 设置编码器参数
    h264CodecCtx->codec_id = AV_CODEC_ID_H264;
    h264CodecCtx->bit_rate = 4000000;
    h264CodecCtx->width = m_width;
    h264CodecCtx->height = m_height;
    h264CodecCtx->time_base = {1, 30};
    h264CodecCtx->framerate = {30, 1};
    h264CodecCtx->gop_size = 10;
    h264CodecCtx->max_b_frames = 0;
    h264CodecCtx->pix_fmt = m_format;

    // 打开编码器
    int result = avcodec_open2(h264CodecCtx, h264Codec, NULL);
    if (result < 0)
    {
        qCritical("failed to open h264 encoder, error is %d", result);
        avcodec_free_context(&h264CodecCtx);
        return;
    }

    AVFormatContext* pRtmpFormatCtx = nullptr;
    result = avformat_alloc_output_context2(&pRtmpFormatCtx, nullptr, "flv", m_rtmpPushUrl.toStdString().c_str());
    if (result < 0)
    {
        qCritical("failed to create flv RTMP context, error is %d", result);
        avcodec_free_context(&h264CodecCtx);
        return;
    }

    // Add video stream to the RTMP context
    AVStream* pRtmpVideoStream = avformat_new_stream(pRtmpFormatCtx, h264Codec);
    if (!pRtmpVideoStream)
    {
        qCritical("failed to create RTMP video stream");
        avcodec_free_context(&h264CodecCtx);
        avformat_free_context(pRtmpFormatCtx);
        return;
    }

    avcodec_parameters_from_context(pRtmpVideoStream->codecpar, h264CodecCtx);

    // Open the RTMP output
    result = avio_open(&pRtmpFormatCtx->pb, m_rtmpPushUrl.toStdString().c_str(), AVIO_FLAG_WRITE);
    if (result < 0)
    {
        qCritical("failed to open RTMP output, error is %d", result);
        avcodec_free_context(&h264CodecCtx);
        avformat_free_context(pRtmpFormatCtx);
        return;
    }

    // Write the header to the RTMP output
    result = avformat_write_header(pRtmpFormatCtx, NULL);
    if (result < 0)
    {
        qCritical("failed to write header to RTMP output, error is %d", result);
        avcodec_free_context(&h264CodecCtx);
        avformat_free_context(pRtmpFormatCtx);
        return;
    }

    int frameCount = 0;
    qint64 lastTime = GetTickCount64();
    while(!m_exit)
    {
        // 统计帧率
        qint64 elapse = GetTickCount64() - lastTime;
        if (elapse > 10000)
        {
            int frameRate = int(frameCount/(elapse/1000.0f));
            qInfo("rtmp push frame rate is %d", frameRate);
            lastTime = GetTickCount64();
            frameCount = 0;
        }

        AVFrame* avFrame = popFrame();
        if (avFrame == nullptr)
        {
            QThread::msleep(5);
            continue;
        }

        // h264 encode
        avcodec_send_frame(h264CodecCtx, avFrame);

        AVPacket* avPacket = av_packet_alloc();
        result = avcodec_receive_packet(h264CodecCtx, avPacket);
        if (result >= 0)
        {
            av_interleaved_write_frame(pRtmpFormatCtx, avPacket);
            frameCount += 1;
        }
        else
        {
            if (result != AVERROR(EAGAIN))
            {
                qCritical("failed to do the h264 encode, error is %d", result);
                av_packet_free(&avPacket);
                av_frame_free(&avFrame);
                break;
            }
        }

        av_packet_free(&avPacket);
        av_frame_free(&avFrame);
    }

    avcodec_free_context(&h264CodecCtx);
    avformat_free_context(pRtmpFormatCtx);
}

void RtmpPushThread::pushFrame(AVFrame* newFrame)
{
    m_mutex.lock();
    if (m_frames.size() >= m_frameCacheSize)
    {
        AVFrame* frame = m_frames.front();
        av_frame_free(&frame);
        m_frames.pop_front();
    }
    m_frames.push_back(newFrame);
    m_mutex.unlock();
}

AVFrame* RtmpPushThread::popFrame()
{
    m_mutex.lock();
    AVFrame* frame = nullptr;
    if (m_frames.size() > 0)
    {
        frame = m_frames.front();
        m_frames.pop_front();
    }
    m_mutex.unlock();
    return frame;
}
