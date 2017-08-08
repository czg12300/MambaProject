//
// Created by jakechen on 2017/7/22.
//

#include "FfmpegCodec.h"

namespace video {
    FfmpegCodec::FfmpegCodec() {
        avcodec_register_all();
        format = new MediaFormat();
        mf_key = new MediaFormatKey();
    }

    void FfmpegCodec::_configure(const char *key, int value) {
        if (strcmp(key, mf_key->BIT_RATE) == 0) {
            format->bit_rate = value;
        } else if (strcmp(key, mf_key->CHANNEL_LAYOUT) == 0) {
            format->channel_layout = value;
        } else if (strcmp(key, mf_key->CHANNELS) == 0) {
            format->channels = value;
        } else if (strcmp(key, mf_key->CODEC_ID) == 0) {
            format->codec_id = value;
        } else if (strcmp(key, mf_key->CODEC_TYPE) == 0) {
            LOGD("key , mf_key->CODEC_TYPE");
            format->codec_type = value;
        } else if (strcmp(key, mf_key->FRAME_RATE) == 0) {
            format->frame_rate = value;
        } else if (strcmp(key, mf_key->HEIGHT) == 0) {
            format->height = value;
        } else if (strcmp(key, mf_key->IS_ENCODE) == 0) {
            format->isEncode = value > 0 ? true : false;
        } else if (strcmp(key, mf_key->KEY_I_FRAME_INTERVAL) == 0) {
            format->key_i_frame_interval = value;
        } else if (strcmp(key, mf_key->PIX_FMT) == 0) {
            format->pix_fmt = value;
        } else if (strcmp(key, mf_key->SAMPLE_RATE) == 0) {
            format->sample_rate = value;
        } else if (strcmp(key, mf_key->WIDTH) == 0) {
            format->width = value;
        }
        LOGD("_configure  %s %d  ", key, value);
    }

    void FfmpegCodec::start() {
        delete mf_key;
        frameIndex = 0;
        if (isEncode()) {
            initEncoder();
        } else {
            initDecoder();
        }
        LOGD("start");
    }

