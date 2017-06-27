//
// Created by bearshi on 2017/6/6.
//

#ifndef ANDROID_BASEAUDIOEFFECT_H
#define ANDROID_BASEAUDIOEFFECT_H

#include <OnProgressListener.h>
#include <string>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
}

namespace video {
    struct DecodeAudioFrame {
        int frame_index;
        int repeat_times;
        AVFrame *frame;
    };

    class BaseAudioEffect {
    public:
        virtual bool needDecode() = 0;

        virtual std::string filterString() = 0;

        /*
        virtual void
        init(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx, AVCodecContext *decode_ctx,
             AVCodecContext *encode_ctx, int frameCount) = 0;
        */

        virtual void init(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx, AVCodecContext *decode_ctx,
                          AVCodecContext *encode_ctx, AVStream *in_stream, int frameCount, long rangeStart, long rangeEnd) = 0;

        virtual long getNeedSeekTime(AVPacket pkt) = 0;

        virtual bool getNeedNewFilter(AVPacket pkt) = 0;

        virtual void optFrame(DecodeAudioFrame *frame) = 0;

        virtual void release() = 0;

        virtual void onSuccess() = 0;

        virtual void onFail() = 0;

        virtual void onProgress(int total, int curr_index) = 0;
    };
}

#endif //ANDROID_BASEAUDIOEFFECT_H
