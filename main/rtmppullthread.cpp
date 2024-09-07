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
    int result = avformat_open_input(&pRtmpFormatCtx, m_rtmpPullUrl.toStdString().c_str(), NULL, NULL);
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

    // Allocate the frame and packet
    AVFrame* pFrame = av_frame_alloc();
    AVPacket* pPacket = av_packet_alloc();

    // Main loop for capturing and streaming   
    while (!m_exit)
    {
        // Read the frame from the camera
        result = av_read_frame(pRtmpFormatCtx, pPacket);
        if (result < 0)
        {
            qCritical("failed to read frame from rtmp");
            break;
        }

        // Decode the frame
        avcodec_send_packet(pDecoderCtx, pPacket);
        result = avcodec_receive_frame(pDecoderCtx, pFrame);
        if (result == 0)
        {
            if (m_rtmpFrameArriveCallback)
            {
                m_rtmpFrameArriveCallback->onRtmpFrameArrive(pFrame);
            }

            if (m_enableImageArriveSignal)
            {
                emit imageArrive(FfmpegUtil::convertToQImage(pFrame));
            }

            // Free the frame
            av_frame_unref(pFrame);
        }

        // Free the packet
        av_packet_unref(pPacket);
    }

    // Clean up
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    avcodec_free_context(&pDecoderCtx);
    avformat_close_input(&pRtmpFormatCtx);
}
