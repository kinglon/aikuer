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
    // 获取H264编码器
    AVCodecContext* h264CodecCtx = getH264Codec();
    if (h264CodecCtx == nullptr)
    {
        return;
    }

    // 获取AAC编码器
    AVCodecContext* aacCodecCtx = getAacCodec();
    if (aacCodecCtx == nullptr)
    {
        avcodec_free_context(&h264CodecCtx);
        return;
    }

    AVFormatContext* pRtmpFormatCtx = nullptr;
    int result = avformat_alloc_output_context2(&pRtmpFormatCtx, nullptr, "flv", m_rtmpPushUrl.toStdString().c_str());
    if (result < 0)
    {
        qCritical("failed to create flv RTMP context, error is %d", result);
        avcodec_free_context(&h264CodecCtx);
        avcodec_free_context(&aacCodecCtx);
        return;
    }

    // Add video stream to the RTMP context
    AVStream* pRtmpVideoStream = avformat_new_stream(pRtmpFormatCtx, nullptr);
    if (!pRtmpVideoStream)
    {
        qCritical("failed to create RTMP video stream");
        avcodec_free_context(&h264CodecCtx);
        avcodec_free_context(&aacCodecCtx);
        avformat_free_context(pRtmpFormatCtx);
        return;
    }
    avcodec_parameters_from_context(pRtmpVideoStream->codecpar, h264CodecCtx);

    // Add audio stream to the RTMP context
    AVStream* pRtmpAudioStream = avformat_new_stream(pRtmpFormatCtx, nullptr);
    if (!pRtmpAudioStream)
    {
        qCritical("failed to create RTMP audio stream");
        avcodec_free_context(&h264CodecCtx);
        avcodec_free_context(&aacCodecCtx);
        avformat_free_context(pRtmpFormatCtx);
        return;
    }
    avcodec_parameters_from_context(pRtmpAudioStream->codecpar, aacCodecCtx);

    // Open the RTMP output
    result = avio_open(&pRtmpFormatCtx->pb, m_rtmpPushUrl.toStdString().c_str(), AVIO_FLAG_WRITE);
    if (result < 0)
    {
        qCritical("failed to open RTMP output, error is %d", result);
        avcodec_free_context(&h264CodecCtx);
        avcodec_free_context(&aacCodecCtx);
        avformat_free_context(pRtmpFormatCtx);
        return;
    }

    // Write the header to the RTMP output
    result = avformat_write_header(pRtmpFormatCtx, NULL);
    if (result < 0)
    {
        qCritical("failed to write header to RTMP output, error is %d", result);
        avcodec_free_context(&h264CodecCtx);
        avcodec_free_context(&aacCodecCtx);
        avformat_free_context(pRtmpFormatCtx);
        return;
    }

    // 推流
    pushStream(pRtmpFormatCtx, h264CodecCtx, aacCodecCtx, pRtmpVideoStream, pRtmpAudioStream);

    avcodec_free_context(&h264CodecCtx);
    avcodec_free_context(&aacCodecCtx);
    avformat_free_context(pRtmpFormatCtx);
}

AVCodecContext* RtmpPushThread::getH264Codec()
{
    // 查找H264编码器
    const AVCodec *h264Codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!h264Codec)
    {
        qCritical("failed to find h264 encoder");
        return nullptr;
    }

    // 分配编码器上下文
    AVCodecContext *h264CodecCtx = avcodec_alloc_context3(h264Codec);
    if (!h264CodecCtx)
    {
        qCritical("failed to create h264 encoder context");
        return nullptr;
    }

    // 设置编码器参数
    h264CodecCtx->codec_id = AV_CODEC_ID_H264;
    h264CodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    h264CodecCtx->bit_rate = 3000 * 1024; // 3000k
    h264CodecCtx->width = m_width;
    h264CodecCtx->height = m_height;
    h264CodecCtx->time_base = {1, 30};
    h264CodecCtx->framerate = {30, 1};
    h264CodecCtx->gop_size = 10;
    h264CodecCtx->pix_fmt = m_format;

    // 打开编码器
    int result = avcodec_open2(h264CodecCtx, h264Codec, NULL);
    if (result < 0)
    {
        qCritical("failed to open h264 encoder, error is %d", result);
        avcodec_free_context(&h264CodecCtx);
        return nullptr;
    }

    return h264CodecCtx;
}

