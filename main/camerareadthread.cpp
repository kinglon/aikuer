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
    AVCodecContext *h264CodecCtx = nullptr;
    bool enablePushRtmp = !m_rtmpPushUrl.isEmpty();
    if (enablePushRtmp)
    {
        // 查找H264编码器
        const AVCodec *h264Codec = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (!h264Codec)
        {
            qCritical("failed to find h264 encoder");
            avcodec_free_context(&pDecoderCtx);
            avformat_close_input(&pCameraFormatCtx);
            return;
        }

        // 分配编码器上下文
        h264CodecCtx = avcodec_alloc_context3(h264Codec);
        if (!h264CodecCtx)
        {
            qCritical("failed to create h264 encoder context");
            avcodec_free_context(&pDecoderCtx);
            avformat_close_input(&pCameraFormatCtx);
            return;
        }

        // 设置编码器参数
        h264CodecCtx->codec_id = AV_CODEC_ID_H264;
        h264CodecCtx->bit_rate = 4000000;
        h264CodecCtx->width = codecPar->width;
        h264CodecCtx->height = codecPar->height;
        h264CodecCtx->time_base = {1, 30};
        h264CodecCtx->framerate = {30, 1};
        h264CodecCtx->gop_size = 10;
        h264CodecCtx->max_b_frames = 1;
        h264CodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

        // 打开编码器
        result = avcodec_open2(h264CodecCtx, h264Codec, NULL);
        if (result < 0)
        {
            qCritical("failed to open h264 encoder, error is %d", result);
            avcodec_free_context(&pDecoderCtx);
            avformat_close_input(&pCameraFormatCtx);
            avcodec_free_context(&h264CodecCtx);
            return;
        }

        result = avformat_alloc_output_context2(&pRtmpFormatCtx, nullptr, "flv", m_rtmpPushUrl.toStdString().c_str());
        if (result < 0)
        {
            qCritical("failed to create flv RTMP context, error is %d", result);
            avcodec_free_context(&pDecoderCtx);
            avformat_close_input(&pCameraFormatCtx);
            avcodec_free_context(&h264CodecCtx);
            return;
        }

        // Add video stream to the RTMP context
        AVStream* pRtmpVideoStream = avformat_new_stream(pRtmpFormatCtx, h264Codec);
        if (!pRtmpVideoStream)
        {
            qCritical("failed to create RTMP video stream");
            avcodec_free_context(&pDecoderCtx);
            avformat_close_input(&pCameraFormatCtx);
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
            avcodec_free_context(&pDecoderCtx);
            avformat_close_input(&pCameraFormatCtx);
            avcodec_free_context(&h264CodecCtx);
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
            avcodec_free_context(&h264CodecCtx);
            avformat_free_context(pRtmpFormatCtx);
            return;
        }
    }

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
            break;
        }

        if (m_enableGenerateQImage)
        {
            QImage* image = FfmpegUtil::convertToQImage(avFrame);
            m_mutex.lock();
            if (m_lastImage)
            {
                delete m_lastImage;
            }
            m_lastImage = image;
            m_mutex.unlock();
        }

        if (enablePushRtmp)
        {
            // h264 encode
            avcodec_send_frame(h264CodecCtx, avFrame);
            result = avcodec_receive_packet(h264CodecCtx, &avPacket);
            if (result >= 0)
            {
                av_interleaved_write_frame(pRtmpFormatCtx, &avPacket);
            }
            else
            {
                if (result != AVERROR(EAGAIN))
                {
                    qCritical("failed to do the h264 encode, error is %d", result);
                    av_packet_unref(&avPacket);
                    av_frame_free(&avFrame);
                    break;
                }
            }
        }

        // Free the packet
        av_packet_unref(&avPacket);
        av_frame_free(&avFrame);
    }

    // Clean up    
    avcodec_free_context(&pDecoderCtx);
    avformat_close_input(&pCameraFormatCtx);
    if (enablePushRtmp)
    {
        avcodec_free_context(&h264CodecCtx);
        avformat_free_context(pRtmpFormatCtx);
    }
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
