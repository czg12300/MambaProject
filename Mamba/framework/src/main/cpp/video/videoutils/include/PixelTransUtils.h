//
// Created by jakechen on 2017/6/30.
//

#ifndef MAMBA_PIXELTRANSUTILS_H
#define MAMBA_PIXELTRANSUTILS_H

namespace video {
    void rgbaToYuv(unsigned char *rgb, int width, int height, unsigned char *yuv);
}

#endif //MAMBA_PIXELTRANSUTILS_H
