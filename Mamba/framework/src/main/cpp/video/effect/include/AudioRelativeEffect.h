//
// Created by bearshi on 2017/6/8.
//

#ifndef ANDROID_AUDIORELATIVEEFFECT_H
#define ANDROID_AUDIORELATIVEEFFECT_H

#include "BaseAudioEffect.h"
using namespace std;

namespace video {
    class AudioRelativeEffect :public BaseAudioEffect {
    private:
        long startTime;
        long endTime;
        OnProgressListener *listener;
    private:
        int frameCount;
        string filterStrings;
        AVStream *in_stream;
        bool speedDown;
        bool speedNormal;

        long duration;
        string startStrings;
        char filters[20];

    public:
        AudioRelativeEffect(long startTime, long endTime, OnProgressListener *listener) ;

        long getNeedSeekTime(AVPacket pkt);

        void init(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx, AVCodecContext *decode_ctx,
                  AVCodecContext *encode_ctx, AVStream *in_stream, int frameCount, long rangeStart, long rangeEnd);

        bool needDecode();

        string  filterString();

        bool getNeedNewFilter(AVPacket pkt);

        void optFrame(DecodeAudioFrame *frame);

        void release();

        void onSuccess();

        void onFail();

        void onProgress(int total, int curr_index);
    };
}


#endif //ANDROID_AUDIORELATIVEEFFECT_H
