
//
// Created by jakechen on 2017/5/12.
//
#include "FfmpegVideoUtilsBase.h"
#include <vector>
#include <vector>
#include <BaseVideoEffect.h>

extern "C" {
#include "libavformat/avformat.h"
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>

}
namespace video {
    class VideoCutHandle : public BaseVideoEffect {
    private:
        int *ret;
        long rangeStart;
    public:
        VideoCutHandle(int *result);

        void onFail();

        void onSuccess();

        long getNeedSeekTime(AVPacket pkt);

        void init(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx, AVCodecContext *decode_ctx,
                  AVCodecContext *encode_ctx, AVStream *in_stream, AVStream *out_stream, long rangeStart,
                  long rangeEnd);

        bool needDecode();


        void optFrame(DecodeFrame *frame);

        void release();

        void onReadPackage(AVPacket pkt);
    };

    int cutVideo(const char *srcFile, long startMillis, long endMillis, const char *outFile);

    int cutAudio(const char *srcFile, long startMillis, long endMillis, const char *outFile);

}