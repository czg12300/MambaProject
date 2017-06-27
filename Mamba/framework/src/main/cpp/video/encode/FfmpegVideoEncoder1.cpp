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
    static queue<AVFrame *> *frameQueue = NULL;
    static pthread_t *threadId = NULL;
    static pthread_mutex_t *mutex_frame = NULL;
    static string resultFile;
    static string tempH264;
    volatile bool isStop = true;
    static int frameCount = 0;
    static int mWidth;
    static int mHeight;

    static void flushEncoder();

    static void *threatExecute(void *arg);

    static struct ThreadExecuteArg {
        AVFormatContext *fmt_ctx;
        queue<AVFrame *> *frameQueue;
        AVCodecContext *pCodecCtx;
        AVStream *stream;
    };

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
        LOGD("encode frameQueue->size() =%d width=%d  height=%d", frameQueue->size(), width,
             height);
//        if (scxt == NULL) {
        SwsContext *scxt = sws_getContext(width, height, AV_PIX_FMT_BGRA, mWidth,
                                          mHeight,
                                          AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);
        if (!scxt) {
            LOGD("sourcePicture sws_getContext failed");
            return;
        }
//        }

        AVFrame *sourcePicture = av_frame_alloc();
        if (!sourcePicture) {
            LOGI("无法分配Video Frame\n");
            return;
        }
        AVFrame *frameYuv = av_frame_alloc();
        if (!frameYuv) {
            LOGI("无法分配Video Frame\n");
            return;
        }
        LOGI("encode frameYuv\n");
        av_image_alloc(frameYuv->data, frameYuv->linesize, pCodecCtx->width,
                       pCodecCtx->height, pCodecCtx->pix_fmt, 16);
        avpicture_fill((AVPicture *) sourcePicture, data, AV_PIX_FMT_BGRA,
                       width, height);
        sws_scale(scxt, (const uint8_t *const *) sourcePicture->data,
                  sourcePicture->linesize,
                  0, height, frameYuv->data, frameYuv->linesize);
        frameYuv->pts = frameCount++;
        LOGI("encode frameCount = %d\n", frameCount);
        pthread_mutex_lock(mutex_frame);
        frameQueue->push(frameYuv);
        pthread_mutex_unlock(mutex_frame);
        //av_freep(&sourcePicture->data[0]);
        av_frame_unref(sourcePicture);
        av_frame_free(&sourcePicture);
        sws_freeContext(scxt);

        sourcePicture = NULL;
        scxt = NULL;
        LOGI("encode end\n");
    }

    void stopEncode() {
        isStop = true;
        pthread_join(*threadId, NULL);
        delete (threadId);
        threadId = NULL;
        delete (frameQueue);
        frameQueue = NULL;
        pthread_mutex_destroy(mutex_frame);
        delete (mutex_frame);
        mutex_frame = NULL;
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
        h264ToFormat(tempH264.c_str(), resultFile.c_str());
        remove(tempH264.c_str());
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
        resultFile = file;
        tempH264 = resultFile + ".h264";
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
        ret = avformat_alloc_output_context2(&fmt_ctx, NULL, NULL, tempH264.c_str());
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
        ret = avio_open(&fmt_ctx->pb, tempH264.c_str(), AVIO_FLAG_READ_WRITE);
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
        frameQueue = new queue<AVFrame *>;
        frameCount = 0;
        mutex_frame = new pthread_mutex_t;
        pthread_mutex_init(mutex_frame, NULL);
        ThreadExecuteArg *arg = new ThreadExecuteArg;
        arg->fmt_ctx = fmt_ctx;
        arg->frameQueue = frameQueue;
        arg->pCodecCtx = pCodecCtx;
        arg->stream = stream;
        threadId = new pthread_t;
        pthread_create(threadId, NULL, threatExecute, arg);
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

    static void *threatExecute(void *arg) {
        if (arg != NULL) {
            ThreadExecuteArg *executeArg = (ThreadExecuteArg *) arg;
            while (!isStop) {
                if (executeArg->frameQueue->size() > 0) {
                    AVFrame *frame = NULL;
                    pthread_mutex_lock(mutex_frame);
                    frame = executeArg->frameQueue->front();
                    executeArg->frameQueue->pop();
                    pthread_mutex_unlock(mutex_frame);
                    //Encode
                    int ret = avcodec_send_frame(executeArg->pCodecCtx, frame);
                    if (ret < 0) {
                        LOGE("Failed to avcodec_send_frame! \n");
                        av_freep(&frame->data[0]);
                        av_frame_unref(frame);
                        av_frame_free(&frame);
                        continue;
                    }
                    AVPacket pkt;
                    av_init_packet(&pkt);
                    pkt.data = NULL; // packet data will be allocated by the encoder
                    pkt.size = 0;
                    ret = avcodec_receive_packet(executeArg->pCodecCtx, &pkt);
                    if (ret < 0) {
                        LOGE("Failed to avcodec_receive_packet! \n");
                        av_freep(&frame->data[0]);
                        av_frame_unref(frame);
                        av_frame_free(&frame);
                        av_packet_unref(&pkt);
                        continue;
                    }
                    pkt.pts = av_rescale_q_rnd(pkt.pts, executeArg->pCodecCtx->time_base,
                                               executeArg->stream->time_base,
                                               (AVRounding) (AV_ROUND_NEAR_INF |
                                                             AV_ROUND_PASS_MINMAX));
                    pkt.dts = av_rescale_q_rnd(pkt.dts, executeArg->pCodecCtx->time_base,
                                               executeArg->stream->time_base,
                                               (AVRounding) (AV_ROUND_NEAR_INF |
                                                             AV_ROUND_PASS_MINMAX));
                    pkt.duration = av_rescale_q(pkt.duration, executeArg->pCodecCtx->time_base,
                                                executeArg->stream->time_base);
                    pkt.pos = -1;
                    pkt.stream_index = executeArg->stream->index;
                    if (av_interleaved_write_frame(executeArg->fmt_ctx, &pkt) < 0) {
                        av_packet_unref(&pkt);
                        av_freep(&frame->data[0]);
                        av_frame_unref(frame);
                        av_frame_free(&frame);
                        LOGE("Error muxing packet\n");
                        continue;
                    }
                    av_packet_unref(&pkt);
                    av_freep(&frame->data[0]);
                    av_frame_unref(frame);
                    av_frame_free(&frame);
                    frame = NULL;
                } else {
                    usleep(30);
                }
            }
            delete (executeArg);
        }
        return NULL;
    }

}