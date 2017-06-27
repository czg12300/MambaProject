//
// Created by yongfali on 2016/4/7.
//

#ifndef E_FACETRACKER_H
#define E_FACETRACKER_H
#include "GPUImageCommon.h"
#include "GPUImageCallback.h"

namespace e
{
    struct ImageSample{
        void* data;
        int size;
        int width;
        int height;
        int bitCount;
        int format;
    };

    struct TrackResult{
        int sender;             //0-hivideo or 1-sensetime
        int located;            //本次是否定位到人脸,0-没检测到，1-检测到
        float x1;               //人脸的区域
        float y1;               
        float x2;
        float y2;
        float face_width;       //人脸的宽，没有归一化
        float face_height;      //人脸的高，没有归一化
        float eye_dist;         //两眼之间的距离，没有归一化
        float angle_x;
        float angle_y;
        float angle_z;         //倾斜度
        float points[212];     //8个特征点，依次为：x0 y0 x1 y1.....
    };

    
    class IFaceTracker : public RefObject
    {
    public:
        IFaceTracker(void);
        virtual ~IFaceTracker(void);
    public:
        void SetDeviceRotation(int rotation);
        void SetTrackCallback(GPUImageCallback callback);
        bool IsAvailable(void) const;
        int OnSampleProc(void* data);

        float MapX(float x);
        float MapY(float y);
    protected:
        virtual int SampleProc(void* data) = 0;
        virtual int ResetResult(TrackResult* trackResult) = 0;
    protected:
        Size _inputSize;
        int _deviceRotation;
        uint64_t _lastTime;
        bool _isAvailable;
        bool _isFaceLocated;
        TrackResult _trackResult;
        GPUImageCallback _trackCallback;
    };
}

#endif //_E_FACETRACKER_H_
