//
// Created by bearshi on 2017/6/13.
//

#ifndef NEWANDROID_FFMPEGMUXERING_H
#define NEWANDROID_FFMPEGMUXERING_H

extern "C"
{
    #include "libavformat/avformat.h"
};

namespace video {
    class FfmpegMuxering {
    private:
        int OpenInputH264File(const char *intputfilename);

        int OpenInputAacFile(const char *intputfilename);

        int OpenOutputAacFile(const char *outputaacfilename);

        AVFormatContext *ifmt_ctx_h264;
        AVFormatContext *ifmt_ctx_aac;
        AVFormatContext *ofmt_ctx;
        int _index_in_h264;
        int _index_in_aac;
        int _index_out_h264;
        int _index_out_aac;
    public:
        FfmpegMuxering();

        ~FfmpegMuxering();

        int
        MuxerH264Aac(const char *h264filename, const char *aacfilename, const char *outputfilename);
    };
}

#endif //NEWANDROID_FFMPEGMUXERING_H
