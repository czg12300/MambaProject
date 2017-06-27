//
// Created by jakechen on 2017/6/3.
//
#ifndef BASEVIDEOEFFECT_DECODEFRAME
#define BASEVIDEOEFFECT_DECODEFRAME

#include <OnProgressListener.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
}

namespace video {

    struct DecodeFrame {
        int writeTimes;
        AVFrame *frame;
    };

    class BaseVideoEffect {

    public:
        virtual bool needDecode()=0;

        virtual void
        init(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx, AVCodecContext *decode_ctx,
             AVCodecContext *encode_ctx, AVStream *in_stream, AVStream *out_stream, long rangeStart,
             long rangeEnd)=0;

/**
 * 根据当前的read出来的时间判断是否要向前seek
 * @param pkt
 * @return 需要seek到的时间点，单位为毫秒
 */
        virtual long getNeedSeekTime(AVPacket pkt)=0;

        virtual void optFrame(DecodeFrame *frame)=0;

        virtual void onReadPackage(AVPacket pkt)=0;

        virtual void release()=0;

        virtual void onSuccess()=0;

        virtual void onFail()=0;

    };

}
#endif
