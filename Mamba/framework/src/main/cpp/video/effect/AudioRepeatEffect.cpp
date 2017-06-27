//
// Created by bearshi on 2017/6/6.
//

#include "AudioRepeatEffect.h"
#include "log.h"
const int REPEAT_TOTAL_TIMES = 3;

namespace video {
    AudioRepeatEffect::AudioRepeatEffect(long startTime, long endTime,
                                         OnProgressListener *listener) {
        this->startTime = startTime;
        this->endTime = endTime;
        this->listener = listener;
        this->repeatTimes = 0;
        this->filterStrings = "anull";
        this->speedUp = false;
        this->speedNormal = false;

        LOGI("AudioRepeatEffect  startTime = %ld endTime = %ld", startTime, endTime);
    }

    void AudioRepeatEffect::init(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx,
                                 AVCodecContext *decode_ctx, AVCodecContext *encode_ctx,
                                 AVStream *in_stream, int frameCount, long rangeStart, long rangeEnd) {
        this->frameCount = frameCount;
        this->in_stream =  in_stream;
        this->rangeEnd = rangeEnd;
        this->rangeStart = rangeStart;
        if (this->endTime >= this->rangeEnd) {
            this->endTime = rangeEnd - 40;
        }

        LOGI("AudioDealProduce effect starttime = %ld endtime = %d", startTime, endTime);
        LOGI("AudioDealProduce range rangeStart = %ld rangeEnd = %d", rangeStart, rangeEnd);
    }

    bool AudioRepeatEffect::needDecode() {
        return true;
    }

    std::string AudioRepeatEffect::filterString()
    {
        return this->filterStrings;
    }

    void AudioRepeatEffect::optFrame(DecodeAudioFrame *frame) {
        //listener->onProgress(frameCount, frame->frame_index);
    }

    bool AudioRepeatEffect::getNeedNewFilter(AVPacket pkt){
        double time = av_q2d(in_stream->time_base) * pkt.pts * 1000;
        if (time >= startTime && time <= endTime && !speedUp ) {
            LOGI("AudioDealProduce effect00 starttime = %lf time = %lf", startTime, time);
            filterStrings = "atempo=2.0,atempo=2.0"; //
            speedUp = true;
            return true;
        }
        if (time > endTime && !speedNormal && repeatTimes >= REPEAT_TOTAL_TIMES) {
            LOGI("AudioDealProduce effect11 starttime = %lf time = %lf", startTime, time);
            filterStrings = "anull";
            speedNormal = true;
            return true;
        }

        return false;
    }

    long AudioRepeatEffect::getNeedSeekTime(AVPacket pkt)
    {
        double time = av_q2d(in_stream->time_base) * pkt.pts * 1000;
        long timeEnd = endTime > rangeEnd ? rangeEnd : endTime;
        if (startTime < endTime  && time > timeEnd && repeatTimes++ < REPEAT_TOTAL_TIMES) {
            LOGI("AudioDealProduce getNeedSeekTime startTime = %ld endTime = %ld time = %lf timeEnd = %ld", startTime, endTime, time, timeEnd);
            return startTime;
        }

        return -1;
    }

    void AudioRepeatEffect::release() {

    }

    void AudioRepeatEffect::onSuccess() {
        listener->onSuccess();
    }

    void AudioRepeatEffect::onFail() {
        listener->onFail();
    }

    void AudioRepeatEffect::onProgress(int total, int curr_index) {
        listener->onProgress(total, curr_index);
    }
}

