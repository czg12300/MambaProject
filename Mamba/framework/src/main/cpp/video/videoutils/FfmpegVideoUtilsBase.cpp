//
// Created by jakechen on 2017/5/12.
//

#include "FfmpegVideoUtilsBase.h"

namespace video {

    OutputStreamContext *
    applyOutputStreamContext(AVFormatContext *fmt_ctx, const char *file, AVMediaType type) {
        OutputStreamContext *os_ctx = (OutputStreamContext *) malloc(sizeof(OutputStreamContext));
        //获取文件描述的上下文
        avformat_alloc_output_context2(&os_ctx->fmt_ctx, NULL, NULL, file);
        if (!os_ctx->fmt_ctx) {
            LOGE("Could not create output context\n");
            free(os_ctx);
            return NULL;
        }
        os_ctx->streamIndex = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
        if (os_ctx->streamIndex == -1) {
            LOGD("can't find video stream in %s\n", fmt_ctx->filename);
            free(os_ctx);
            return NULL;
        }
        AVCodec *codec = avcodec_find_decoder(
                fmt_ctx->streams[os_ctx->streamIndex]->codecpar->codec_id);
        if (codec == NULL) {
            LOGD("input codec is null");
            avformat_free_context(os_ctx->fmt_ctx);
            free(os_ctx);
            return NULL;
        }
        os_ctx->stream = avformat_new_stream(os_ctx->fmt_ctx, codec);

        if (os_ctx->stream == NULL) {
            LOGD("avformat_new_stream is null");
            avformat_free_context(os_ctx->fmt_ctx);
            free(os_ctx);
            return NULL;
        }
        int ret = avcodec_parameters_copy(os_ctx->stream->codecpar,
                                          fmt_ctx->streams[os_ctx->streamIndex]->codecpar);
        if (ret < 0) {
            LOGD("avformat_new_stream is null");
            avformat_free_context(os_ctx->fmt_ctx);
            free(os_ctx);
            return NULL;
        }
        //打开输出文件（Open output file）
        if (avio_open(&os_ctx->fmt_ctx->pb, file, AVIO_FLAG_READ_WRITE) < 0) {
            LOGD("Couldn't open output file.");
            avio_close(os_ctx->fmt_ctx->pb);
            avformat_free_context(os_ctx->fmt_ctx);
            free(os_ctx);
            return NULL;
        }
        //写文件头（Write file header）
        if (avformat_write_header(os_ctx->fmt_ctx, NULL) < 0) {
            LOGD("Error occurred when opening video output file.");
            avio_close(os_ctx->fmt_ctx->pb);
            avformat_free_context(os_ctx->fmt_ctx);
            free(os_ctx);
            return NULL;
        }
        return os_ctx;
    }

}