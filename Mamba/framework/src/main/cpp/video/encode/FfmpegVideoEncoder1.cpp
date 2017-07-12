//
// Created by jakechen on 2017/6/17.
//

#include "FfmpegVideoEncoder1.h"

namespace video {
    static const int CODEC_TYPE_H264 = 1;
    static const int CODEC_TYPE_H265 = 2;
    static AVCodecContext *pCodecCtx = NULL;
    volatile bool isStop = true;
    static int frameCount = 0;
    static int mWidth;
    static int mHeight;
    static AVFrame *frame;
    FILE *fp_out;

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
        if (!frame) {
            frame = av_frame_alloc();
            if (!frame) {
                LOGI("无法分配Video Frame\n");
                return;
            }
        }
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
        while (true) {
            AVPacket pkt;
            av_init_packet(&pkt);
            pkt.data = NULL; // packet data will be allocated by the encoder
            pkt.size = 0;
            ret = avcodec_receive_packet(pCodecCtx, &pkt);
            if (ret < 0) {
                LOGE("Failed to avcodec_receive_packet! \n");
                break;
            }
            fwrite(pkt.data, 1, pkt.size, fp_out);
            av_packet_unref(&pkt);
        }


        LOGI("encode end\n");
    }

    void stopEncode() {
        isStop = true;
        fclose(fp_out);
        av_freep(&frame->data[0]);
        av_frame_unref(frame);
        av_frame_free(&frame);
        avcodec_close(pCodecCtx);
        avcodec_free_context(&pCodecCtx);
        pCodecCtx = NULL;
    }

    int
    startEncode(const char *file, int codecType, int keyIFrameInterval, int width,
                int height, int frameRate,
                int bitRate) {
        mWidth = width;
        mHeight = height;
        isStop = false;
        int ret = -1;
        av_register_all();
        AVCodec *pCodec = avcodec_find_encoder(getCodeIdByType(codecType));
        pCodecCtx = avcodec_alloc_context3(pCodec);
        if (pCodecCtx == NULL) {
            LOGD("can not  find AVCodecContext! \n");
            return -1;
        }
        pCodecCtx->codec_tag = 0;
        pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
        pCodecCtx->bit_rate_tolerance = 40 * 10000;
        pCodecCtx->width = width;
        pCodecCtx->height = height;
        pCodecCtx->bit_rate = bitRate;
        pCodecCtx->gop_size = keyIFrameInterval * frameRate;
        pCodecCtx->time_base.num = 1;
        pCodecCtx->me_pre_cmp = 1;
        pCodecCtx->max_b_frames = 0;
        pCodecCtx->time_base.den = frameRate;
        pCodecCtx->lumi_masking = 0.0;
        pCodecCtx->dark_masking = 0.0;
        pCodecCtx->qmin = 10;
        pCodecCtx->qmax = 51;
        av_opt_set(pCodecCtx->priv_data, "preset", "ultrafast", 0);
        av_opt_set(pCodecCtx->priv_data, "tune", "zerolatency", 0);
        ret = avcodec_open2(pCodecCtx, pCodec, NULL);
        if (ret < 0) {
            LOGD("Failed to open encoder! \n");
            return ret;
        }
        frameCount = 0;
        if ((fp_out = fopen(file, "wb")) == NULL) {
            LOGD("open file fail");
            return -1;
        }
        ret = 1;
        return ret;
    }

}