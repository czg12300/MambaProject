//
// Created by jakechen on 2017/7/24.
//
#include "MediaFormat.h"

namespace video {
    MediaFormat::MediaFormat() {
        av_register_all();
    }

    int MediaFormat::transCodeId(AVCodecID id) {
        int result = -1;
        switch (id) {
            case AV_CODEC_ID_H264:
                result = CODE_ID_H264;
                break;
            case AV_CODEC_ID_AAC:
                result = CODE_ID_AAC;
                break;
        }
        return result;
    }

    AVCodecID MediaFormat::transCodeId(int id) {
        AVCodecID result = AV_CODEC_ID_H264;
        switch (id) {
            case CODE_ID_H264:
                result = AV_CODEC_ID_H264;
                break;
            case CODE_ID_AAC:
                result = AV_CODEC_ID_AAC;
                break;
        }
        return result;
    }

    int MediaFormat::transCodeType(AVMediaType id) {
        int result = -1;
        switch (id) {
            case AVMEDIA_TYPE_VIDEO:
                result = CODE_TYPE_VIDEO;
                break;
            case AVMEDIA_TYPE_AUDIO:
                result = CODE_TYPE_AUDIO;
                break;
        }
        return result;
    }

    AVMediaType MediaFormat::transCodeType(int id) {
        AVMediaType result = AVMEDIA_TYPE_VIDEO;
        switch (id) {
            case CODE_TYPE_VIDEO:
                result = AVMEDIA_TYPE_VIDEO;
                break;
            case CODE_TYPE_AUDIO:
                result = AVMEDIA_TYPE_AUDIO;
                break;
        }
        return result;
    }

    int MediaFormat::transPixelFormat(AVPixelFormat pix_fmt) {
        int result = -1;
        switch (pix_fmt) {
            case AV_PIX_FMT_YUV420P:
                result = PIX_FMT_I420P;
                break;
            case AV_PIX_FMT_NV21:
                result = PIX_FMT_NV21;
                break;
        }
        return result;
    }

    AVPixelFormat MediaFormat::transPixelFormat(int pix_fmt) {
        AVPixelFormat result = AV_PIX_FMT_YUV420P;
        switch (pix_fmt) {
            case PIX_FMT_I420P:
                result = AV_PIX_FMT_YUV420P;
                break;
            case PIX_FMT_NV21:
                result = AV_PIX_FMT_NV21;
                break;
        }
        return result;
    }
}
