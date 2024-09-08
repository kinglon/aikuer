#include "camerareadthread.h"
#include "ffmpegutil.h"
#include <Windows.h>

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

// Custom log callback function
void custom_log_callback(void* ptr, int level, const char* fmt, va_list vl)
{
    // Define the maximum log message length
    constexpr int MAX_LOG_LENGTH = 1024;
    char log_message[MAX_LOG_LENGTH];

    // Format the log message
    static int print_prefix = 1;
    av_log_format_line(ptr, level, fmt, vl, log_message, MAX_LOG_LENGTH, &print_prefix);
    qDebug(log_message);
}

CameraReadThread::CameraReadThread()
{

}

CameraReadThread::~CameraReadThread()
{
    if (m_lastImage)
    {
        delete m_lastImage;
    }
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
    // av_log_set_level(AV_LOG_DEBUG); // Set the log level
    // av_log_set_callback(custom_log_callback); // Set the custom log callback function

    // Open the camera
    AVFormatContext* pCameraFormatCtx = avformat_alloc_context();
    AVDictionary* options = nullptr;
    // Set encoder options for zero latency
    av_dict_set(&options, "preset", "ultrafast", 0);
    av_dict_set(&options, "tune", "zerolatency", 0);
    av_dict_set(&options, "crf", "23", 0);
    int result = avformat_open_input(&pCameraFormatCtx, nullptr, m_camera, &options);
    av_dict_free(&options);
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
    m_cameraSize.setWidth(codecPar->width);
    m_cameraSize.setHeight(codecPar->height);
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
    m_format = pDecoderCtx->pix_fmt;

    // Main loop for capturing and streaming
    AVPacket avPacket;
    int64_t lastTime = 0;
    const int durationPerTime = 1000 / m_frameCount;
    while (!m_exit)
    {
        int64_t elapse = GetTickCount64() - lastTime;
        if (elapse < durationPerTime)
        {
            QThread::msleep(durationPerTime - elapse);
        }
        lastTime = GetTickCount64();

        // Read the frame from the camera
        result = av_read_frame(pCameraFormatCtx, &avPacket);
        if (result < 0)
        {
            qCritical("failed to read frame from camera");
            break;
        }

        // Decode the frame
        avcodec_send_packet(pDecoderCtx, &avPacket);
        AVFrame* avFrame = av_frame_alloc();
        result = avcodec_receive_frame(pDecoderCtx, avFrame);
        if (result < 0)
        {
            qCritical("failed to decode frame from camera");
            av_packet_unref(&avPacket);
            av_frame_free(&avFrame);
            continue;
        }        

        if (m_enableGenerateQImage)
        {
            AVFrame* rgbFrame = FfmpegUtil::convertToRGB24Format(avFrame);
            QImage* image = FfmpegUtil::convertToQImage(rgbFrame);
            av_frame_free(&rgbFrame);

            m_mutex.lock();
            if (m_lastImage)
            {
                delete m_lastImage;
            }
            m_lastImage = image;
            m_mutex.unlock();
        }

        if (m_frameArriveCallback)
        {
            m_frameArriveCallback->onFrameArrive(avFrame);
        }
        else
        {
            av_frame_free(&avFrame);
        }

        // Free the packet
        av_packet_unref(&avPacket);        
    }

    // Clean up    
    avcodec_free_context(&pDecoderCtx);
    avformat_close_input(&pCameraFormatCtx);    
}

QImage* CameraReadThread::popImage()
{
    QImage* image = nullptr;
    m_mutex.lock();
    image = m_lastImage;
    m_lastImage = nullptr;
    m_mutex.unlock();
    return image;
}
