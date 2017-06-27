//
// Created by jakechen on 2017/6/6.
//

#ifndef NEWSHOWGUYS_ANDROID_VIDEONORMALEFFECT_H
#define NEWSHOWGUYS_ANDROID_VIDEONORMALEFFECT_H

#include "BaseVideoEffect.h"

namespace video {
    class VideoNormalEffect : public BaseVideoEffect {
    private:
        OnProgressListener *listener;
        long duration;
        AVStream *in_stream;
        long rangeStart;
        long rangeEnd;
    public:
        VideoNormalEffect(OnProgressListener *listener);

        void release();

        void optFrame(DecodeFrame *frame);

        bool needDecode();

        long getNeedSeekTime(AVPacket pkt);

        void init(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx, AVCodecContext *decode_ctx,
                  AVCodecContext *encode_ctx, AVStream *in_stream, AVStream *out_stream,long rangeStart,
                  long rangeEnd);


        void onFail();

        void onSuccess();

        void onReadPackage(AVPacket pkt);
    };
}


#endif //NEWSHOWGUYS_ANDROID_VIDEONORMALEFFECT_H
