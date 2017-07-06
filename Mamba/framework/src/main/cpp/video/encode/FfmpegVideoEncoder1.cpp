//
// Created by jakechen on 2017/6/17.
//

#include "FfmpegVideoEncoder1.h"

namespace video {
    static const int CODEC_TYPE_H264 = 1;
    static const int CODEC_TYPE_H265 = 2;
    static AVCodecContext *pCodecCtx = NULL;
    static AVStream *stream = NULL;
    static AVFormatContext *fmt_ctx = NULL;
    volatile bool isStop = true;
    static int frameCount = 0;
    static int mWidth;
    static int mHeight;

    static void flushEncoder();


    static int initEncoder(AVCodec *codec, int width, int height, int bitRate, int frameRate,
                           int keyIFrameInterval);

    AVCodecID getCodeIdByType(int type) {
        AVCodecID id = AV_CODEC_ID_H264;
        switch (type) {
            case CODEC_TYPE_H264:
                id = AV_CODEC_ID_H264;
                break;
            case CODEC_TYPE_H265:
                id = AV_CODEC_ID_H265;
                break;
        }
        return id;
    }


    void encode(uint8_t *data, int width, int height) {
        if (isStop) {
            return;
        }
        LOGI("encode encode");
        AVFrame *frame = av_frame_alloc();
        if (!frame) {
            LOGI("无法分配Video Frame\n");
            return;
        }
        LOGI("encode frameYuv\n");
        avpicture_fill((AVPicture *) frame, data, AV_PIX_FMT_YUV420P, mWidth, mHeight);
        frame->pts = frameCount++;
        LOGI("encode frameCount = %d\n", frameCount);
        int ret = avcodec_send_frame(pCodecCtx, frame);
        if (ret < 0) {
            LOGE("Failed to avcodec_send_frame! \n");
            av_freep(&frame->data[0]);
            av_frame_unref(frame);
            av_frame_free(&frame);
            return;
        }
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = NULL; // packet data will be allocated by the encoder
        pkt.size = 0;
        ret = avcodec_receive_packet(pCodecCtx, &pkt);
        if (ret < 0) {
            LOGE("Failed to avcodec_receive_packet! \n");
            av_freep(&frame->data[0]);
            av_frame_unref(frame);
            av_frame_free(&frame);
            av_packet_unref(&pkt);
            return;
        }
        pkt.pts = av_rescale_q_rnd(pkt.pts, pCodecCtx->time_base,
                                   stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF |
                                                 AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, pCodecCtx->time_base,
                                   stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF |
                                                 AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, pCodecCtx->time_base,
                                    stream->time_base);
        pkt.pos = -1;
        pkt.stream_index = stream->index;
        if (av_interleaved_write_frame(fmt_ctx, &pkt) < 0) {
            av_packet_unref(&pkt);
            av_freep(&frame->data[0]);
            av_frame_unref(frame);
            av_frame_free(&frame);
            LOGE("Error muxing packet\n");
            return;
        }
        LOGD("av_interleaved_write_frame  success");
        av_packet_unref(&pkt);
        av_freep(&frame->data[0]);
        av_frame_unref(frame);
        av_frame_free(&frame);
        frame = NULL;

        LOGI("encode end\n");
    }

    void stopEncode() {
        isStop = true;
        flushEncoder();
        av_write_trailer(fmt_ctx);
        frameCount = 0;
        if (fmt_ctx && !(fmt_ctx->flags & AVFMT_NOFILE)) {
            avio_closep(&fmt_ctx->pb);
        }
        avcodec_free_context(&pCodecCtx);
        avformat_free_context(fmt_ctx);
        stream = NULL;
        pCodecCtx = NULL;
        fmt_ctx = NULL;
    }

