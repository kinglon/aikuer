#ifndef FFMPEGUTIL_H
#define FFMPEGUTIL_H

#include <QImage>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

class FfmpegUtil
{
public:
    FfmpegUtil();

public:
    // 解码后的帧数据转成RGB24格式
    static AVFrame* convertToRGB24Format(const AVFrame* frame);

    // frame format is AV_PIX_FMT_RGB24
    static QImage* convertToQImage(const AVFrame *frame);
};

#endif // FFMPEGUTIL_H
