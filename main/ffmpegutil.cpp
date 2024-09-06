#include "ffmpegutil.h"

FfmpegUtil::FfmpegUtil()
{

}

QImage* FfmpegUtil::convertToQImage(const AVFrame *frame)
{
    QImage* image = new QImage(frame->width, frame->height, QImage::Format_RGB32);
    for (int y = 0; y < frame->height; y++)
    {
        for (int x = 0; x < frame->width; x++)
        {
            uint8_t *ptr = image->scanLine(y) + x * 4;
            ptr[0] = frame->data[0][y * frame->linesize[0] + x * 3];
            ptr[1] = frame->data[0][y * frame->linesize[0] + x * 3 + 1];
            ptr[2] = frame->data[0][y * frame->linesize[0] + x * 3 + 2];
            ptr[3] = 255;
        }
    }

    return image;
}