    void FfmpegCodec::initDecoder() {
        AVCodecID codecID = format->transCodeId(format->codec_id);
        AVCodec *pCodec = avcodec_find_decoder(codecID);
        if (pCodec == NULL) {
            LOGE("open_decodec_context()无法根据avstream找到decoder");
            return;
        }
        pCodecCtx = avcodec_alloc_context3(pCodec);
        if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
            LOGE("open_decodec_context()无法打开编码器");
            return;
        }
        pCodecParserCtx = av_parser_init(codecID);
        if (!pCodecParserCtx) {
            printf("Could not allocate video parser context\n");
            return;
        }
    }

    void FfmpegCodec::initEncoder() {
        //打开解码器
        AVCodec *pCodec = avcodec_find_decoder(format->transCodeId(format->codec_id));
        if (pCodec == NULL) {
            LOGE("open_decodec_context()无法根据avstream找到decoder");
            return;
        }
        pCodecCtx = avcodec_alloc_context3(pCodec);
        pCodecCtx->codec_type = format->transCodeType(format->codec_type);
        pCodecCtx->pix_fmt = format->transPixelFormat(format->pix_fmt);
        pCodecCtx->width = format->width;
        pCodecCtx->height = format->height;
        pCodecCtx->bit_rate = format->bit_rate;
        pCodecCtx->lumi_masking = 0.0;
        pCodecCtx->dark_masking = 0.0;
        pCodecCtx->qmin = 10;
        pCodecCtx->qmax = 51;
        pCodecCtx->time_base.num = 1;
        pCodecCtx->time_base.den = format->frame_rate;
        pCodecCtx->bit_rate_tolerance = 40 * 10000;
        pCodecCtx->qcompress = 1;
        pCodecCtx->gop_size = format->key_i_frame_interval * format->frame_rate;
        av_opt_set(pCodecCtx->priv_data, "preset", "ultrafast", 0);
        av_opt_set(pCodecCtx->priv_data, "tune", "zerolatency", 0);
        if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
            LOGE("open_decodec_context()无法打开编码器");
            return;
        }

    }

    bool FfmpegCodec::isEncode() {
        return format != NULL && format->isEncode;
    }

    uint8_t *FfmpegCodec::_decodeOrEncode(uint8_t *src, int srcSize, int *len) {
        if (isEncode()) {
            return encode(src, srcSize, len);
        } else {
            return decode(src, srcSize, len);
        }
        return NULL;
    }

    uint8_t *FfmpegCodec::decode(uint8_t *src, int srcSize, int *len) {
        if (src == NULL) {
            return NULL;
        }
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = src;
        pkt.size = srcSize;
        av_parser_parse2(pCodecParserCtx, pCodecCtx, &pkt.data, &pkt.size, src, srcSize,
                         AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
        AVFrame *frame = av_frame_alloc();;
        int ret = avcodec_send_packet(pCodecCtx, &pkt);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            LOGE("avcodec_send_packet fail ret  %d", ret);
            av_packet_unref(&pkt);
            return NULL;
        }
        //从解码器返回解码输出数据
        ret = avcodec_receive_frame(pCodecCtx, frame);
        if (ret < 0 && ret != AVERROR_EOF) {
            LOGE("avcodec_receive_frame fail");
            av_packet_unref(&pkt);
            av_frame_free(&frame);
            return NULL;
        }
        LOGD("decode success format->codec_type=%d  format->CODE_TYPE_VIDEO=%d", format->codec_type,
             format->CODE_TYPE_VIDEO);
        av_packet_unref(&pkt);
        if (format->codec_type == format->CODE_TYPE_VIDEO) {
            LOGD("format->codec_type == format->CODE_TYPE_VIDEO frame->format  %d  %d",frame->format,AV_PIX_FMT_YUV420P);
//            switch (frame->format) {
//                case AV_PIX_FMT_YUV420P:
                    LOGD("AV_PIX_FMT_YUV420P frame->width %d  frame->height  %d  %d" ,format->width,format->height,frame->pkt_size);
                    int size = format->width * format->height;
                    uint8_t *data = new uint8_t[size * 3 / 2];
                    memcpy(data, frame->data[0], size);
                    memcpy(data, frame->data[1], size / 4);
                    memcpy(data, frame->data[2], size / 4);
                    *len = size * 3 / 2;
                    return data;
//            }
        }

        return NULL;
    }

    uint8_t *FfmpegCodec::encode(uint8_t *src, int srcSize, int *len) {
        if (src == NULL) {
            return NULL;
        }
        AVFrame *frame = av_frame_alloc();
        if (!frame) {
            return NULL;
        }
        avpicture_fill((AVPicture *) frame, src, AV_PIX_FMT_YUV420P, format->width, format->height);
        frame->pts = frameIndex++;
        LOGI("encode frameCount = %d\n", frameIndex);
        int ret = avcodec_send_frame(pCodecCtx, frame);
        if (ret < 0) {
            LOGE("Failed to avcodec_send_frame! \n");
            av_freep(&frame->data[0]);
            av_frame_unref(frame);
            av_frame_free(&frame);
            return NULL;
        }
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = NULL; // packet data will be allocated by the encoder
        pkt.size = 0;
        ret = avcodec_receive_packet(pCodecCtx, &pkt);
        if (ret < 0) {
            LOGE("Failed to avcodec_receive_packet! \n");
            return NULL;
        }
        uint8_t *data = new uint8_t[pkt.size];
        memcpy(data, pkt.data, pkt.size);
        av_packet_unref(&pkt);
        *len = pkt.size;
        return data;
    }

    AVFrame *FfmpegCodec::pktFrame(uint8_t *data) {
        AVFrame *frame = av_frame_alloc();
        if (!frame) {
            LOGI("无法分配Video Frame\n");
            return NULL;
        }
        avpicture_fill((AVPicture *) frame, data, AV_PIX_FMT_YUV420P, format->width,
                       format->height);
        return frame;
    }

    void FfmpegCodec::release() {
        if (pCodecCtx != NULL) {
            avcodec_close(pCodecCtx);
        }
        delete format;
    }

}