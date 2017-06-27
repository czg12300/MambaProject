//
// Created by bearshi on 2017/6/16.
//

#ifndef ANDROID_AUDIODEALPRODUCE_H
#define ANDROID_AUDIODEALPRODUCE_H

#include "FFmpegBase.h"
#include "ThreadExecutor.h"
#include "ThreadPool.h"
#include "BaseAudioEffect.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libswresample/swresample.h>
#include <libavutil/audio_fifo.h>
}

namespace video {
    struct AudioFilterParams {
        string srcFile;
        string outFile;
        int rangeStart;
        int rangeEnd;
        BaseAudioEffect *effect;
    };

    class AudioDealProduce {
    private:
        AVFormatContext *input_format_context;
        AVFormatContext *output_format_context;
        AVCodecContext *input_codec_context;
        AVCodecContext *output_codec_context;
        SwrContext *resample_context;
        AVAudioFifo *fifo;
        pthread_t mutex_start_thread;
        AVFilterContext *buffersink_ctx;
        AVFilterContext *buffersrc_ctx;
        AVFilterGraph *filter_graph;
        int64_t pts = 0;
        AudioFilterParams *params;
        int opkt_size;
        int out_nb_frames;
        int g_audio_index;
        int g_enc_frame_size;
        bool decode_pkt_flag;
        int package_count;
        int total_package_count;

        int open_input_file(const char *filename, AVFormatContext **input_format_context,
                            AVCodecContext **input_codec_context);
        int open_output_file(const char *filename, AVCodecContext *input_codec_context,
                             AVFormatContext **output_format_context, AVCodecContext **output_codec_context);
        void init_packet(AVPacket *packet);
        int init_input_frame(AVFrame **frame);
        int init_filter(AVCodecContext *dec_ctx, AVCodecContext *enc_ctx, const char *filter_spec);
        int init_resampler(AVCodecContext *input_codec_context, AVCodecContext *output_codec_context,
                            SwrContext **resample_context);
        int init_fifo(AVAudioFifo **fifo, AVCodecContext *output_codec_context);
        int write_output_file_header(AVFormatContext *output_format_context);
        int init_converted_samples(uint8_t ***converted_input_samples,
                                   AVCodecContext *output_codec_context,
                                   int nb_samples);

        int convert_samples(SwrContext *resample_context, uint8_t **converted_data,	int out_samples,
                            const uint8_t **input_data,	int in_samples);
        int add_samples_to_fifo(AVAudioFifo *fifo, uint8_t **converted_input_samples,
                                const int nb_samples);
        int init_output_frame(AVFrame **frame, AVCodecContext *output_codec_context, int frame_size);
        int encode_audio_frame(AVFrame *frame, AVFormatContext *output_format_context, AVCodecContext *output_codec_context,
                                                int *data_present, int *finish);
        int load_encode_and_write(AVAudioFifo *fifo, AVFormatContext *output_format_context,
                                   AVCodecContext *output_codec_context, int *finish);
        void flush_encoders(AVFormatContext *output_format_context, AVCodecContext *output_codec_context);
        int write_output_file_trailer(AVFormatContext *output_format_context);
        int convert_frame_to_fifo(AVAudioFifo *fifo, AVFrame *frame, AVFilterContext *sink, AVFilterContext *src);
        void get_total_Package();

    public:
        AudioDealProduce(AudioFilterParams *params);
        ~AudioDealProduce();
        void Start();
        void DoDeal();
        void NoDecodeEncode();
    };

    struct ThreadExecuteAudioParams1 {
        int methodId;
        AudioDealProduce *produce;
    };

    static const int AUDIO_METHOD_ID_START = 1;
    static const int AUDIO_METHOD_ID_NODECODE_ENCODE = 2;
    static void *threatExecute(void *arg);
}

#endif //ANDROID_AUDIODEALPRODUCE_H
