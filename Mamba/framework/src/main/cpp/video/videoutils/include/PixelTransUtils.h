//
// Created by jakechen on 2017/6/30.
//

#ifndef MAMBA_PIXELTRANSUTILS_H
#define MAMBA_PIXELTRANSUTILS_H
extern "C" {
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>

#include "libyuv/rotate.h"
#include "libyuv/convert.h"
#include "libyuv/scale.h"
};

#include <Log.h>

using namespace libyuv;
namespace video {
    void rgbaToYuv(unsigned char *rgb, int width, int height, unsigned char *yuv);

    void
    yuv420spToYuv420p(unsigned char *yuv, int width, int height, unsigned char *yuv420p,
                      int dest_width,
                      int dest_height);

    void
    nv21ToYv12(unsigned char *yuv, int width, int height, unsigned char *yuv420p,
               int dest_width,
               int dest_height, int rotate);
    void
    nv21ToI420(unsigned char *yuv, int width, int height, unsigned char *yuv420p,
               int dest_width,
               int dest_height, int rotate);

}

#endif //MAMBA_PIXELTRANSUTILS_H
