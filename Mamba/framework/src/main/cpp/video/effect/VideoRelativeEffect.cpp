//
// Created by jakechen on 2017/6/6.
//

#include "VideoRelativeEffect.h"


namespace video {
    VideoRelativeEffect::VideoRelativeEffect(long startTime, long endTime,
                                             OnProgressListener *listener) {
        this->startTime = startTime;
        this->endTime = endTime;
        this->listener = listener;
    }


    void VideoRelativeEffect::optFrame(DecodeFrame *frame) {
        double time = av_q2d(in_stream->time_base) * frame->frame->pts * 1000;
        if (startTime < endTime) {
            if (time >= startTime && time <= endTime) {
                long effectDuration = endTime - startTime;
                int frameCount = effectDuration * frameRate;
                int needAdd = 1000 * frameRate;
                int remove_frame_spit = (int) (frameCount * 1.0f / needAdd + 0.5);
                if ((slowFrameIndex % remove_frame_spit) == 0) {
                    frame->writeTimes = 2;
                }
                slowFrameIndex++;
            } else {
                long otherDuration = duration - (endTime - startTime);
                int frameCount = otherDuration * frameRate;
                int needAdd = 1000 * frameRate;
                int remove_frame_spit = (int) (frameCount * 1.0f / needAdd + 0.5);
                if ((festFrameIndex % remove_frame_spit) == 0) {
                    frame->writeTimes = 0;
                }
                festFrameIndex++;
            }
        }
    }

    void VideoRelativeEffect::onReadPackage(AVPacket pkt) {
        if (listener != NULL) {
            double time = av_q2d(in_stream->time_base) * pkt.pts * 1000;
            listener->onProgress(rangeEnd - rangeStart, time);
        }
    }

    bool VideoRelativeEffect::needDecode() {
        return true;
    }

    long VideoRelativeEffect::getNeedSeekTime(AVPacket pkt) {
        return -1;
    }

    void VideoRelativeEffect::init(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx,
                                   AVCodecContext *decode_ctx, AVCodecContext *encode_ctx,
                                   AVStream *in_stream, AVStream *out_stream, long rangeStart,
                                   long rangeEnd) {
        this->rangeStart = rangeStart;
        this->rangeEnd = rangeEnd;
        duration = ifmt_ctx->duration / 1000;
        frameRate = ((float) in_stream->avg_frame_rate.num / in_stream->avg_frame_rate.den) + 0.5;
        this->in_stream = in_stream;
    }


    void VideoRelativeEffect::release() {

    }

    void VideoRelativeEffect::onFail() {
        if (listener != NULL) {
            listener->onFail();
        }
    }

    void VideoRelativeEffect::onSuccess() {
        if (listener != NULL) {
            listener->onSuccess();
        }
    }

}
