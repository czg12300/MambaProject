//
// Created by bearshi on 2017/6/7.
//

#ifndef ANDROID_AUDIONORMALEFFECT_H
#define ANDROID_AUDIONORMALEFFECT_H

#include "BaseAudioEffect.h"
#include <string>
using namespace std;

namespace video {
    class AudioNormalEffect:public BaseAudioEffect {
    private:
        OnProgressListener *listener;
    private:
        int frameCount;
    public:
        AudioNormalEffect(OnProgressListener *listener) ;

        void init(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx, AVCodecContext *decode_ctx,
                  AVCodecContext *encode_ctx, AVStream *in_stream, int frameCount, long rangeStart, long rangeEnd);

        bool needDecode();

        std::string filterString();

        bool getNeedNewFilter(AVPacket pkt);

        long getNeedSeekTime(AVPacket pkt);

        void optFrame(DecodeAudioFrame *frame) ;

        void release();

        void onSuccess();

        void onFail();

        void onProgress(int total, int curr_index);
    };
}

#endif //ANDROID_AUDIONORMALEFFECT_H