AVCodecContext* RtmpPushThread::getAacCodec()
{
    // 查找Aac编码器
    const AVCodec *aacCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!aacCodec)
    {
        qCritical("failed to find aac encoder");
        return nullptr;
    }

    // 分配编码器上下文
    AVCodecContext *aacCodecCtx = avcodec_alloc_context3(aacCodec);
    if (!aacCodecCtx)
    {
        qCritical("failed to create aac encoder context");
        return nullptr;
    }

    // 设置编码器参数
    aacCodecCtx->codec_id = AV_CODEC_ID_AAC;
    aacCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
    aacCodecCtx->bit_rate = 160 * 1024; // 160k
    aacCodecCtx->sample_rate  = 44100;
    aacCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
    aacCodecCtx->ch_layout.order = AV_CHANNEL_ORDER_NATIVE;
    aacCodecCtx->ch_layout.nb_channels = 1;
    aacCodecCtx->ch_layout.u.mask = AV_CH_LAYOUT_MONO;
    aacCodecCtx->time_base = {1, aacCodecCtx->sample_rate};

    // 打开编码器
    int result = avcodec_open2(aacCodecCtx, aacCodec, NULL);
    if (result < 0)
    {
        qCritical("failed to open aac encoder, error is %d", result);
        avcodec_free_context(&aacCodecCtx);
        return nullptr;
    }

    return aacCodecCtx;
}

void RtmpPushThread::pushStream(AVFormatContext* rtmpFormatCtx,
                                AVCodecContext* h264CodecCtx, AVCodecContext* aacCodecCtx,
                                AVStream* videoStream, AVStream* audioStream)
{
    // Prepare frames
    AVPacket videoPacket, audioPacket;
    av_init_packet(&videoPacket);
    av_init_packet(&audioPacket);
    videoPacket.data = nullptr;
    audioPacket.data = nullptr;

    int videoFrameCount = 0; // For video pts
    int audioFrameCount = 0; // For audio pts

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

        // 获取一帧
        AVFrame* srcFrame = popFrame();
        if (srcFrame == nullptr)
        {
            QThread::msleep(5);
            continue;
        }

        // 转成推送到服务器需要的帧格式
        AVFrame* videoFrame = handleFrame(srcFrame, h264CodecCtx);
        av_frame_free(&srcFrame);

        // Encode Video Frame
        videoFrame->pts = videoFrameCount++;
        int response = avcodec_send_frame(h264CodecCtx, videoFrame);
        if (response < 0)
        {
            qDebug("failed to call avcodec_send_frame, error is %d", response);
            av_frame_free(&videoFrame);
            continue;
        }

        while (response >= 0)
        {
            response = avcodec_receive_packet(h264CodecCtx, &videoPacket);
            if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
            {
                break;
            }
            else if (response < 0)
            {
                qDebug("failed to call avcodec_receive_packet, error is %d", response);
                break;
            }
            videoPacket.stream_index = videoStream->index;
            av_interleaved_write_frame(rtmpFormatCtx, &videoPacket);
            av_packet_unref(&videoPacket);
        }

        av_frame_free(&videoFrame);

        // Simulate capturing audio
        AVFrame *audioFrame = av_frame_alloc();
        audioFrame->format = aacCodecCtx->sample_fmt;
        audioFrame->ch_layout = aacCodecCtx->ch_layout;
        audioFrame->nb_samples = aacCodecCtx->frame_size;
        av_frame_get_buffer(audioFrame, 0);

        // Fill audio frame
        for (int i = 0; i < audioFrame->nb_samples; i++)
        {
            ((float*)audioFrame->data[0])[i] = 0.5f;
        }

        // Encode Audio Frame
        audioFrame->pts = audioFrameCount++;
        response = avcodec_send_frame(aacCodecCtx, audioFrame);
        if (response < 0)
        {
            qDebug("failed to call avcodec_send_frame, error is %d", response);
            av_frame_free(&audioFrame);
            continue;
        }

        while (response >= 0)
        {
            response = avcodec_receive_packet(aacCodecCtx, &audioPacket);
            if (response == AVERROR(EAGAIN) || response == AVERROR_EOF)
            {
                break;
            }
            else if (response < 0)
            {
                qDebug("failed to call avcodec_receive_packet, error is %d", response);
                break;
            }
            audioPacket.stream_index = audioStream->index;
            av_interleaved_write_frame(rtmpFormatCtx, &audioPacket);
            av_packet_unref(&audioPacket);
            frameCount++;
        }

        av_frame_free(&audioFrame);
    }
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

AVFrame* RtmpPushThread::handleFrame(const AVFrame* srcFrame, AVCodecContext* codecCtx)
{
    // Allocate the new AVFrame
    AVFrame *destframe = av_frame_alloc();

    // Set the parameters for the new frame
    destframe->format = codecCtx->pix_fmt;
    destframe->width = codecCtx->width;
    destframe->height = codecCtx->height;

    // Allocate the frame data
    av_frame_get_buffer(destframe, 0);

    // Create SwsContext for the conversion
    struct SwsContext *sws_ctx = sws_getContext(
        srcFrame->width, srcFrame->height, (AVPixelFormat)srcFrame->format,
        destframe->width, destframe->height, (AVPixelFormat)destframe->format,
        SWS_BILINEAR, NULL, NULL, NULL
    );

    // Perform the color conversion
    sws_scale(sws_ctx, srcFrame->data, srcFrame->linesize, 0, srcFrame->height,
              destframe->data, destframe->linesize);

    // Free the SwsContext
    sws_freeContext(sws_ctx);

    return destframe;
}
