//
// Created by jakechen on 2017/6/1.
//
#ifndef SG_ONPROGRESSLISTENER
#define SG_ONPROGRESSLISTENER
namespace video {


    class OnProgressListener {
    public:
        virtual void onProgress(int total, int progress)=0;

        virtual void onSuccess()=0;

        virtual void onFail()=0;
    };


}
#endif
