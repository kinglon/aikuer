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
    static QImage* convertToQImage(const AVFrame *frame);
};

#endif // FFMPEGUTIL_H
