//
// Created by jakechen on 2017/2/10.
//


#include "VideoRecord.h"

static AVCodecContext *pCodecCtx;
static AVCodec *pCodec;
AVFrame *frame;
AVPacket pkt;
int y_size;
static int frameCount = 0;
static const AVCodecID codecId = AV_CODEC_ID_H264;
FILE *fp_out;


int videoRecordStart(const char *h264File, int width, int height) {
    LOGD("videoRecordStart! \n");
    av_register_all();
    //获取文件描述的上下文、
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
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
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
        return -1;
    }
    if ((fp_out = fopen(h264File, "wb")) == NULL) {
        LOGD("open file fail");
        return -1;
    }
    y_size = pCodecCtx->width * pCodecCtx->height;
    frameCount=0;
    frame = av_frame_alloc();
    if(!frame){
        LOGI("无法分配Video Frame\n");
        return -1;
    }
    frame->format = pCodecCtx->pix_fmt;
    frame->width = pCodecCtx->width;
    frame->height = pCodecCtx->height;
    int ret = av_image_alloc(frame->data, frame->linesize, pCodecCtx->width,
                             pCodecCtx->height, pCodecCtx->pix_fmt, 16);
    if (ret < 0) {
        LOGE("Could not allocate raw picture buffer\n");
        return -1;
    }
    return 1;
}

int videoRecording(uint8_t *data) {
    frame->data[0] = data;  //PCM Data
    frame->data[1] = (data + y_size * 5 / 4);      // U
    frame->data[2] = (data + y_size);  // V
    //PTS
    frame->pts = frameCount;
    int got_output = 0;
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
    frameCount++;
    fwrite(pkt.data, 1, pkt.size, fp_out);
    av_packet_unref(&pkt);
    return 1;
}

int videoRecordEnd() {
    //Flush Encoder
    while (1) {
        int ret = avcodec_send_frame(pCodecCtx, NULL);
        if (ret < 0) {
            LOGE("Failed to avcodec_send_frame! \n");
            break;
        }
        av_init_packet(&pkt);
        pkt.data = NULL; // packet data will be allocated by the encoder
        pkt.size = 0;
        ret = avcodec_receive_packet(pCodecCtx, &pkt);
        if (ret < 0) {
            LOGE("Failed to avcodec_receive_packet! \n");
            break;
        }
        fwrite(pkt.data, 1, pkt.size, fp_out);
        av_free_packet(&pkt);
    }
    frameCount=0;
    fclose(fp_out);
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
    return 1;
}