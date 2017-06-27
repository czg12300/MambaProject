//
// Created by liyongfa on 2017/3/18.
//

#ifndef KUGOUEFFECT_VIDEOTRACKER_H
#define KUGOUEFFECT_VIDEOTRACKER_H

#include "GPUImageBase.h"
#include "GPUImageCallback.h"
#include "IFaceTracker.h"

namespace e
{
    class VideoTracker: public GPUImageCallback
    {
    public:
        VideoTracker();
        ~VideoTracker();
    public:
        void SetEnable(bool enable);
        bool IsEnable(void) const;
        int OnSampleProc(void* data);
    private:
        bool _enable;
    };
}


#endif //KUGOUEFFECT_VIDEOTRACKER_H
