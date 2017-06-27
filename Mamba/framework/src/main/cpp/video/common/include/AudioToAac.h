//
// Created by bearshi on 2017/6/13.
//

#ifndef NEWANDROID_AUDIOTOAAC_H
#define NEWANDROID_AUDIOTOAAC_H
extern "C"{
    #include "libavformat/avformat.h"
    #include "libavfilter/avfilter.h"
    #include "libavutil/opt.h"
    #include "libavfilter/buffersrc.h"
    #include "libavfilter/buffersink.h"
    #include "libavformat/avformat.h"
    #include "libavformat/avio.h"
    #include "libavcodec/avcodec.h"
    #include "libavutil/audio_fifo.h"
    #include "libavutil/avassert.h"
    #include "libavutil/avstring.h"
    #include "libavutil/frame.h"
    #include "libavutil/opt.h"
    #include "libswresample/swresample.h"
};

namespace video {
    class AudioToAac{
        private:
            AVFormatContext *ifmt_ctx = NULL;
            AVFormatContext *ofmt_ctx = NULL;
            int _index_in_audio = -1;
            int _index_out_audio = 0;
            AVAudioFifo* _fifo_src = NULL;
            AVFilterGraph* _filter_graph = NULL;
            AVFilterContext* _filter_ctx_src = NULL;
            AVFilterContext* _filter_ctx_sink = NULL;

            int open_input_file(const char *filename);
            int open_output_file(const char *filename);
            int init_filter(char* filter_desc);
            int init_fifo();
            int init_input_frame(AVFrame **frame);
            int read_decode_to_fifo(int *finished);
            int init_output_frame(AVFrame **frame, int frame_size);
            int write_encode_from_fifo(int *frame_count);

    public:
        AudioToAac();
        ~AudioToAac();
        int audioMp3ToAacBase(const char *inputfilename, const char *outputfilename);
    };
}
#endif //NEWANDROID_AUDIOTOAAC_H
