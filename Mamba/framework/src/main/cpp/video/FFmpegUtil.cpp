//
// Created by jakechen on 2017/2/7.
//




#include "FFmpegUtil.h"

void copyAvStreamDict(AVStream *in, AVStream *out) {
    AVDictionaryEntry *tag = NULL;
    tag = av_dict_get(in->metadata, "rotate", tag, 0);
    if(tag!=NULL){
        av_dict_set(&out->metadata, tag->key, tag->value, 0);
    }

    LOGD("copyAvStreamDict %s",tag->key);
    LOGD("copyAvStreamDict %s",tag->value);
}

AVFrame *decodec_video_frame(AVCodecContext *pCodecCtx, AVPacket pkt) {
    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        return NULL;
    }
//的作用是解码一帧视频数据。输入一个压缩编码的结构体AVPacket，输出一个解码后的结构体AVFrame。该函数的声明位于libavcodec\avcodec.h
    int result = avcodec_send_packet(pCodecCtx, &pkt);
    if (result < 0 && result != AVERROR(EAGAIN) && result != AVERROR_EOF) {
        LOGI("decodec_video_frame()向解码器发送数据失败");
        av_frame_free(&frame);
        return NULL;
    }
    //从解码器返回解码输出数据
    result = avcodec_receive_frame(pCodecCtx, frame);
    if (result < 0 && result != AVERROR_EOF) {
        LOGI("decodec_video_frame()解码数据失败");
        av_frame_free(&frame);
        return NULL;
    }
    return frame;
}


AVCodecContext *open_decodec_context(AVStream *avStream) {
    if (avStream == NULL) {
        return NULL;
    }
    AVCodecParameters *pAVCodecParameters = avStream->codecpar;
    //根据编解码信息查找解码器
    AVCodec *pCodec = avcodec_find_decoder(pAVCodecParameters->codec_id);
    if (pCodec == NULL) {
        LOGI("open_decodec_context()无法根据avstream找到decoder");
        return NULL;
    }

    AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);
    if ((avcodec_parameters_to_context(pCodecCtx, pAVCodecParameters)) < 0) {
        LOGI("open_decodec_context()无法根据pCodec分配AVCodecContext");
        return NULL;
    }
//打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGI("open_decodec_context()无法打开编码器");
        return NULL;
    }
    return pCodecCtx;
}

AVCodecContext *applay_common_encoder(AVPixelFormat format, AVCodecID codecId, int width,
                                      int height) {
    AVCodecContext *pCodecCtx = NULL;
    //获取文件描述的上下文、
    AVCodec *pCodec = avcodec_find_encoder(codecId);
    if (pCodec == NULL) {
        LOGD("Can not find encoder! \n");
        return NULL;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (pCodecCtx == NULL) {
        LOGD("can not  find AVCodecContext! \n");
        return NULL;
    }

    pCodecCtx->pix_fmt = format;
    pCodecCtx->bit_rate_tolerance = 40 * 10000;
    pCodecCtx->width = width;
    pCodecCtx->height = height;
//    pCodecCtx->bit_rate =400000;
    pCodecCtx->bit_rate = width * height * 25 * 0.08 + 128;
    pCodecCtx->gop_size = 25;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;
    pCodecCtx->qmin = 10;
    pCodecCtx->qmax = 51;
    pCodecCtx->max_b_frames = 0;
    av_opt_set(pCodecCtx->priv_data, "preset", "ultrafast", 0);
    av_opt_set(pCodecCtx->priv_data, "tune", "zerolatency", 0);
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGD("Failed to open encoder! \n");
        return NULL;
    }
    return pCodecCtx;
}

