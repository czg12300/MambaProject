//
// Created by jakechen on 2017/6/6.
//

#ifndef NEWSHOWGUYS_ANDROID_VIDEORELATIVEEFFECT_H
#define NEWSHOWGUYS_ANDROID_VIDEORELATIVEEFFECT_H

#include "BaseVideoEffect.h"
#include "FFmpegBase.h"
namespace video{
    class VideoRelativeEffect : public BaseVideoEffect {
    private:
        long startTime;
        long endTime;
        OnProgressListener *listener;
        long duration;
        long rangeStart;
        long rangeEnd;
        AVStream *in_stream;
        int frameRate;
//        double lastSlowFrameTime=-1;
//        double lastFestFrameTime=-1;

        int slowFrameIndex=0;
        int festFrameIndex=0;
    public:
        VideoRelativeEffect(long startTime, long endTime, OnProgressListener *listener);

        void release();

        void optFrame(DecodeFrame *frame);

        bool needDecode();

        long getNeedSeekTime(AVPacket pkt);

        void init(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx, AVCodecContext *decode_ctx,
                  AVCodecContext *encode_ctx,
                  AVStream *in_stream, AVStream *out_stream, long rangeStart,
                  long rangeEnd);


        void onFail();

        void onSuccess();


        void onReadPackage(AVPacket pkt);
    };
}


#endif //NEWSHOWGUYS_ANDROID_VIDEORELATIVEEFFECT_H
