//
// Created by jakechen on 2017/3/16.
//
#include "VideoRecodeFactory.h"
#include "audioRate.h"
#include "BeautyAvFrame.h"

#ifndef XIUGE_TIMEMACHINE_H
#define XIUGE_TIMEMACHINE_H
static const int TYPE_FAST = 1;
static const int TYPE_SLOW = 2;
static const int MAX = 100;


class OptVideoProgressCallback : public ProgressCallback {
public:
    int *progress;
    int *ret;
    int total;

    OptVideoProgressCallback(int *progress, int *ret, int total);

    void onProgress(int progress, int total);

    void onFinish(int progress, int total);
};

class OptAudioProgressCallback : public ProgressCallback {
public:
    int *progress;
    int *ret;
    int total;

    OptAudioProgressCallback(int *progress, int *ret, int total);

    void onProgress(int progress, int total);

    void onFinish(int progress, int total);
};

class TimeMachineVideoOptHandler : public VideoOptHandler {
private:
    long duration;
    int frameCount;
    AVCodecContext *codecContext;
    OptVideoProgressCallback *callback;
    VideoRecodeFactory *factory;
    AVFrame *rgbFrame;
public:
    int type;
    int rate;

    TimeMachineVideoOptHandler(VideoRecodeFactory *factory, OptVideoProgressCallback *callback,
                               int type, int rate);

    int
    init(AVFormatContext *ifmt_ctx, AVCodecContext *decodecCtx, int video_index, int frameCount);

    AVFrame *optAvFrame(AVFrame *frame, int frameIndex);

    int changeFrameRate(int frameRate);

    AllowEncodeAndWriteFrameState
    allowEncodeAndWriteFrame(AVStream *inStream, AVPacket pkt, int frameIndex);

    void releaseInEnd();

    void changeTimebase(AVStream *inStream, AVStream *outStream, AVPacket pkt);
};

struct OptVideoData {
    OptVideoProgressCallback *callback;
    VideoOptHandler *handler;
    VideoRecodeFactory *factory;
};
struct OptAudioData {
    OptAudioProgressCallback *callback;
    VideoRecodeFactory *factory;
    TimeMachineVideoOptHandler *handler;
};

#endif //XIUGE_TIMEMACHINE_H

/**
 * 视频加速
 * @param srcFile
 * @param outFile
 * @param rate
 * @return
 */
int fastVideo(string srcFile, string outFile, int rate, HandleProgressCallback *callback);


/**
 * 视频减速
 * @param srcFile
 * @param outFile
 * @param rate
 * @return
 */
int slowVideo(string srcFile, string outFile, int rate, HandleProgressCallback *callback);