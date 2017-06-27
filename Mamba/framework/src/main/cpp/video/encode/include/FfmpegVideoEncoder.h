//
// Created by jakechen on 2017/5/8.
//
#ifndef NEWSHOWGUYS_ANDROID_FFMPEGVIDEOENCODER_H
#define NEWSHOWGUYS_ANDROID_FFMPEGVIDEOENCODER_H

#include <queue>
#include "FfmpegMuxer.h"
#include "FFmpegBase.h"

extern "C" {
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
};
namespace video {

    class VideoEncoder {
    private:
        AVCodecContext *pCodecCtx;
        AVCodec *pCodec;
        int frameCount = 0;
        static const AVCodecID codecId = AV_CODEC_ID_H264;
        static const int CODEC_TYPE_H264 = 1;
        static const int CODEC_TYPE_H265 = 2;
        int mWidth;
        int mHeight;
//        SwsContext *scxt;
        AVStream *stream;
        AVFormatContext *fmt_ctx;
        string resultFile;
        string tempH264;
        queue<AVFrame *> *frameQueue;
        pthread_t *threadId;
        pthread_mutex_t *mutex_frame;
        int threadIdLen = 1;

        AVCodecID getCodeIdByType(int type);

        void flushEncoder();

        int initEncoder(AVCodecID codecID, int width, int height, int bitRate, int frameRate,
                        int keyIFrameInterval);

    public:
        volatile bool isStop = true;


        void encode();


        int start(const char *file, int codecType, int keyIFrameInterval, int width, int height,
                  int frameRate,
                  int bitRate);


        void encode(uint8_t *data, int width, int height);


        void stop();


    };

    static void *threatExecute(void *arg);


}


#endif //NEWSHOWGUYS_ANDROID_FFMPEGVIDEOENCODER_H
