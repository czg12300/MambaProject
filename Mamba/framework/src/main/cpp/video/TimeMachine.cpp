//
// Created by jakechen on 2017/3/16.
//

#include "TimeMachine.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int lastVideoProgress = 0;
static int lastAudioProgress = 0;
static bool isBeauty = false;

void *optAudioInThread(void *arg);

void *optVideoInThread(void *arg);

void *optVideoInThread(void *arg) {
    OptVideoData *videoData = (OptVideoData *) arg;
    if (videoData != NULL && videoData->factory != NULL) {
        *videoData->callback->ret = videoData->factory->optVideo(videoData->handler);
        if (videoData->callback != NULL) {
            videoData->callback->onFinish(videoData->callback->total,
                                          videoData->callback->total);
        }
    }
    return NULL;
}

void *optAudioInThread(void *arg) {
    OptAudioData *audioData = (OptAudioData *) arg;
    if (audioData != NULL && audioData->factory != NULL) {
        if (audioData->handler != NULL) {
            if (audioData->handler->type == TYPE_FAST) {
                *audioData->callback->ret = audioRateFast(audioData->factory->outAac.c_str(),
                                                          audioData->factory->outOptAac.c_str(),
                                                          audioData->handler->rate / 100.0f,
                                                          audioData->callback);
            } else {
                *audioData->callback->ret = audioRate(audioData->factory->outAac.c_str(),
                                                      audioData->factory->outOptAac.c_str(),
                                                      audioData->handler->rate / 100.0f,
                                                      audioData->callback);
            }
            if (audioData->callback != NULL) {
                audioData->callback->onFinish(audioData->callback->total,
                                              audioData->callback->total);
            }
        }
    }
    return NULL;
}

int fastVideo(string srcFile, string outFile, int rate, HandleProgressCallback *callback) {
    int progress = 0;
    int ret = -1;
    lastVideoProgress = 0;
    lastAudioProgress = 0;
    int retOptAudio = 1;
    int retOptVideo = 1;
    VideoRecodeFactory *factory = new VideoRecodeFactory(srcFile, outFile);
    OptVideoProgressCallback *videoCallback = new OptVideoProgressCallback(&progress, &retOptVideo,
                                                                           rate > 0 ? 50 : 100);
    TimeMachineVideoOptHandler *handler = new TimeMachineVideoOptHandler(factory, videoCallback,
                                                                         TYPE_FAST,
                                                                         rate);
    string aac = factory->outAac;
    OptVideoData *videoData = (OptVideoData *) malloc(sizeof(OptVideoData));
    OptAudioData *audioData;
    OptAudioProgressCallback *audioCallback;
    ret = factory->demuxerAudio();
    if (ret < 0) {
        goto end;
    }
    videoData->callback = videoCallback;
    videoData->factory = factory;
    videoData->handler = handler;
    pool_add_worker(optVideoInThread, videoData);
    if (rate > 0) {
        audioData = (OptAudioData *) malloc(sizeof(OptAudioData));
        audioCallback = new OptAudioProgressCallback(&progress, &retOptAudio, 50);
        audioData->callback = audioCallback;
        audioData->factory = factory;
        audioData->handler = handler;
        pool_add_worker(optAudioInThread, audioData);
        aac = factory->outOptAac;
    }
    while (true) {
        if (callback != NULL) {
            callback->onHandleProgress(progress, 100);
        }
        if (progress >= 100 || retOptVideo < 0 || retOptAudio < 0) {
            break;
        }
        usleep(20);
    }
    ret = factory->muxer(factory->outOptH264, aac, factory->outFile);
    if (ret < 0) {
        goto end;
    }
    ret = 1;
    end:
    free(videoData);
    if (audioData != NULL) {
        free(audioData);
    }
    delete (videoCallback);
    if (audioCallback != NULL) {
        delete (audioCallback);
    }
    delete (handler);
    delete (factory);
    return ret;

}