    static void flushEncoder() {
        while (true) {
            int ret = avcodec_send_frame(pCodecCtx, NULL);
            if (ret < 0) {
                LOGE("Failed to avcodec_send_frame! \n");
                break;
            }
            AVPacket pkt;
            av_init_packet(&pkt);
            pkt.data = NULL; // packet data will be allocated by the encoder
            pkt.size = 0;
            ret = avcodec_receive_packet(pCodecCtx, &pkt);
            if (ret < 0) {
                LOGE("Failed to avcodec_receive_packet! \n");
                break;
            }
            pkt.pts = av_rescale_q_rnd(pkt.pts, pCodecCtx->time_base, stream->time_base,
                                       (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pkt.dts = av_rescale_q_rnd(pkt.dts, pCodecCtx->time_base, stream->time_base,
                                       (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pkt.duration = av_rescale_q(pkt.duration, pCodecCtx->time_base, stream->time_base);
            pkt.pos = -1;
            pkt.stream_index = stream->index;
            if (av_interleaved_write_frame(fmt_ctx, &pkt) < 0) {
                av_packet_unref(&pkt);
                LOGE("Error muxing packet\n");
                return;
            }
            av_packet_unref(&pkt);
        }
    }

    int
    startEncode(const char *file, int codecType, int keyIFrameInterval, int width,
                int height, int frameRate,
                int bitRate) {
        mWidth = width;
        mHeight = height;
        isStop = false;
        int ret = -1;
        fmt_ctx = NULL;
        av_register_all();
        AVCodec *pCodec = avcodec_find_encoder(getCodeIdByType(codecType));
        ret = initEncoder(pCodec, width, height, bitRate, frameRate, keyIFrameInterval);
        if (ret < 0) {
            return ret;
        }
        ret = avformat_alloc_output_context2(&fmt_ctx, NULL, NULL, file);
        if (!fmt_ctx || ret < 0) {
            LOGE("Could not create output context\n");
            return ret;
        }
        if (fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER) {
            pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
        }
        stream = avformat_new_stream(fmt_ctx, pCodec);
        AVCodecParameters *par = avcodec_parameters_alloc();
        ret = avcodec_parameters_from_context(par, pCodecCtx);
        if (ret < 0) {
            LOGD("avcodec_parameters_from_context fail");
            return ret;
        }
        ret = avcodec_parameters_copy(stream->codecpar, par);
        if (ret < 0) {
            LOGD("avcodec_parameters_copy fail");
            return ret;
        }
        //打开输出文件（Open output file）
        ret = avio_open(&fmt_ctx->pb, file, AVIO_FLAG_READ_WRITE);
        if (ret < 0) {
            LOGD("Couldn't open output file.");
            return ret;
        }
        if (fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;

        //写文件头（Write file header）
        ret = avformat_write_header(fmt_ctx, NULL);
        if (ret < 0) {
            LOGD("Error occurred when opening video output file.");
            avio_close(fmt_ctx->pb);
            avformat_free_context(fmt_ctx);
            return ret;
        }
        frameCount = 0;
        return ret;
    }

    int
    initEncoder(AVCodec *pCodec, int width, int height, int bitRate, int frameRate,
                int keyIFrameInterval) {
        //获取文件描述的上下文、
        int ret = -1;
        if (pCodec == NULL) {
            LOGD("Can not find encoder! \n");
            return ret;
        }
        pCodecCtx = avcodec_alloc_context3(pCodec);
        if (pCodecCtx == NULL) {
            LOGD("can not  find AVCodecContext! \n");
            return ret;
        }
        pCodecCtx->codec_tag = 0;
        pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
        pCodecCtx->bit_rate_tolerance = bitRate;
        pCodecCtx->width = width;
        pCodecCtx->height = height;
        pCodecCtx->bit_rate = bitRate;
        pCodecCtx->gop_size = frameRate;
        pCodecCtx->time_base.num = 1;
        pCodecCtx->time_base.den = frameRate;
        pCodecCtx->qmin = 10;
        pCodecCtx->qmax = 51;
        av_opt_set(pCodecCtx->priv_data, "preset", "ultrafast", 0);
        av_opt_set(pCodecCtx->priv_data, "tune", "zerolatency", 0);
        ret = avcodec_open2(pCodecCtx, pCodec, NULL);
        if (ret < 0) {
            LOGD("Failed to open encoder! \n");
            return ret;
        }
        return ret;
    }

}