AVCodecContext *applay_encoder_form_decoder(AVStream *avStream, int frameRate) {
    AVCodecContext *pCodecCtx = NULL;
    //获取文件描述的上下文、
    AVCodec *pCodec = avcodec_find_encoder(avStream->codecpar->codec_id);
    if (pCodec == NULL) {
        LOGD("Can not find encoder! \n");
        return NULL;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (pCodecCtx == NULL) {
        LOGD("can not  find AVCodecContext! \n");
        return NULL;
    }
    pCodecCtx->sample_aspect_ratio = avStream->codec->sample_aspect_ratio;
    pCodecCtx->codec_type = avStream->codecpar->codec_type;
    pCodecCtx->pix_fmt = avStream->codec->pix_fmt;
    pCodecCtx->bit_rate_tolerance = avStream->codec->bit_rate_tolerance;
    pCodecCtx->width = avStream->codecpar->width;

    pCodecCtx->height = avStream->codecpar->height;
    pCodecCtx->bit_rate = avStream->codec->bit_rate;
    pCodecCtx->gop_size = avStream->codec->gop_size;
    pCodecCtx->time_base = avStream->codec->time_base;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = frameRate;
    pCodecCtx->qmin = avStream->codec->qmin;
    pCodecCtx->qmax = avStream->codec->qmax;
    pCodecCtx->max_b_frames = avStream->codec->max_b_frames;
    LOGD("avStream->codec->qmin=%d", avStream->codec->qmin);
    LOGD("avStream->codec->qmax=%d", avStream->codec->qmax);
    LOGD(" avStream->codec->sample_aspect_ratio=%d", avStream->codec->sample_aspect_ratio);
    LOGD(" avStream->codec->bit_rate_tolerance=%d", avStream->codec->bit_rate_tolerance);
    LOGD(" avStream->codecpar->width=%d", avStream->codecpar->width);
    LOGD(" avStream->codecpar->height=%d", avStream->codecpar->height);
    LOGD(" avStream->codec->bit_rate=%d", avStream->codec->bit_rate);
    LOGD("avStream->codec->max_b_frames=%d", avStream->codec->max_b_frames);
    av_opt_set(pCodecCtx->priv_data, "preset", "ultrafast", 0);
    av_opt_set(pCodecCtx->priv_data, "tune", "zerolatency", 0);
    int ret = avcodec_open2(pCodecCtx, pCodec, NULL);
    if (ret < 0) {
        LOGD("Failed to open encoder! ret=%d", ret);
        return NULL;
    }
    return pCodecCtx;
}

int encodec_video_frame(AVCodecContext *codecContext, AVFrame *avFrame, AVPacket *avPacket) {
    int ret = avcodec_send_frame(codecContext, avFrame);
    if (ret != 0) {
        LOGD("avcodec_send_frame fail,ret=%d", ret);
        return ret;
    }
    ret = avcodec_receive_packet(codecContext, avPacket);
    return ret;
}

void flushEncoder(AVFormatContext *ofmt_ctx, AVCodecContext *ecodecCtx) {
    while (1) {
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = NULL; // packet data will be allocated by the encoder
        pkt.size = 0;
        int ret = encodec_video_frame(ecodecCtx, NULL, &pkt);
        if (ret < 0) {
            av_packet_unref(&pkt);
            break;
        }
        int videoIndex = av_find_best_stream(ofmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
        pkt.stream_index = videoIndex;
        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0) {
            LOGE("Error muxing packet\n");
        }
        av_packet_unref(&pkt);
    }
}

void flushEncoder(FILE *fp_out, AVCodecContext *ecodecCtx) {
    while (1) {
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = NULL; // packet data will be allocated by the encoder
        pkt.size = 0;
        int ret = encodec_video_frame(ecodecCtx, NULL, &pkt);
        if (ret < 0) {
            av_packet_unref(&pkt);
            break;
        }
        fwrite(pkt.data, 1, pkt.size, fp_out);
        av_packet_unref(&pkt);
    }
}

void setVideoEncoderParams(AVCodecContext *pCodecCtx, int width, int height) {
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    pCodecCtx->codec_tag = 0;
    pCodecCtx->bit_rate_tolerance = 40 * 10000;
    pCodecCtx->width = width;
    pCodecCtx->height = height;
    pCodecCtx->bit_rate = 3 * 1024 * 1024;
    pCodecCtx->gop_size = 250;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;
    pCodecCtx->qmin = 10;
    pCodecCtx->qmax = 51;
    pCodecCtx->max_b_frames = 3;
    av_opt_set(pCodecCtx->priv_data, "preset", "ultrafast", 0);
    av_opt_set(pCodecCtx->priv_data, "tune", "zerolatency", 0);
}

/**
yyyy yyyy
uv    uv
->
yyyy yyyy
uu
vv
*/
void yuv420sp_to_yuv420p(unsigned char *yuv420sp, unsigned char *yuv420p, int width, int height) {
    int i, j;
    int y_size = width * height;

    unsigned char *y = yuv420sp;
    unsigned char *uv = yuv420sp + y_size;

    unsigned char *y_tmp = yuv420p;
    unsigned char *u_tmp = yuv420p + y_size;
    unsigned char *v_tmp = yuv420p + y_size * 5 / 4;

    // y
    memcpy(y_tmp, y, y_size);

    // u
    for (j = 0, i = 0; j < y_size / 2; j += 2, i++) {
        u_tmp[i] = uv[j];
        v_tmp[i] = uv[j + 1];
    }
}

/**
yyyy yyyy
uu
vv
->
yyyy yyyy
uv    uv
*/
void yuv420p_to_yuv420sp(unsigned char *yuv420p, unsigned char *yuv420sp, int width, int height) {
    int i, j;
    int y_size = width * height;

    unsigned char *y = yuv420p;
    unsigned char *u = yuv420p + y_size;
    unsigned char *v = yuv420p + y_size * 5 / 4;

    unsigned char *y_tmp = yuv420sp;
    unsigned char *uv_tmp = yuv420sp + y_size;

    // y
    memcpy(y_tmp, y, y_size);

    // u
    for (j = 0, i = 0; j < y_size / 2; j += 2, i++) {
        // 此处可调整U、V的位置，变成NV12或NV21
#if 01
        uv_tmp[j] = u[i];
        uv_tmp[j + 1] = v[i];
#else
        uv_tmp[j] = v[i];
        uv_tmp[j+1] = u[i];
#endif
    }
}

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

AVFrame *yuv_to_avframe(AVCodecContext *pCodecCtx, int pts, uint8_t *y, uint8_t *u, uint8_t *v) {
    AVFrame *pFrame = av_frame_alloc();
    if (!pFrame) {
        LOGE("Could not allocate video frame\n");
        return NULL;
    }
    if (pCodecCtx == NULL) {
        LOGE("AVCodecContext is null");
        return NULL;
    }
    pFrame->format = pCodecCtx->pix_fmt;
    pFrame->width = pCodecCtx->width;
    pFrame->height = pCodecCtx->height;
    int ret = av_image_alloc(pFrame->data, pFrame->linesize, pCodecCtx->width,
                             pCodecCtx->height, pCodecCtx->pix_fmt, 16);
    if (ret < 0) {
        LOGE("Could not allocate raw picture buffer\n");
        return NULL;
    }
    pFrame->data[0] = y;  //PCM Data
    pFrame->data[1] = u;      // U
    pFrame->data[2] = v;  // V
    //PTS
    pFrame->pts = pts;
    return pFrame;
}

AVFrame *swsFrame(AVFrame *orgFrame, AVPixelFormat srcFormat, AVPixelFormat dest) {
    int width = orgFrame->width;
    int height = orgFrame->height;
    AVFrame *pFrameYUV;
    pFrameYUV = av_frame_alloc();
    uint8_t *out_buffer;
    out_buffer = new uint8_t[avpicture_get_size(dest, width, height)];
    avpicture_fill((AVPicture *) pFrameYUV, out_buffer, dest, width, height);
    SwsContext *img_convert_ctx = sws_getContext(width, height, srcFormat, width, height, dest,
                                                 SWS_BICUBIC, NULL, NULL, NULL);
    sws_scale(img_convert_ctx, (const uint8_t *const *) orgFrame->data, orgFrame->linesize, 0,
              height, pFrameYUV->data, pFrameYUV->linesize);

    return pFrameYUV;
}