int slowVideo(string srcFile, string outFile, int rate, HandleProgressCallback *callback) {
    int progress = 0;
    int ret = -1;
    lastVideoProgress = 0;
    lastAudioProgress = 0;
    int retOptAudio = 1;
    int retOptVideo = 1;
    VideoRecodeFactory *factory = new VideoRecodeFactory(srcFile, outFile);
    OptVideoProgressCallback *videoCallback = new OptVideoProgressCallback(&progress, &retOptVideo,
                                                                           rate > 0 ? 50 : 100);
    TimeMachineVideoOptHandler *handler = new TimeMachineVideoOptHandler(factory, videoCallback,
                                                                         TYPE_SLOW,
                                                                         rate);
    string aac = factory->outAac;
    OptVideoData *videoData = (OptVideoData *) malloc(sizeof(OptVideoData));
    OptAudioData *audioData;
    OptAudioProgressCallback *audioCallback;
    ret = factory->demuxerAudio();
    if (ret < 0) {
        goto end;
    }
    videoData->callback = videoCallback;
    videoData->factory = factory;
    videoData->handler = handler;
    pool_add_worker(optVideoInThread, videoData);
    if (rate > 0) {
        audioData = (OptAudioData *) malloc(sizeof(OptAudioData));
        audioCallback = new OptAudioProgressCallback(&progress, &retOptAudio, 50);
        audioData->callback = audioCallback;
        audioData->factory = factory;
        audioData->handler = handler;
        pool_add_worker(optAudioInThread, audioData);
        aac = factory->outOptAac;
    }
    while (true) {
        if (callback != NULL) {
            callback->onHandleProgress(progress, 100);
        }
        if (progress >= 100 || retOptVideo < 0 || retOptAudio < 0) {
            break;
        }
        usleep(10);
    }
    ret = factory->muxer(factory->outOptH264, aac, factory->outFile);
    if (ret < 0) {
        goto end;
    }
    ret = 1;
    end:
    free(videoData);
    if (audioData != NULL) {
        free(audioData);
    }
    delete (videoCallback);
    if (audioCallback != NULL) {
        delete (audioCallback);
    }
    delete (handler);
    delete (factory);
    return ret;
}


OptVideoProgressCallback::OptVideoProgressCallback(int *progress, int *ret, int total) {
    this->progress = progress;
    this->ret = ret;
    this->total = total;
}


void OptVideoProgressCallback::onProgress(int progress, int total) {
    pthread_mutex_lock(&mutex);
    int temp = progress * this->total / total;
    if (temp > lastVideoProgress) {
        *this->progress += temp - lastVideoProgress;
        lastVideoProgress = temp;
    }
    pthread_mutex_unlock(&mutex);
}

void OptVideoProgressCallback::onFinish(int progress, int total) {
    pthread_mutex_lock(&mutex);
    if (this->total > lastVideoProgress) {
        *this->progress += this->total - lastVideoProgress;
        lastVideoProgress = this->total;
    }
    pthread_mutex_unlock(&mutex);
}


OptAudioProgressCallback::OptAudioProgressCallback(int *progress, int *ret, int total) {
    this->progress = progress;
    this->ret = ret;
    this->total = total;
}

void OptAudioProgressCallback::onFinish(int progress, int total) {
    pthread_mutex_lock(&mutex);
    if (this->total > lastAudioProgress) {
        *this->progress += this->total - lastAudioProgress;
        lastAudioProgress = this->total;
    }
    pthread_mutex_unlock(&mutex);

}


void OptAudioProgressCallback::onProgress(int progress, int total) {
    pthread_mutex_lock(&mutex);
    int temp = progress * this->total / total;
    if (temp > lastAudioProgress) {
        *this->progress += temp - lastAudioProgress;;
        lastAudioProgress = temp;
    }
    pthread_mutex_unlock(&mutex);
}


TimeMachineVideoOptHandler::TimeMachineVideoOptHandler(VideoRecodeFactory *factory,
                                                       OptVideoProgressCallback *callback, int
                                                       type,
                                                       int
                                                       rate) {
    this->rate = rate;
    this->type = type;
    this->callback = callback;
    this->factory = factory;
}


