//
// Created by jakechen on 2017/6/3.
//
#include "BaseVideoEffect.h"
#include "FFmpegBase.h"
namespace video {
    class VideoRepeatEffect : public BaseVideoEffect {
    private:
        long startTime;
        long endTime;
        OnProgressListener *listener;
        long duration;
        long rangeEnd;
        long rangeStart;
        AVStream *in_stream;
        int frameRate;
        double lastFrameTime=-1;
        double lastTime=-1;
        int repeatTimes=0;
    public:
        VideoRepeatEffect(long startTime, long endTime, OnProgressListener *listener);

        long getNeedSeekTime(AVPacket pkt);

        void init(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx, AVCodecContext *decode_ctx,
                  AVCodecContext *encode_ctx, AVStream *in_stream, AVStream *out_stream, long rangeStart,
                  long rangeEnd);

        bool needDecode();

        void optFrame(DecodeFrame *frame);

        void release();

        void onSuccess();

        void onFail();

        void onReadPackage(AVPacket pkt);
    };
}

