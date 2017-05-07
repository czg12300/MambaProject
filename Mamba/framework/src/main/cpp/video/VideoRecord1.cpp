//
// Created by jakechen on 2017/2/10.
//
//
//
#include "VideoRecord1.h"

static AVCodecContext *pCodecCtx;
static AVCodec *pCodec;
static AVFrame *frame;
static AVPacket pkt;
static int y_size;
static int frameCount = 0;
static const AVCodecID codecId = AV_CODEC_ID_H264;
static string tempH264;
static string outFile;
static AVFormatContext *fmt_ctx = NULL;
static int video_index = -1;
static AVStream *stream;
static int y_length;
static int uv_length;
static string rotate;

static int setEncoder(int width, int height, int frameRate, int64_t bitRate);


int videoRecordStart1(const char *file, int width, int height, const char *rotate_, int frameRate,
                      int64_t bitRate) {
    rotate = rotate_;
    tempH264 = file;
    tempH264 += ".h264";
    outFile = file;
    av_register_all();
    int ret = setEncoder(width, height, frameRate, bitRate);
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
    y_size = pCodecCtx->width * pCodecCtx->height;
    frameCount = 0;
    frame = av_frame_alloc();
    if (!frame) {
        LOGD("无法分配Video Frame\n");
        return -1;
    }
    frame->format = pCodecCtx->pix_fmt;
    frame->width = pCodecCtx->width;
    frame->height = pCodecCtx->height;
    ret = av_image_alloc(frame->data, frame->linesize, pCodecCtx->width,
                         pCodecCtx->height, pCodecCtx->pix_fmt, 16);
    y_length = width * height;
    uv_length = width * height / 4;
    if (ret < 0) {
        LOGD("Could not allocate raw picture buffer\n");
        return -1;
    }
    ret = 1;
    return ret;
}

int videoRecording1(uint8_t *data) {
    //安卓摄像头数据为NV21格式，此处将其转换为YUV420P格式
    memcpy(frame->data[0], data, y_length);
    for (int i = 0; i < uv_length; i++) {
        *(frame->data[2] + i) = *(data + y_length + i * 2);
        *(frame->data[1] + i) = *(data + y_length + i * 2 + 1);
    }

//    frame->data[0] = data;  //PCM Data
//    frame->data[1] = (data + y_size * 5 / 4);      // U
//    frame->data[2] = (data + y_size);  // V
    //PTS
    frame->pts = frameCount;
    //Encode
    int ret = avcodec_send_frame(pCodecCtx, frame);
    if (ret < 0) {
        LOGE("Failed to avcodec_send_frame! \n");
        return -1;
    }
    av_init_packet(&pkt);
    pkt.data = NULL; // packet data will be allocated by the encoder
    pkt.size = 0;
    ret = avcodec_receive_packet(pCodecCtx, &pkt);
    if (ret < 0) {
        LOGE("Failed to avcodec_receive_packet! \n");
        return -1;
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
        return -1;
    }
    frameCount++;
    av_packet_unref(&pkt);
    return 1;
}


int videoRecordEnd1() {
    //Flush Encoder
    flushEncoder(fmt_ctx, pCodecCtx);
    //写文件尾（Write file trailer）
    av_write_trailer(fmt_ctx);
    frameCount = 0;
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);
    av_frame_free(&frame);
    avio_closep(&fmt_ctx->pb);
    avformat_close_input(&fmt_ctx);
    avformat_free_context(fmt_ctx);
    LOGD("rotate=%s", rotate.c_str());
    int ret = h264ToFormat(tempH264.c_str(), outFile.c_str(), rotate.c_str());
    if (ret >= 0) {
        remove(tempH264.c_str());
    }
    return ret;
}

static int setEncoder(int width, int height, int frameRate, int64_t bitRate) {
    pCodec = avcodec_find_encoder(codecId);
    if (pCodec == NULL) {
        LOGD("Can not find encoder! \n");
        return -1;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (pCodecCtx == NULL) {
        LOGD("can not  find AVCodecContext! \n");
        return -1;
    }
    setVideoEncoderParams(pCodecCtx, width, height);
    pCodecCtx->time_base.den = frameRate;
    pCodecCtx->bit_rate = bitRate;
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGD("Failed to open encoder! \n");
        return -1;
    }
    LOGD("open encoder success");
    return 1;
}