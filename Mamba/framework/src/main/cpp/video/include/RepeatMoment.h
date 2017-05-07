//
// Created by jakechen on 2017/3/20.
//
#include "VideoOptHandler.h"
#include "FFmpegBase.h"
#include "VideoRecodeFactory.h"
#include "list"

#ifndef XIUGE_REPEATMOMENT_H
#define XIUGE_REPEATMOMENT_H 
static const int TYPE_REPEAT_MOMENT = 0;
static const int TYPE_RELATIVE_MOMENT = 1;

class RepeatMomentVideoOptHandler : public VideoOptHandler {
private:
    double seconds;
    int startFrameIndex;
    int endFrameIndex;
    typedef list<AVFrame> LIST;
    LIST::iterator iterator;
    LIST array;
    int frameCount=0;
    int frameRate=0;
    int type = TYPE_REPEAT_MOMENT;
    VideoRecodeFactory *recodeFactory;
public:
    RepeatMomentVideoOptHandler(VideoRecodeFactory *recodeFactory, double seconds,int type);

    int
    init(AVFormatContext *ifmt_ctx, AVCodecContext *decodecCtx, int video_index, int frameCount);

    AVFrame *optAvFrame(AVFrame *frame, int frameIndex);

    int changeFrameRate(int frameRate);

    AllowEncodeAndWriteFrameState
    allowEncodeAndWriteFrame(AVStream *inStream, AVPacket pkt, int frameIndex);

    void changeTimebase(AVStream *inStream, AVStream *outStream, AVPacket pkt);

    void releaseInEnd();

};

#endif //XIUGE_REPEATMOMENT_H

/**
 * 鬼畜效果
 * @param srcFile
 * @param outFile
 * @param second 鬼畜时间节点
 * @return
 */
int repeatMoment(string srcFile, string outFile, double second);


/**
 * 相对论效果
 * @param srcFile
 * @param outFile
 * @param second
 * @return
 */
int relativeMoment(string srcFile, string outFile, double second);