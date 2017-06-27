//
// Created by jakechen on 2017/6/6.
//
#include "VideoNormalEffect.h"

namespace video {
    VideoNormalEffect::VideoNormalEffect(OnProgressListener *listener) {
        this->listener = listener;
    }


    void VideoNormalEffect::optFrame(DecodeFrame *frame) {

    }

    bool VideoNormalEffect::needDecode() {
        return rangeStart > 0;
    }

    long VideoNormalEffect::getNeedSeekTime(AVPacket pkt) {
        return -1;
    }

    void VideoNormalEffect::init(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx,
                                 AVCodecContext *decode_ctx, AVCodecContext *encode_ctx,
                                 AVStream *in_stream, AVStream *out_stream, long rangeStart,
                                 long rangeEnd) {
        duration = ifmt_ctx->duration / 1000;
        this->rangeStart = rangeStart;
        this->rangeEnd = rangeEnd;
        this->in_stream = in_stream;
    }


    void VideoNormalEffect::release() {

    }

    void VideoNormalEffect::onFail() {
        if (listener != NULL) {
            listener->onFail();
        }
    }

    void VideoNormalEffect::onReadPackage(AVPacket pkt) {
        if (listener != NULL) {
            double time = av_q2d(in_stream->time_base) * pkt.pts * 1000;
            listener->onProgress(rangeEnd - rangeStart, time);
        }
    }

    void VideoNormalEffect::onSuccess() {
        if (listener != NULL) {
            listener->onSuccess();
        }
    }
}
