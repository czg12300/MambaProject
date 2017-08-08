//
// Created by jakechen on 2017/7/22.
//

#ifndef MAMBA_FFMPEGCODEC_H
#define MAMBA_FFMPEGCODEC_H

#include "MediaFormat.h"
#include "Log.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
}
namespace video {
    class FfmpegCodec {
    private:
        MediaFormat *format;
        AVCodecContext *pCodecCtx;
        AVCodecParserContext *pCodecParserCtx;
        MediaFormatKey *mf_key;
        int frameIndex = 0;

        void initDecoder();

        void initEncoder();

        uint8_t *decode(uint8_t *src,int srcSize, int *len);

        uint8_t *encode(uint8_t *src,int srcSize, int *len);

        AVFrame *pktFrame(uint8_t *data);

    public:
        FfmpegCodec();

        void _configure(const char *key, int value);
//        void _configure(MediaFormat *format);

        bool isEncode();

        uint8_t *_decodeOrEncode(uint8_t *src,int srcSize, int *len);


        void start();

        void release();
    };
}


#endif //MAMBA_FFMPEGCODEC_H
