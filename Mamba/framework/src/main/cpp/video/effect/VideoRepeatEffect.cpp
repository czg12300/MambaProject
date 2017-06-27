//
// Created by jakechen on 2017/6/3.
//

#include "VideoRepeatEffect.h"

namespace video {
    VideoRepeatEffect::VideoRepeatEffect(long startTime, long endTime,
                                         OnProgressListener *listener) {
        this->startTime = startTime;
        this->endTime = endTime;
        this->listener = listener;
    }

    long VideoRepeatEffect::getNeedSeekTime(AVPacket pkt) {
        double time = av_q2d(in_stream->time_base) * pkt.pts * 1000;
        long timeEnd = endTime >= rangeEnd ? rangeEnd : endTime;
        if (startTime < timeEnd && time >= timeEnd && repeatTimes++ < 4) {
            return startTime;
        }
        return -1;
    }

    void VideoRepeatEffect::init(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx,
                                 AVCodecContext *decode_ctx, AVCodecContext *encode_ctx,
                                 AVStream *in_stream, AVStream *out_stream, long rangeStart,
                                 long rangeEnd) {
        duration = ifmt_ctx->duration / 1000;
        this->in_stream = in_stream;
        this->rangeEnd = rangeEnd;
        this->rangeStart = rangeStart;
        frameRate = (int) (((float) in_stream->avg_frame_rate.num / in_stream->avg_frame_rate.den) +
                           0.5);
    }

    void VideoRepeatEffect::onReadPackage(AVPacket pkt) {
        if (listener != NULL) {
            double time = av_q2d(in_stream->time_base) * pkt.pts * 1000;
            listener->onProgress(rangeEnd - rangeStart, time);
        }
    }

    bool VideoRepeatEffect::needDecode() {
        return true;
    }

    void VideoRepeatEffect::optFrame(DecodeFrame *frame) {

        double time = av_q2d(in_stream->time_base) * frame->frame->pts * 1000;
        double newTimeSpit = 4000.0f / frameRate;
        if (time < lastTime) {
            lastFrameTime = -1;
        }
        if (time > startTime && time < endTime) {
            if (lastFrameTime < 0 || time > (lastFrameTime + newTimeSpit)) {
                lastFrameTime = time;
            } else {
                frame->writeTimes = 0;
            }
        }
        lastTime = time;

    }


    void VideoRepeatEffect::release() {

    }

    void VideoRepeatEffect::onSuccess() {
        if (listener != NULL) {
            listener->onSuccess();
        }
    }

    void VideoRepeatEffect::onFail() {
        if (listener != NULL) { listener->onFail(); }
    }
}