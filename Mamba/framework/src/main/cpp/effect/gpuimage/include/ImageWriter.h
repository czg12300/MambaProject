//
// Created by liyongfa on 2017/3/18.
//

#ifndef KUGOUEFFECT_IMAGEWRITER_H
#define KUGOUEFFECT_IMAGEWRITER_H

#include "GPUImageBase.h"
#include "GPUImageCallback.h"
#include "GPUImageWriteFilter.h"

namespace e
{
    class ImageWriter: public Object
    {
    public:
        ImageWriter();
        ~ImageWriter();
    public:
        void SetEnable(bool enable);
        bool IsEnable(void) const;
        int OnSampleProc(void* data);
    private:
        bool _enableWrite;
    };
}

#endif //KUGOUEFFECT_GPUIMAGEIMAGEWRITER6_H
