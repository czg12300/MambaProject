//
// Created by jakechen on 2017/7/22.
//

#ifndef MAMBA_MEDIAFORMAT_H
#define MAMBA_MEDIAFORMAT_H
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
}
namespace video {
    struct MediaFormatKey{
        const char *CHANNELS = "channels";
        const char *CODEC_TYPE = "codec_type";
        const char *CODEC_ID = "codec_id";
        const char *CHANNEL_LAYOUT = "channel_layout";
        const char *WIDTH = "width";
        const char *HEIGHT = "height";
        const char *BIT_RATE = "bit_rate";
        const char *FRAME_RATE = "frame_rate";
        const char *SAMPLE_RATE = "sample_rate";
        const char *PIX_FMT = "pix_fmt";
        const char *KEY_I_FRAME_INTERVAL = "key_i_frame_interval";
        const char *IS_ENCODE = "is_encode";
    };

    class MediaFormat {

    public:

        const static int CODE_TYPE_VIDEO = 1;
        const static int CODE_TYPE_AUDIO = 2;
        const static int CODE_ID_H264 = 1;
        const static int CODE_ID_AAC = 2;
        const static int PIX_FMT_I420P = 1;
        const static int PIX_FMT_NV21 = 2;

        int codec_type;
        int width;
        int height;
        int bit_rate;
        int frame_rate;
        int sample_rate;
        int channels;
        int channel_layout;
        int codec_id;
        int pix_fmt;
        int key_i_frame_interval;
        bool isEncode = false;

        MediaFormat();

        int transCodeId(AVCodecID id);

        AVCodecID transCodeId(int id);

        int transCodeType(AVMediaType id);

        AVMediaType transCodeType(int id);

        int transPixelFormat(AVPixelFormat id);

        AVPixelFormat transPixelFormat(int pix_fmt);
    };

}
#endif //MAMBA_MEDIAFORMAT_H
