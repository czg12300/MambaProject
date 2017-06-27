//
// Created by jakechen on 2017/6/1.
//
#ifndef SG_FFMPEGEFFECT_H
#define SG_FFMPEGEFFECT_H

#include "FFmpegBase.h"
#include <VideoCodeProduce.h>
#include <VideoRepeatEffect.h>
#include "AudioDealProduce.h"
#include "AudioRepeatEffect.h"
#include "AudioNormalEffect.h"

namespace video {
    const int STATE_FAIL = -1;
    const int STATE_PROGRESS = 0;
    const int STATE_SUCCESS = 1;


    /**
        * 正常
        */
    static const int TYPE_NORMAL = 1;
    /**
     * 鬼畜
     */
    static const int TYPE_REPEAT = 2;
    /**
     * 相对论
     */
    static const int TYPE_RELATIVE = 3;


    class OptProgressListener : public OnProgressListener {
    private:
        int *total;
        int *progress;
        int *state;
    public:


        OptProgressListener(int *total, int *progress, int *state);

        void onProgress(int total, int progress);

        void onSuccess();

        void onFail();
    };


    class FFmpegEffect {
    private:
        BaseVideoEffect *
        createVideoEffect(int type, long start, long end, OnProgressListener *listener);

        BaseAudioEffect *
        createAudioEffect(int type, long start, long end, OnProgressListener *listener);

    public:
        FFmpegEffect();

        ~FFmpegEffect();

        void
        handleEffect(bool isDealAudio, int type, string srcFile, string outFile, long rangStart,
                     long rangeEnd,
                     long effectStart, long effectEnd, OnProgressListener *listener);
    };

}
#endif