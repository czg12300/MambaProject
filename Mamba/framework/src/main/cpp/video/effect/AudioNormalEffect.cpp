//
// Created by bearshi on 2017/6/7.
//


#include "AudioNormalEffect.h"

namespace video {
    AudioNormalEffect::AudioNormalEffect(OnProgressListener *listener) {
        this->listener = listener;
    }

    void AudioNormalEffect::init(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx, AVCodecContext *decode_ctx,
                                 AVCodecContext *encode_ctx, AVStream *in_stream, int frameCount, long rangeStart, long rangeEnd){
    }
    bool AudioNormalEffect::needDecode() {
        return true;
    }
    string AudioNormalEffect::filterString()
    {
        return "anull";
    }
    void AudioNormalEffect::release() {

    }
    void AudioNormalEffect::optFrame(DecodeAudioFrame *frame) {
    }

    void AudioNormalEffect::onSuccess() {
        listener->onSuccess();
    }
    void AudioNormalEffect::onFail() {
        listener->onFail();
    }
    bool AudioNormalEffect::getNeedNewFilter(AVPacket pkt){
        return false;
    }
    long AudioNormalEffect::getNeedSeekTime(AVPacket pkt)
    {
        return -1;
    }
    void AudioNormalEffect::onProgress(int total, int curr_index) {
        listener->onProgress(total, curr_index);
    }
}