AVFrame *TimeMachineVideoOptHandler::optAvFrame(AVFrame *frame, int frameIndex) {
    if (callback != NULL) {
        callback->onProgress(frameIndex, this->frameCount);
    }
    if (isBeauty) {
        SwsContext *img_convert_ctx = sws_getContext(codecContext->width, codecContext->height,
                                                     codecContext->pix_fmt, codecContext->width,
                                                     codecContext->height, AV_PIX_FMT_RGB24,
                                                     SWS_BICUBIC, NULL,
                                                     NULL, NULL);
        sws_scale(img_convert_ctx, (const uint8_t *const *) frame->data, frame->linesize, 0,
                  codecContext->height, rgbFrame->data, rgbFrame->linesize);
        //处理美颜效果
        beautifyAlgorithm(rgbFrame);
    }
    if (TYPE_SLOW == type) {
        double rateTemp = 0;
        if (rate < MAX) {//理论上放慢100%就是永远停止这一帧，但是这样程序就会出错，所以做这个容错 处理
            rateTemp = (double) MAX / (double) (MAX - rate);
        } else {
            rateTemp = MAX;
        };
        int addFrameCount = ((int) (frameCount * rateTemp)) % frameCount;
        if (addFrameCount > 0) {
            double remove_frame_spit_d = (double) frameCount / (double) addFrameCount;
            bool needReverse = false;
            if (remove_frame_spit_d < 2) {
                remove_frame_spit_d =
                        (double) frameCount / (double) (frameCount - addFrameCount);
                needReverse = true;
            }
            int remove_frame_spit = (int) (remove_frame_spit_d + 0.5);
            if ((frameIndex % remove_frame_spit) == 0) {
                if (needReverse) {
                    factory->encode_write_frame(frame);
                }
            } else {
                if (!needReverse) {
                    factory->encode_write_frame(frame);
                }
            }
        }

        if (rateTemp > 1) {
            int i = 0;
            while (i < rateTemp) {
                factory->encode_write_frame(frame);
                i++;
            }
        }
    }
    return isBeauty ? rgbFrame : frame;
}

int TimeMachineVideoOptHandler::changeFrameRate(int frameRate) {
    LOGD("frameRate %d", frameRate);
//    originFrameRate = frameRate;
//    if (TYPE_SLOW == type) {
//        this->newFrameRate = frameRate / rate;
//    } else if (TYPE_FAST == type) {
//        this->newFrameRate = frameRate;
//    }
//    this->duration = this->frameCount / this->newFrameRate;
    return frameRate;
}

int TimeMachineVideoOptHandler::init(AVFormatContext *ifmt_ctx, AVCodecContext *decodecCtx,
                                     int video_index, int frameCount) {
    this->frameCount = frameCount;
    this->codecContext = decodecCtx;
    if (isBeauty) {
        rgbFrame = av_frame_alloc();
        rgbFrame->width = decodecCtx->width;
        rgbFrame->height = decodecCtx->height;
        uint8_t *out_buffer = new uint8_t[avpicture_get_size(AV_PIX_FMT_RGB24, decodecCtx->width,
                                                             decodecCtx->height)];
        avpicture_fill((AVPicture *) rgbFrame, out_buffer, AV_PIX_FMT_RGB24, decodecCtx->width,
                       decodecCtx->height);
    }
//    rgbFrame->format=AV_PIX_FMT_RGB24;
//    originFrameRate = ifmt_ctx->streams[video_index]->avg_frame_rate.num /
//                      ifmt_ctx->streams[video_index]->avg_frame_rate.den;
    return 1;
}

void TimeMachineVideoOptHandler::changeTimebase(AVStream *in_stream, AVStream *out_stream,
                                                AVPacket pkt) {

//    if (TYPE_SLOW == type) {
    pkt.duration = (int) av_rescale_q((int64_t) pkt.duration, in_stream->time_base,
                                      out_stream->time_base);
    pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
                               (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
    pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
                               (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
    pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
    pkt.pos = -1;
    pkt.stream_index = 0;
//    }

}

void TimeMachineVideoOptHandler::releaseInEnd() {
//    free(dts_start_from);
//    free(pts_start_from);
    //释放美颜
    if (isBeauty) {
        freeM();
        av_frame_free(&rgbFrame);
    }

}

AllowEncodeAndWriteFrameState
TimeMachineVideoOptHandler::allowEncodeAndWriteFrame(AVStream *in_stream, AVPacket pkt,
                                                     int frameIndex) {
    if (this->type == TYPE_FAST) {
        if (rate >= MAX) {//防止程序出错，理论上加速100%是一帧都不保存，但这样就会没有文件输出了
            rate = MAX - 1;
        }
        int left_frame_count = frameCount * (MAX - rate) / MAX;
        int remove_frame_count = frameCount - left_frame_count;
        if (remove_frame_count == 0) {
            return STATE_ALLOW;
        }
        double remove_frame_spit_d = (double) frameCount / (double) remove_frame_count;
        bool needReverse = false;
        if (remove_frame_spit_d < 2) {
            remove_frame_spit_d = (double) frameCount / (double) left_frame_count;
            needReverse = true;
        }
        int remove_frame_spit = (int) (remove_frame_spit_d + 0.5);
        if ((frameIndex % remove_frame_spit) == 0) {
            return needReverse ? STATE_ALLOW : STATE_NOT_ALLOW;
        }
        return needReverse ? STATE_NOT_ALLOW : STATE_ALLOW;
    } else if (this->type == TYPE_SLOW) {
//do nothing
    }

    return STATE_ALLOW;

}
