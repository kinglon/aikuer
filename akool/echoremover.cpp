#include "echoremover.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#define AUDIO_FRAME_CHANNELS 2
#define AUDIO_FRAME_SAMPLES_PER_SEC 44100


EchoRemover::EchoRemover(QObject *parent)
    : QThread{parent}
{

}

void EchoRemover::run()
{
    qInfo("echo remover begins to run");

    if (m_microphoneName.isEmpty() || m_audioTrackId == INVALID_TRACK_ID)
    {
        qCritical("invalid param");
        return;
    }

    agora::util::AutoPtr<agora::media::IMediaEngine> mediaEngine;
    mediaEngine.queryInterface(m_rtcEngine, AGORA_IID_MEDIA_ENGINE);
    if (mediaEngine.get() == nullptr)
    {
        qCritical("failed to get the media engine interface");
        return;
    }

    // 初始化扬声器输出（Windows waveOut）
    WAVEFORMATEX wfx = { 0 };
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = AUDIO_FRAME_CHANNELS;                // 立体声
    wfx.nSamplesPerSec = AUDIO_FRAME_SAMPLES_PER_SEC;       // 44.1kHz
    wfx.wBitsPerSample = TWO_BYTES_PER_SAMPLE*8;          // 16-bit
    wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    HWAVEOUT hWaveOut;
    MMRESULT mmresult = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, (DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION);
    if (mmresult != MMSYSERR_NOERROR)
    {
        qCritical("failed to call waveOutOpen, error:%d", mmresult);
        return;
    }

    // 初始化FFmpeg    
    avdevice_register_all();
    avformat_network_init();

    // 打开麦克风设备
    AVFormatContext* formatContext = nullptr;
    QString url = QString("audio=%1").arg(m_microphoneName);
    std::string audioDeviceName = url.toStdString().c_str();
    const AVInputFormat* format = av_find_input_format("dshow");
    int result = avformat_open_input(&formatContext, audioDeviceName.c_str(), format, nullptr);
    if (result != 0)
    {
        qCritical("failed to call avformat_open_input, error: %d", result);
        waveOutClose(hWaveOut);
        return;
    }

    // 获取流信息
    result = avformat_find_stream_info(formatContext, nullptr);
    if (result < 0)
    {
        qCritical("failed to call avformat_find_stream_info, error: %d", result);
        avformat_close_input(&formatContext);
        waveOutClose(hWaveOut);
        return;
    }

    // 查找音频流
    int audioStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++)
    {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioStreamIndex = i;
            break;
        }
    }

    if (audioStreamIndex == -1)
    {
        qCritical("failed to find an audio stream");
        avformat_close_input(&formatContext);
        waveOutClose(hWaveOut);
        return;
    }

    // 获取解码器参数
    AVCodecParameters* codecParameters = formatContext->streams[audioStreamIndex]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecParameters->codec_id);
    if (!codec)
    {
        qCritical("failed to find the decoder of %d", codecParameters->codec_id);
        avformat_close_input(&formatContext);
        waveOutClose(hWaveOut);
        return;
    }

    // 创建解码器上下文
    AVCodecContext* codecContext = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codecContext, codecParameters);
    result = avcodec_open2(codecContext, codec, nullptr);
    if (result < 0)
    {
        qCritical("failed to call avcodec_open2, error: %d", result);
        avformat_close_input(&formatContext);
        waveOutClose(hWaveOut);
        return;
    }

    // 初始化重采样器
    SwrContext* swrContext = swr_alloc();
    av_opt_set_int(swrContext, "in_channel_count", codecContext->channels, 0);
    av_opt_set_int(swrContext, "out_channel_count", AUDIO_FRAME_CHANNELS, 0); // 输出为立体声
    av_opt_set_int(swrContext, "in_sample_rate", codecContext->sample_rate, 0);
    av_opt_set_int(swrContext, "out_sample_rate", AUDIO_FRAME_SAMPLES_PER_SEC, 0); // 输出采样率为44.1kHz
    av_opt_set_sample_fmt(swrContext, "in_sample_fmt", codecContext->sample_fmt, 0);
    av_opt_set_sample_fmt(swrContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0); // 输出为16位PCM
    swr_init(swrContext);

    // 读取音频数据
    AVPacket packet;
    AVFrame* frame = av_frame_alloc();
    while (true)
    {
        if (!m_enabled)
        {
            QThread::msleep(100);
            continue;
        }

        result = av_read_frame(formatContext, &packet);
        if (result < 0)
        {
            qCritical("failed to read audio frame, error: %d", result);
            break;
        }

        if (packet.stream_index == audioStreamIndex)
        {
            avcodec_send_packet(codecContext, &packet);
            while (avcodec_receive_frame(codecContext, frame) == 0)
            {
                // 重采样
                uint8_t* outputBuffer = nullptr;
                av_samples_alloc(&outputBuffer, nullptr, 2, frame->nb_samples, AV_SAMPLE_FMT_S16, 0);
                int sampleCount = swr_convert(swrContext, &outputBuffer, frame->nb_samples, (const uint8_t**)frame->data, frame->nb_samples);

                // 填充AudioFrame结构体
                IAudioFrameObserverBase::AudioFrame audioFrame;
                audioFrame.type = IAudioFrameObserverBase::FRAME_TYPE_PCM16;
                audioFrame.samplesPerChannel = sampleCount;
                audioFrame.bytesPerSample = rtc::TWO_BYTES_PER_SAMPLE;
                audioFrame.channels = AUDIO_FRAME_CHANNELS; // 立体声
                audioFrame.samplesPerSec = AUDIO_FRAME_SAMPLES_PER_SEC; // 44.1kHz
                int bufferLength = sampleCount * AUDIO_FRAME_CHANNELS * rtc::TWO_BYTES_PER_SAMPLE; // 每个样本2字节，2个声道
                audioFrame.buffer = malloc(bufferLength);
                memcpy(audioFrame.buffer, outputBuffer, bufferLength);

                // 把声音推送到声网回声识别系统
                int result = mediaEngine.get()->pushAudioFrame(&audioFrame, m_audioTrackId);
                if (result < 0)
                {
                    qDebug("failed to call pushAudioFrame, error: %d", result);
                }
                else
                {
                    qDebug("successful to call pushAudioFrame");
                }
                free(audioFrame.buffer);

                // 通过扬声器播放出来
                WAVEHDR* waveHdr = new WAVEHDR;
                waveHdr->lpData = new char[bufferLength];
                memcpy(waveHdr->lpData, outputBuffer, bufferLength);
                waveHdr->dwBufferLength = bufferLength;
                waveHdr->dwFlags = 0;
                mmresult = waveOutPrepareHeader(hWaveOut, waveHdr, sizeof(WAVEHDR));
                if (mmresult == MMSYSERR_NOERROR)
                {
                    mmresult = waveOutWrite(hWaveOut, waveHdr, sizeof(WAVEHDR));
                }

                if (mmresult != MMSYSERR_NOERROR)
                {
                    qDebug("failed to call waveOutPrepareHeader or waveOutWrite, error: %d", mmresult);
                }

                // 释放资源
                av_freep(&outputBuffer);
            }
        }
        av_packet_unref(&packet);
    }

    // 释放资源
    av_frame_free(&frame);
    swr_free(&swrContext);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    waveOutClose(hWaveOut);

    qInfo("echo remover finish to run");
}

void EchoRemover::waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
{
    (void)dwParam2;
    (void)dwInstance;
    if (uMsg == WOM_DONE)
    {
        LPWAVEHDR pWaveHdr = (LPWAVEHDR)dwParam1;
        waveOutUnprepareHeader(hWaveOut, pWaveHdr, sizeof(WAVEHDR));
        delete[] pWaveHdr->lpData;
        delete pWaveHdr;
    }
}
