#include "CVFaceTracker.h"
#include "GPUImageBase.h"
#include "GPUImageAnimateFilter.h"

namespace e 
{
    CVFaceTracker::CVFaceTracker(void)
    {
        char modelPath[1024] = {0};
        sprintf(modelPath, "%strack.tar", GPUImageEnvironment::GetPath(KG_PATH_DATA).c_str());

        st_result_t res = st_mobile_tracker_106_create(modelPath
            , ST_MOBILE_TRACKING_DEFAULT_CONFIG
            , &_trackHandle);

        if (!_trackHandle){
            LOGE("create sensetime face track failed! %d", res);
        }else{
            LOGD("create sensetime face track ok.");
        }

        if (_trackHandle){
            res = st_mobile_tracker_106_reset(_trackHandle);
            if (res==ST_OK){
                LOGD("reset sensetime face track ok.");
            }else{
                LOGE("reset sensetime face track failed!");
            }
        }

        //当跟踪不到人脸的时候才进行检测
        if (res==ST_OK && _trackHandle!=NULL){
            res = st_mobile_tracker_106_set_facelimit(_trackHandle, 1);
            if (res != ST_OK){
                LOGE("limit sensetime face track failed!");
            }else{
                LOGD("limit sensetime face track ok.");
            }
        }

        //设置，每隔20帧检测一次人脸
        if (res==ST_OK && _trackHandle!=NULL){
            res = st_mobile_tracker_106_set_detect_interval(_trackHandle, 20);
            if (res != ST_OK){
                LOGE("set sensetime face interval failed!");
            }else{
                LOGD("set sensetime face interval ok.");
            }
        }

        _isAvailable = _trackHandle?true:false;

        ResetResult(&_trackResult);
    }

    CVFaceTracker::~CVFaceTracker(void)
    {
        if (_trackHandle){
            st_mobile_tracker_106_destroy(_trackHandle);
            _trackHandle = NULL;
        }
    }

    int CVFaceTracker::ResetResult(TrackResult* trackResult)
    {
        memset(trackResult, 0, sizeof(TrackResult));
        trackResult->sender = 1;
        return 0;
    }
   
    int CVFaceTracker::SampleProc(void* _data)
    {
        assert(_trackHandle);
        ResetResult(&_trackResult);

        if (!_isAvailable){
             if (_trackCallback){
                 _trackCallback(&_trackResult);
             } 
             LOGE("cv face track inavailable!!!");
             return -1;  
        }
#ifdef _TIME_LIMIT
        if (!_isFaceLocated){
            uint64_t time = Time::GetTime();
            if (time - _lastTime < 250){
                if (_trackCallback){
                    _trackCallback(&_trackResult);
                }
                LOGD("cv face track time limit!");
                return 0;
            } 
            _lastTime = time;
        }
#endif
        ImageSample* p = static_cast<ImageSample*>(_data);
        void* data = p->data;
        //int size = p->size;
        int width = p->width;
        int height = p->height;
        int bitCount = p->bitCount;

        int faceCount = 0;
        st_mobile_106_t* faces = NULL;
        st_result_t res = st_mobile_tracker_106_track(_trackHandle
            , (const unsigned char*)data
            , ST_PIX_FMT_BGRA8888
            , width
            , height
            , WidthBytes(width*bitCount)
            , ST_CLOCKWISE_ROTATE_0
            , &faces
            , &faceCount);

        if (res != ST_OK){
            LOGD("cv face track error : %d", res);
        }

        _isFaceLocated = false;
        st_mobile_106_t *best = NULL, *face = NULL;

        if (faceCount > 0) best = &faces[0];
        int w1 = best ? abs(best->rect.right - best->rect.left) : 0;
        int h1 = best ? abs(best->rect.bottom - best->rect.top) : 0;
       
        for (int i=1; i<faceCount; i++)
        {
            face = &faces[i];
            int w2 = abs(face->rect.right - face->rect.left);
            int h2 = abs(face->rect.bottom - face->rect.top);
            if (w2 * h2 > w1 * h1) {
                best = face;
                w1 = w2;
                h1 = h2;
            }
        }

        if (best != NULL)
        {
            face = best;
            _isFaceLocated = true;
            _trackResult.located = 1;
            _trackResult.x1 = MapX(face->rect.left);
            _trackResult.y1 = MapY(face->rect.top);
            _trackResult.x2 = MapX(face->rect.right);
            _trackResult.y2 = MapY(face->rect.bottom);
            _trackResult.face_width = abs(face->rect.right - face->rect.left) + 1;
            _trackResult.face_height = abs(face->rect.bottom - face->rect.top) + 1;
            _trackResult.eye_dist = face->eye_dist;//两个眼中心间距离
            _trackResult.angle_x = face->pitch;
            _trackResult.angle_y = face->yaw;
            //_trackResult.angle_z = face->roll;
            
            //适配人脸的标志点
            AdapterPoints(face);
        }
        
        //将人脸识别结果传给效果合成模块
        if (_trackCallback) {
            _trackCallback(&_trackResult);
        }
        
        if (faces){
            st_mobile_tracker_106_release_result(faces, faceCount);
        }
        
        return 1;
    }
    
    void CVFaceTracker::AdapterPoints(const st_mobile_106_t* face)
    {
        for (int i=0; i<106; i++)
        {
            _trackResult.points[i*2+0] = MapX(face->points_array[i].x);
            _trackResult.points[i*2+1] = MapY(face->points_array[i].y);
        }

        //左眼中&右眼的中心点
        float x1 = face->points_array[74].x;
        float y1 = face->points_array[74].y;
        float x2 = face->points_array[77].x;
        float y2 = face->points_array[77].y;
        
        _trackResult.angle_z = -atan2(y2-y1, x2-x1) * 180 / M_PI;
        _trackResult.eye_dist = sqrt(square(x2-x1) + square(y2-y1));
    }
}