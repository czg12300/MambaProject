//
// Created by jakechen on 2017/6/1.
//

#include <VideoNormalEffect.h>
#include <VideoRelativeEffect.h>
#include "FFmpegEffect.h"
#include "FfmpegMuxer.h"

#include "AudioRelativeEffect.h"
#include "FfmpegMuxering.h"
#include "AudioToAac.h"


namespace video {
    FFmpegEffect::FFmpegEffect() {

    }

    FFmpegEffect::~FFmpegEffect() {

    }

    void FFmpegEffect::handleEffect(bool isDealAudio, int type, string srcFile, string outFile,
                                    long rangStart,
                                    long rangeEnd,
                                    long effectStart, long effectEnd,
                                    OnProgressListener *listener) {
        LOGI("handleEffect start");
        string outVideo = outFile + ".h264";
        string outAudio = outFile + ".aac";
        bool hasAudioStream = false;
        int vTotal = 0;
        int vProgress = 0;
        int vState = STATE_PROGRESS;
        int aTotal = 0;
        int aProgress = 0;
        int aState = STATE_PROGRESS;
        int ret = -1;

        OnProgressListener *vListener = new OptProgressListener(&vTotal, &vProgress, &vState);
        VideoCodeParams *videoCodeParams = new VideoCodeParams();
        videoCodeParams->srcFile = srcFile;
        videoCodeParams->outFile = outVideo;
        videoCodeParams->rangeStart = rangStart;
        videoCodeParams->rangeEnd = rangeEnd;
        videoCodeParams->effect = createVideoEffect(type, effectStart, effectEnd, vListener);
        VideoCodeProduce *videoCodeProduce = new VideoCodeProduce(videoCodeParams);
        if (isDealAudio) {
            hasAudioStream = videoCodeProduce->hasAudioStream();
        }
        videoCodeProduce->start();
        LOGD("hasAudioStream  %d", hasAudioStream);

        AudioFilterParams *audioFilterParams;
        if (hasAudioStream) {
            OnProgressListener *aListener = new OptProgressListener(&aTotal, &aProgress, &aState);
            audioFilterParams = new AudioFilterParams();
            audioFilterParams->srcFile = srcFile;
            audioFilterParams->outFile = outAudio;
            audioFilterParams->rangeStart = rangStart;
            audioFilterParams->rangeEnd = rangeEnd;
            audioFilterParams->effect = createAudioEffect(type, effectStart, effectEnd, aListener);
            AudioDealProduce *audioCodeProduce = new AudioDealProduce(audioFilterParams);
            audioCodeProduce->Start();
        }

        int lastProgress = -1;
        while (listener != NULL) {
            int max = aTotal + vTotal;
            int progress = aProgress + vProgress;
            if (hasAudioStream) {
                if (vState == STATE_PROGRESS || aState == STATE_PROGRESS) {
                    if (lastProgress != progress) {
                        listener->onProgress(max, progress);
                        lastProgress = progress;
                    }
                } else if (vState == STATE_SUCCESS && aState == STATE_SUCCESS) {
                    LOGI("outVideo = %s", outVideo.c_str());
                    LOGI("outAudio = %s", outAudio.c_str());
                    LOGI("outFile = %s", outFile.c_str());
                    ret = muxing(outVideo.c_str(), outAudio.c_str(),
                                 outFile.c_str());
                    remove(outAudio.c_str());
                    remove(outVideo.c_str());
                    LOGI("muxing  ret = %d", ret);
                    LOGD("muxing  success");
                    break;
                } else {
                    remove(outAudio.c_str());
                    remove(outVideo.c_str());
                    ret = -1;
                    break;
                }
            } else {
                if (vState == STATE_PROGRESS) {
                    if (lastProgress != progress) {
                        listener->onProgress(max, progress);
                        lastProgress = progress;
                    }
                } else if (vState == STATE_SUCCESS) {
                    ret = h264ToFormat(outVideo.c_str(), outFile.c_str());
                    remove(outVideo.c_str());
                    LOGD("h264ToFormat ret = %d", ret);
                    break;
                } else {
                    ret = -1;
                    remove(outVideo.c_str());
                    LOGD("listener->onFail() vedio");
                    break;
                }
            }
            usleep(300);
        }

        LOGD("handleEffect ret = %d", ret);
        if (ret < 0) {
            listener->onFail();
        } else {
            listener->onSuccess();
        }

        LOGD("handleEffect finish ret = %d", ret);
    }

    BaseVideoEffect *
    FFmpegEffect::createVideoEffect(int type, long start, long end, OnProgressListener *listener) {
        BaseVideoEffect *effect = NULL;
        switch (type) {
            case TYPE_NORMAL:
                effect = new VideoNormalEffect(listener);
                break;
            case TYPE_REPEAT:
                effect = new VideoRepeatEffect(start, end, listener);
                break;
            case TYPE_RELATIVE:
                effect = new VideoRelativeEffect(start, end, listener);
                break;
            default:
                effect = new VideoNormalEffect(listener);
                break;
        }
        LOGD("createVideoEffect type %d", type);
        return effect;
    }

    BaseAudioEffect *
    FFmpegEffect::createAudioEffect(int type, long start, long end, OnProgressListener *listener) {
        BaseAudioEffect *effect = NULL;
        switch (type) {
            case TYPE_NORMAL:
                effect = new AudioNormalEffect(listener);
                break;
            case TYPE_REPEAT:
                effect = new AudioRepeatEffect(start, end, listener);
                break;
            case TYPE_RELATIVE:
                effect = new AudioRelativeEffect(start, end, listener);
                break;
            default:
                effect = new AudioNormalEffect(listener);
                break;
        }
        LOGD("createAudioEffect type %d", type);
        return effect;
    }

    OptProgressListener::OptProgressListener(int *total, int *progress, int *state) {
        this->total = total;
        this->progress = progress;
        this->state = state;
    }

    void OptProgressListener::onProgress(int total, int progress) {
        LOGD("max  =%d  progress=%d", total, progress);
        *this->total = 100;
        *this->progress = progress * 100 / total;
    }

    void OptProgressListener::onSuccess() {
        LOGD(" OptProgressListener::onSuccess");
        *state = STATE_SUCCESS;
    }

    void OptProgressListener::onFail() {
        LOGD(" OptProgressListener::onFail");
        *state = STATE_FAIL;
    }
}
