//
// Created by jakechen on 2017/3/20.
//

#include "RepeatMoment.h"

//丢帧，帧与帧之间的间隔
static int speed = 2;
//加速比例  0-10
static int rate = 8;
//相对论减速时间 如1s->3s
static int relativeTimes = 3;
//相对论减速时间间隔  如：1s 2s 3s
static double relativeTime = 1.0;

int repeatMoment(string srcFile, string outFile, double second) {
    VideoRecodeFactory *factory = new VideoRecodeFactory(srcFile, outFile);
    VideoOptHandler *handler = new RepeatMomentVideoOptHandler(factory, second, TYPE_REPEAT_MOMENT);
    int ret = factory->demuxerAudio();
    if (ret < 0) {
        return ret;
    }
    ret = factory->optVideo(handler);
    if (ret < 0) {
        return ret;
    }
    ret = factory->muxer(factory->outOptH264, factory->outAac, factory->outFile);
    delete (handler);
    delete (factory);
    return ret;
}

int relativeMoment(string srcFile, string outFile, double second) {
    VideoRecodeFactory *factory = new VideoRecodeFactory(srcFile, outFile);
    VideoOptHandler *handler = new RepeatMomentVideoOptHandler(factory, second,
                                                               TYPE_RELATIVE_MOMENT);
    int ret = factory->demuxerAudio();
    if (ret < 0) {
        return ret;
    }
    ret = factory->optVideo(handler);
    if (ret < 0) {
        return ret;
    }
    ret = factory->muxer(factory->outOptH264, factory->outAac, factory->outFile);
    delete (handler);
    delete (factory);
    return ret;
}

RepeatMomentVideoOptHandler::RepeatMomentVideoOptHandler(VideoRecodeFactory *recodeFactory,
                                                         double seconds, int type) {
    this->seconds = seconds;
    this->recodeFactory = recodeFactory;
    this->type = type;
}

int RepeatMomentVideoOptHandler::init(AVFormatContext *ifmt_ctx, AVCodecContext *decodecCtx,
                                      int video_index, int frameCount) {
    this->frameCount = frameCount;
    return 1;
}

int RepeatMomentVideoOptHandler::changeFrameRate(int frameRate) {
    int temp = this->seconds * frameRate;
    this->frameRate = frameRate;
    if (this->type == TYPE_REPEAT_MOMENT) {
        speed = frameRate / rate;
        relativeTime = 1;
    } else if (this->type == TYPE_RELATIVE_MOMENT) {
        relativeTimes = this->frameCount / frameRate / 3;
    }
    if (this->seconds <= 0) {
        this->startFrameIndex = temp;
        this->endFrameIndex = temp + frameRate * relativeTime;
    } else {
        this->startFrameIndex = temp - frameRate * relativeTime;
        this->endFrameIndex = temp;
    }


    return frameRate;
}

AllowEncodeAndWriteFrameState
RepeatMomentVideoOptHandler::allowEncodeAndWriteFrame(AVStream *inStream, AVPacket pkt,
                                                      int frameIndex) {
    if (this->type == TYPE_REPEAT_MOMENT) {
        if (frameIndex >= this->startFrameIndex && frameIndex < this->endFrameIndex) {
            if ((frameIndex - this->startFrameIndex) % speed == 0) {
                return STATE_NOT_ALLOW;
            } else {
                return STATE_ALLOW;

            }
        } else if (frameIndex == this->endFrameIndex) {
            int times = 0;
            const static int count = 3;
            while (times < count) {
                for (this->iterator = this->array.begin();
                     this->iterator != this->array.end(); ++this->iterator) {
                    AVFrame *avFrame = &(*this->iterator);
                    if (avFrame != NULL) {
                        this->recodeFactory->encode_write_frame(avFrame);
                    }
                    if (times == count - 1) {
                        av_frame_free(&avFrame);
                    }
                }
                times++;
            }

            return STATE_ALLOW;
        }
    } else if (this->type == TYPE_RELATIVE_MOMENT) {
        if (frameIndex < this->startFrameIndex || frameIndex >= this->endFrameIndex) {
            int left = this->frameCount - this->frameRate * relativeTime;
            int dump = relativeTimes * this->frameRate * relativeTime;
            double remove_frame_spit_d = (double) left / (double) dump;
            bool needReverse = false;
            if (remove_frame_spit_d < 2) {
                remove_frame_spit_d = (double) left / (double) (left - dump);
                needReverse = true;
            }
            int remove_frame_spit = (int) (remove_frame_spit_d + 0.5);
            if ((frameIndex % remove_frame_spit) == 0) {
                return needReverse ? STATE_ALLOW : STATE_NOT_ALLOW;
            }
            return needReverse ? STATE_NOT_ALLOW : STATE_ALLOW;

        }
    }

    return STATE_ALLOW;
}

AVFrame *RepeatMomentVideoOptHandler::optAvFrame(AVFrame *frame, int frameIndex) {
    if (this->type == TYPE_REPEAT_MOMENT) {
        if (frameIndex >= this->startFrameIndex && frameIndex < this->endFrameIndex) {
            if ((frameIndex - this->startFrameIndex) % speed != 0) {
                this->array.push_back(*av_frame_clone(frame));
            }
        }
    } else if (this->type == TYPE_RELATIVE_MOMENT) {
        if (frameIndex >= this->startFrameIndex && frameIndex < this->endFrameIndex) {
            int times = 0;
            while (times < relativeTimes) {
                this->recodeFactory->encode_write_frame(frame);
                times++;
            }
        }

    }

    return frame;
}

void RepeatMomentVideoOptHandler::releaseInEnd() {}

void RepeatMomentVideoOptHandler::changeTimebase(AVStream *inStream, AVStream *outStream,
                                                 AVPacket pkt) {}