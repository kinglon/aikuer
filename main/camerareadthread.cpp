#include "camerareadthread.h"
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

CameraReadThread::CameraReadThread()
{

}

void CameraReadThread::run()
{
    run2();
    emit runFinish();
}

void CameraReadThread::run2()
{
    // Initialize FFmpeg
    avformat_network_init();

    // Open the camera
    AVFormatContext* pCameraFormatCtx = avformat_alloc_context();
    int result = avformat_open_input(&pCameraFormatCtx, nullptr, m_camera, NULL);
    if (result != 0)
    {
        qCritical("failed to open camera, error is %d", result);
        return;
    }

    // Find the video stream information
    result = avformat_find_stream_info(pCameraFormatCtx, NULL);
    if (result < 0)
    {
        qCritical("failed to find stream, error is %d", result);
        avformat_close_input(&pCameraFormatCtx);
        return;
    }

    // Find the video stream
    unsigned int videoStream = -1;
    for (unsigned int i = 0; i < pCameraFormatCtx->nb_streams; i++)
    {
        if (pCameraFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            break;
        }
    }

    if (videoStream == -1)
    {
        qCritical("failed to find video stream");
        avformat_close_input(&pCameraFormatCtx);
        return;
    }

    // Allocate the codec context for the video stream
    AVCodecContext* pDecoderCtx = avcodec_alloc_context3(NULL);
    AVCodecParameters* codecPar = pCameraFormatCtx->streams[videoStream]->codecpar;
    result = avcodec_parameters_to_context(pDecoderCtx, codecPar);
    if (result < 0)
    {
        qCritical("failed to set codec parameters, error is %d", result);
        avcodec_free_context(&pDecoderCtx);
        avformat_close_input(&pCameraFormatCtx);
        return;
    }

    // Find the codec
    const AVCodec* pCodec = avcodec_find_decoder(pDecoderCtx->codec_id);
    if (!pCodec)
    {
        qCritical("failed to find decoder");
        avcodec_free_context(&pDecoderCtx);
        avformat_close_input(&pCameraFormatCtx);
        return;
    }

    // Open the codec
    result = avcodec_open2(pDecoderCtx, pCodec, NULL);
    if (result < 0)
    {
        qCritical("failed to open decoder, error is %d", result);
        avcodec_free_context(&pDecoderCtx);
        avformat_close_input(&pCameraFormatCtx);
        return;
    }

    AVFormatContext* pRtmpFormatCtx = nullptr;
    bool enablePushRtmp = !m_rtmpPushUrl.isEmpty();
    if (enablePushRtmp)
    {
        result = avformat_alloc_output_context2(&pRtmpFormatCtx, nullptr, "flv", m_rtmpPushUrl.toStdString().c_str());
        if (result < 0)
        {
            qCritical("failed to create flv RTMP context, error is %d", result);
            avcodec_free_context(&pDecoderCtx);
            avformat_close_input(&pCameraFormatCtx);
            return;
        }

        // Add video stream to the RTMP context
        AVStream* pRtmpVideoStream = avformat_new_stream(pRtmpFormatCtx, NULL);
        if (!pRtmpVideoStream)
        {
            qCritical("failed to create RTMP video stream");
            avcodec_free_context(&pDecoderCtx);
            avformat_close_input(&pCameraFormatCtx);
            avformat_free_context(pRtmpFormatCtx);
            return;
        }

        // Copy the codec parameters to the RTMP stream
        result = avcodec_parameters_copy(pRtmpVideoStream->codecpar, codecPar);
        if (result < 0)
        {
            qCritical("failed to copy codec parameters to RTMP stream, error is %d", result);
            avcodec_free_context(&pDecoderCtx);
            avformat_close_input(&pCameraFormatCtx);
            avformat_free_context(pRtmpFormatCtx);
            return;
        }

        // Open the RTMP output
        result = avio_open(&pRtmpFormatCtx->pb, m_rtmpPushUrl.toStdString().c_str(), AVIO_FLAG_WRITE);
        if (result < 0)
        {
            qCritical("failed to open RTMP output, error is %d", result);
            avcodec_free_context(&pDecoderCtx);
            avformat_close_input(&pCameraFormatCtx);
            avformat_free_context(pRtmpFormatCtx);
            return;
        }

        // Write the header to the RTMP output
        result = avformat_write_header(pRtmpFormatCtx, NULL);
        if (result < 0)
        {
            qCritical("failed to write header to RTMP output, error is %d", result);
            avcodec_free_context(&pDecoderCtx);
            avformat_close_input(&pCameraFormatCtx);
            avio_close(pRtmpFormatCtx->pb);
            avformat_free_context(pRtmpFormatCtx);
            return;
        }
    }

    // Allocate the frame and packet
    AVFrame* pFrame = av_frame_alloc();
    AVPacket* pPacket = av_packet_alloc();

    // Main loop for capturing and streaming
    bool needPushRtmp = true;
    while (!m_exit)
    {
        // Read the frame from the camera
        result = av_read_frame(pCameraFormatCtx, pPacket);
        if (result < 0)
        {
            qCritical("failed to read frame from camera");
            break;
        }

        // Decode the frame
        avcodec_send_packet(pDecoderCtx, pPacket);
        result = avcodec_receive_frame(pDecoderCtx, pFrame);
        if (result == 0)
        {
            // Encode and send the frame to the RTMP server
            if (needPushRtmp && pRtmpFormatCtx && av_interleaved_write_frame(pRtmpFormatCtx, pPacket) < 0) {
                qCritical("failed to write frame to RTMP output");
                needPushRtmp = false;
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

        // Sleep to control the frame rate
        QThread::msleep(1000/m_frameCount);
    }

    // Clean up
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    avcodec_free_context(&pDecoderCtx);
    avformat_close_input(&pCameraFormatCtx);
    if (enablePushRtmp)
    {
        avio_close(pRtmpFormatCtx->pb);
        avformat_free_context(pRtmpFormatCtx);
    }
}
