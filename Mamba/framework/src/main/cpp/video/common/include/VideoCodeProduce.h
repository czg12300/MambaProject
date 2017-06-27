//
// Created by jakechen on 2017/6/3.
//
#ifndef SG_VIDEOCODEPRODUCE_H
#define SG_VIDEOCODEPRODUCE_H

#include "FFmpegBase.h"
#include "ThreadExecutor.h"
#include "ThreadPool.h"
#include <queue>
#include <BaseVideoEffect.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
}

namespace video {
    struct VideoCodeParams {
        string srcFile;
        string outFile;
        long rangeStart;
        long rangeEnd;
        BaseVideoEffect *effect;
    };


    class VideoCodeProduce {
    private:
        VideoCodeParams *params;
        queue<AVPacket> *srcPacketQueue = NULL;
        queue<AVPacket> *outPacketQueue = NULL;
        queue<DecodeFrame *> *decodeFrameQueue = NULL;
        long seekTime = -1;
        int video_index = -1;
        int audio_index = -1;
        int frame_rate = 0;
        volatile bool readFinish = false;
        bool isSuccess = false;
        bool isInitSuccess = false;
        bool isStop = false;
        pthread_t *threadIds;
        AVFormatContext *ifmt_ctx = NULL;
        AVFormatContext *ofmt_ctx = NULL;
        AVCodecContext *decode_ctx = NULL;
        AVCodecContext *encode_ctx = NULL;
        AVStream *in_stream;
        AVStream *out_stream;
        int64_t *dts_start_from;
        int64_t *pts_start_from;
        double lastFrameTime=0;
        pthread_mutex_t *mutex_src_package;
        pthread_mutex_t *mutex_decode_frame;
        pthread_mutex_t *mutex_out_package;


        void init();

        void release();

    public:
        bool hasAudioStream();

        void readPackage();

        void decodePackage();

        void encodeFrame();

        void writeFile();

        VideoCodeProduce(VideoCodeParams *params);

        ~VideoCodeProduce();

        void start();
    };

    struct ThreadExecuteParams {
        int methodId;
        VideoCodeProduce *produce;
    };

    static const int METHOD_ID_READ = 1;
    static const int METHOD_ID_DECODE = 2;
    static const int METHOD_ID_ENCODE = 3;
    static const int METHOD_ID_WRITE = 4;

    static void *threatExecute(void *arg);
}


#endif