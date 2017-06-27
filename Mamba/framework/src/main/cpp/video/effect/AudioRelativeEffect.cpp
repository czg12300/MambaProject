//
// Created by bearshi on 2017/6/8.
//

#include "include/AudioRelativeEffect.h"
#include "log.h"

namespace video {
    AudioRelativeEffect::AudioRelativeEffect(long startTime, long endTime,
                                             OnProgressListener *listener) {
        this->startTime = startTime;
        this->endTime = endTime;
        this->listener = listener;
        this->speedDown = false;
        this->filterStrings = "anull";
        this->speedNormal = false;
    }

    void AudioRelativeEffect::init(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx,
                                   AVCodecContext *decode_ctx, AVCodecContext *encode_ctx,
                                   AVStream *in_stream, int frameCount, long rangeStart, long rangeEnd) {
        this->frameCount = frameCount;
        this->in_stream = in_stream;
        this->duration = ifmt_ctx->duration / 1000;
        double ral = (endTime - startTime);

        double rate = 1.0;
        if (ral < 0) ral = 0;
        rate = 1.0*(duration - ral) /(duration - ral - 1000);

        LOGI("AudioDealProduce:: duration = %ld", duration);
        LOGI("AudioDealProduce:: startTime = %ld endTime = %ld ral = %lf",startTime, endTime, ral);
        if (rate <= 0.5 ) rate = 0.5;
        if (rate >= 2.0 ) rate = 2.0;
        sprintf(filters, "atempo=%.2lf", rate);
        this->startStrings = filters;
        this->filterStrings = filters;
    }

    bool AudioRelativeEffect::needDecode() {
        return true;
    }

    std::string AudioRelativeEffect::filterString()
    {
        return this->filterStrings;
    }

    void AudioRelativeEffect::optFrame(DecodeAudioFrame *frame) {
        //listener->onProgress(frameCount, frame->frame_index);
    }

    void AudioRelativeEffect::release() {

    }

    void AudioRelativeEffect::onSuccess() {
        listener->onSuccess();
    }

    void AudioRelativeEffect::onFail() {
        listener->onFail();
    }

    bool AudioRelativeEffect::getNeedNewFilter(AVPacket pkt){
        double time = av_q2d(in_stream->time_base) * pkt.pts * 1000;
        if (time >= startTime && time <= endTime && !speedDown) {
            double ral = (endTime - startTime);
            double rate = (ral - 1000) / ral;
            //char filters[20];

            if (rate <= 0.5 ) rate = 0.5;
            if (rate >= 2.0 ) rate = 2.0;
            sprintf(filters, "atempo=%.2lf", rate);
            //sprintf(filters, "atempo=2.0");
            this->filterStrings = filters;  //"anull";
            speedDown = true;
            return true;
        }
        if(time > endTime && !speedNormal) {
           // this->filterStrings = "anull"; //"atempo=2.0";   //
            this->filterStrings = startStrings;
            speedNormal = true;
            return true;
        }

        return false;
    }

    long AudioRelativeEffect::getNeedSeekTime(AVPacket pkt)
    {
        return -1;
    }

    void AudioRelativeEffect::onProgress(int total, int curr_index) {
        listener->onProgress(total, curr_index);
    }
}

