//
// Created by bearshi on 2017/6/6.
//

#ifndef ANDROID_AUDIOREPEATEFFECT_H
#define ANDROID_AUDIOREPEATEFFECT_H

#include "BaseAudioEffect.h"
using namespace std;

namespace video {
    class AudioRepeatEffect :public BaseAudioEffect {
    private:
        long startTime;
        long endTime;
        OnProgressListener *listener;
        long rangeStart;
        long rangeEnd;
    private:
        int frameCount;
        int repeatTimes;
        string filterStrings;
        AVStream *in_stream;
        bool speedUp;
        bool speedNormal;

    public:
        AudioRepeatEffect(long startTime, long endTime, OnProgressListener *listener) ;

        long getNeedSeekTime(AVPacket pkt);

        void init(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx, AVCodecContext *decode_ctx,
                  AVCodecContext *encode_ctx, AVStream *in_stream, int frameCount, long rangeStart, long rangeEnd);

        bool needDecode();

        std::string  filterString();

        bool getNeedNewFilter(AVPacket pkt);

        void optFrame(DecodeAudioFrame *frame);

        void release();

        void onSuccess();

        void onFail();

        void onProgress(int total, int curr_index);
    };
}

#endif //ANDROID_AUDIOREPEATEFFECT_H
