//
// Created by yongfali on 2016/4/7.
//

#include "IFaceTracker.h"
#include "GPUImageCommon.h"
#include "GPUImageAnimateFilter.h"
#include <stdlib.h>

namespace e
{
    IFaceTracker::IFaceTracker(void)
    {
        _inputSize = kSmallImageSize;
        _lastTime = 0;
        _deviceRotation = 0;
        _isAvailable = false;
        _isFaceLocated = false;

        memset(&_trackResult, 0, sizeof(_trackResult));
    }

    IFaceTracker::~IFaceTracker(void)
    {

    }
    
    float IFaceTracker::MapX(float x)
    {
        return 2 * x / _inputSize.width - 1.0f;
    }

    float IFaceTracker::MapY(float y)
    {
        return 1.0f - 2 * y / _inputSize.height;
    }    

    bool IFaceTracker::IsAvailable(void) const
    {
        return _isAvailable;
    }

    void IFaceTracker::SetDeviceRotation(int rotation)
    {
        _deviceRotation = rotation;
    }
    
    void IFaceTracker::SetTrackCallback(GPUImageCallback callback)
    {
        _trackCallback = callback;
    }
    
    int IFaceTracker::OnSampleProc(void* data)
    {
        return SampleProc(data);
    }
}
