#include "ffmpegutil.h"

FfmpegUtil::FfmpegUtil()
{

}

AVFrame* FfmpegUtil::convertToRGB24Format(const AVFrame* frame)
{
    // Allocate the new AVFrame with RGB format
    AVFrame *rgb_frame = av_frame_alloc();

    // Set the parameters for the new frame
    rgb_frame->format = AV_PIX_FMT_RGB24;
    rgb_frame->width = frame->width;
    rgb_frame->height = frame->height;

    // Allocate the frame data
    av_frame_get_buffer(rgb_frame, 0);

    // Create SwsContext for the conversion
    struct SwsContext *sws_ctx = sws_getContext(
        frame->width, frame->height, (AVPixelFormat)frame->format,
        rgb_frame->width, rgb_frame->height, (AVPixelFormat)rgb_frame->format,
        SWS_BILINEAR, NULL, NULL, NULL
    );

    // Perform the color conversion
    sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height,
              rgb_frame->data, rgb_frame->linesize);

    // Free the SwsContext
    sws_freeContext(sws_ctx);

    return rgb_frame;
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
