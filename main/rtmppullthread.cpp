#include "rtmppullthread.h"
#include "ffmpegutil.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

RtmpPullThread::RtmpPullThread()
{

}

RtmpPullThread::~RtmpPullThread()
{
    if (m_lastImage)
    {
        delete m_lastImage;
    }
}

QImage* RtmpPullThread::popImage()
{
    QImage* image = nullptr;
    m_mutex.lock();
    image = m_lastImage;
    m_lastImage = nullptr;
    m_mutex.unlock();
    return image;
}

void RtmpPullThread::run()
{
    run2();
    emit runFinish();
}

void RtmpPullThread::run2()
{
    // Initialize FFmpeg
    avformat_network_init();

    // Open rtmp link
    AVFormatContext* pRtmpFormatCtx = avformat_alloc_context();
    AVDictionary* options = nullptr;
    av_dict_set(&options, "stimeout", "60000000", 0);
    av_dict_set(&options, "rtmp_live", "live", 0);
    int result = avformat_open_input(&pRtmpFormatCtx, m_rtmpPullUrl.toStdString().c_str(), NULL, &options);
    av_dict_free(&options);
    if (result != 0)
    {
        qCritical("failed to open rtmp video, error is %d", result);
        return;
    }

    // Find the video stream information
    result = avformat_find_stream_info(pRtmpFormatCtx, NULL);
    if (result < 0)
    {
        qCritical("failed to find stream, error is %d", result);
        avformat_close_input(&pRtmpFormatCtx);
        return;
    }

    // Find the video stream
    unsigned int videoStream = -1;
    for (unsigned int i = 0; i < pRtmpFormatCtx->nb_streams; i++)
    {
        if (pRtmpFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            break;
        }
    }

    if (videoStream == -1)
    {
        qCritical("failed to find video stream");
        avformat_close_input(&pRtmpFormatCtx);
        return;
    }

    // Allocate the codec context for the video stream
    AVCodecContext* pDecoderCtx = avcodec_alloc_context3(NULL);
    AVCodecParameters* codecPar = pRtmpFormatCtx->streams[videoStream]->codecpar;
    result = avcodec_parameters_to_context(pDecoderCtx, codecPar);
    if (result < 0)
    {
        qCritical("failed to set codec parameters, error is %d", result);
        avcodec_free_context(&pDecoderCtx);
        avformat_close_input(&pRtmpFormatCtx);
        return;
    }

    // Find the codec
    const AVCodec* pCodec = avcodec_find_decoder(pDecoderCtx->codec_id);
    if (!pCodec)
    {
        qCritical("failed to find decoder");
        avcodec_free_context(&pDecoderCtx);
        avformat_close_input(&pRtmpFormatCtx);
        return;
    }

    // Open the codec
    result = avcodec_open2(pDecoderCtx, pCodec, NULL);
    if (result < 0)
    {
        qCritical("failed to open decoder, error is %d", result);
        avcodec_free_context(&pDecoderCtx);
        avformat_close_input(&pRtmpFormatCtx);
        return;
    }    

    // Main loop for capturing and streaming
    AVPacket avPacket;
    while (!m_exit)
    {
        // Read the frame from the camera
        result = av_read_frame(pRtmpFormatCtx, &avPacket);
        if (result < 0)
        {
            if (result == AVERROR_EOF)
            {
                qInfo("rtmp not have more data");
                break;
            }

            qDebug("failed to read frame from rtmp, error is %d", result);
            continue;
        }

        // Decode the frame
        avcodec_send_packet(pDecoderCtx, &avPacket);
        AVFrame* avFrame = av_frame_alloc();
        result = avcodec_receive_frame(pDecoderCtx, avFrame);
        if (result == 0)
        {
            AVFrame* rgbFrame = FfmpegUtil::convertToRGB24Format(avFrame);

            if (m_enableGenerateQImage)
            {
                QImage* image = FfmpegUtil::convertToQImage(rgbFrame);
                m_mutex.lock();
                if (m_lastImage)
                {
                    delete m_lastImage;
                }
                m_lastImage = image;
                m_mutex.unlock();
            }

            if (m_rtmpFrameArriveCallback)
            {
                m_rtmpFrameArriveCallback->onRtmpFrameArrive(rgbFrame);
            }
            else
            {
                av_frame_free(&rgbFrame);
            }
        }
        else
        {
            if (result != AVERROR(EAGAIN))
            {
                qCritical("failed to do the h264 decode, error is %d", result);
                av_packet_unref(&avPacket);
                av_frame_free(&avFrame);
                break;
            }
        }

        av_packet_unref(&avPacket);
        av_frame_free(&avFrame);
    }

    // Clean up
    avcodec_free_context(&pDecoderCtx);
    avformat_close_input(&pRtmpFormatCtx);
}
