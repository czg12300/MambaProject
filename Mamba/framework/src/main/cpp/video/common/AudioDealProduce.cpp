//
// Created by bearshi on 2017/6/16.
//

#include "AudioDealProduce.h"
#include "FfmpegVideoCut.h"

#define OUTPUT_BIT_RATE 128000
#define OUTPUT_CHANNELS 2

const int DEFAULT_FRAME_SIZE = 1024;
namespace video {
    AudioDealProduce::AudioDealProduce(AudioFilterParams *params) {
        this->buffersink_ctx = NULL;
        this->buffersrc_ctx = NULL;
        this->filter_graph = NULL;
        this->params = params;
        this->input_format_context = NULL;
        this->output_format_context = NULL;
        this->input_codec_context = NULL;
        this->output_codec_context = NULL;
        this->resample_context = NULL;
        this->fifo = NULL;
        this->pts = 0;
        this->opkt_size = 0;
        this->out_nb_frames = 0;
        this->g_audio_index = -1;
        this->g_enc_frame_size = 0;
        this->package_count = 0;
        this->decode_pkt_flag = false;

        if (params->rangeEnd < params->rangeStart) {
            params->rangeEnd = 30000;
        }
    }

    int AudioDealProduce::open_input_file(const char *filename, AVFormatContext **input_format_context,
                        AVCodecContext **input_codec_context) {
        AVCodecContext *avctx;
        AVCodec *input_codec;
        int error;

        error = avformat_open_input(input_format_context, filename, NULL,NULL);
        if (error < 0)
        {
            fprintf(stderr, "Could not open input file '%s' (error '%s')\n",
                    filename, av_err2str(error));
            *input_format_context = NULL;
            return error;
        }

        /** Get information on the input file (number of streams etc.). */
        if ((error = avformat_find_stream_info(*input_format_context, NULL)) < 0) {
            fprintf(stderr, "Could not open find stream info (error '%s')\n",
                    av_err2str(error));
            avformat_close_input(input_format_context);
            return error;
        }

        /** Find a decoder for the audio stream. */
        for (unsigned int i = 0; i < (*input_format_context)->nb_streams; i++) {
            if ((*input_format_context)->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                g_audio_index = i;
                printf("Find Audio streams[%d]\n", i);
                break;

            }
        }
        if (g_audio_index == -1) {
            printf("No Audio Streams\n");
            avformat_close_input(input_format_context);
            return AVERROR_EXIT;
        }
        if (!(input_codec = avcodec_find_decoder((*input_format_context)->streams[g_audio_index]->codecpar->codec_id))) {
            fprintf(stderr, "Could not find input codec\n");
            avformat_close_input(input_format_context);
            return AVERROR_EXIT;
        }

        avctx = avcodec_alloc_context3(input_codec);
        if (!avctx) {
            fprintf(stderr, "Could not allocate a decoding context\n");
            avformat_close_input(input_format_context);
            return AVERROR(ENOMEM);
        }

        /** initialize the stream parameters with demuxer information */
        error = avcodec_parameters_to_context(avctx, (*input_format_context)->streams[g_audio_index]->codecpar);
        if (error < 0) {
            avformat_close_input(input_format_context);
            avcodec_free_context(&avctx);
            return error;
        }

        /** Open the decoder for the audio stream to use it later. */
        if ((error = avcodec_open2(avctx, input_codec, NULL)) < 0) {
            fprintf(stderr, "Could not open input codec (error '%s')\n",
                    av_err2str(error));
            avcodec_free_context(&avctx);
            avformat_close_input(input_format_context);
            return error;
        }

        *input_codec_context = avctx;

        return 0;
    }

    int AudioDealProduce::open_output_file(const char *filename,
                                AVCodecContext *input_codec_context,
                                AVFormatContext **output_format_context,
                                AVCodecContext **output_codec_context)
    {
        AVCodecContext *avctx          = NULL;
        AVStream *stream               = NULL;
        AVCodec *output_codec          = NULL;
        int error;

        error = avformat_alloc_output_context2(output_format_context, NULL, NULL, filename);
        if (error < 0) {
            fprintf(stderr, "Could not create output context-'%s' (error '%s')\n",
                    filename, av_err2str(error));
            return error;
        }

        /** Create a new audio stream in the output file container. */
        if (!(stream = avformat_new_stream(*output_format_context, NULL))) {
            fprintf(stderr, "Could not create new stream\n");
            error = AVERROR(ENOMEM);
            goto cleanup;
        }

        /** Find the encoder to be used by its name. */
        stream->codecpar->codec_id = av_guess_codec((*output_format_context)->oformat, NULL, (*output_format_context)->filename, NULL, AVMEDIA_TYPE_AUDIO);
        output_codec = avcodec_find_encoder(stream->codecpar->codec_id);
        if (!output_codec) {
            fprintf(stderr, "Could not find encoder.\n");
            goto cleanup;
        }

        avctx = avcodec_alloc_context3(output_codec);
        if (!avctx) {
            fprintf(stderr, "Could not allocate an encoding context\n");
            error = AVERROR(ENOMEM);
            goto cleanup;
        }

        /**
         * Set the basic encoder parameters.
         * The input file's sample rate is used to avoid a sample rate conversion.
         */

        avctx->channels       = input_codec_context->channels;//OUTPUT_CHANNELS;
        avctx->channel_layout = input_codec_context->channel_layout;//av_get_default_channel_layout(OUTPUT_CHANNELS);
        avctx->sample_rate    = input_codec_context->sample_rate;//48000;
        avctx->sample_fmt     = output_codec->sample_fmts[0];
        avctx->bit_rate       = input_codec_context->bit_rate;//OUTPUT_BIT_RATE;

        /** Allow the use of the experimental AAC encoder */
        avctx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

        /**
         * Some container formats (like MP4) require global headers to be present
         * Mark the encoder so that it behaves accordingly.
         */
        if ((*output_format_context)->oformat->flags & AVFMT_GLOBALHEADER)
            avctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        /** Open the encoder for the audio stream to use it later. */
        if ((error = avcodec_open2(avctx, output_codec, NULL)) < 0) {
            fprintf(stderr, "Could not open output codec (error '%s')\n",
                    av_err2str(error));
            goto cleanup;
        }

        g_enc_frame_size = (avctx->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE) ?
                           DEFAULT_FRAME_SIZE : avctx->frame_size;

        error = avcodec_parameters_from_context(stream->codecpar, avctx);
        if (error < 0) {
            fprintf(stderr, "Could not initialize stream parameters\n");
            goto cleanup;
        }


        *output_codec_context = avctx;

        if (!((*output_format_context)->oformat->flags &  AVFMT_NOFILE)) {
            error = avio_open(&(*output_format_context)->pb, filename, AVIO_FLAG_WRITE);
            if (error < 0) {
                fprintf(stderr, "Could not open output file '%s'", filename);
                goto cleanup;
            }
        }

        return 0;

        cleanup:
        avcodec_free_context(&avctx);
        avio_closep(&(*output_format_context)->pb);
        avformat_free_context(*output_format_context);
        *output_format_context = NULL;
        return error < 0 ? error : AVERROR_EXIT;
    }

    void AudioDealProduce::init_packet(AVPacket *packet)
    {
        av_init_packet(packet);
        /** Set the packet data and size so that it is recognized as being empty. */
        packet->data = NULL;
        packet->size = 0;
    }

    int AudioDealProduce::init_input_frame(AVFrame **frame)
    {
        if (!(*frame = av_frame_alloc())) {
            fprintf(stderr, "Could not allocate input frame\n");
            return AVERROR(ENOMEM);
        }
        return 0;
    }

    int AudioDealProduce::init_filter(AVCodecContext *dec_ctx,
                    AVCodecContext *enc_ctx, const char *filter_spec)
    {
        char args[512];
        int ret = 0;
        AVFilter *buffersrc = NULL;
        AVFilter *buffersink = NULL;
        AVFilterInOut *outputs = avfilter_inout_alloc();
        AVFilterInOut *inputs = avfilter_inout_alloc();
        filter_graph = avfilter_graph_alloc();

        LOGI("AudioDealProduce::init_filter filter_spec = %s", filter_spec);
        if (!outputs || !inputs || !filter_graph) {
            ret = AVERROR(ENOMEM);
            goto end;
        }

        buffersrc = avfilter_get_by_name("abuffer");
        buffersink = avfilter_get_by_name("abuffersink");
        if (!buffersrc || !buffersink) {
            av_log(NULL, AV_LOG_ERROR, "filtering source or sink element not found\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }
/* input */
        if (!dec_ctx->channel_layout)
            dec_ctx->channel_layout =
                    av_get_default_channel_layout(dec_ctx->channels);
        snprintf(args, sizeof(args),
                 "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%llu",
                 dec_ctx->time_base.num, dec_ctx->time_base.den, dec_ctx->sample_rate,
                 av_get_sample_fmt_name(dec_ctx->sample_fmt),
                 dec_ctx->channel_layout);

        ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
                                           args, NULL, filter_graph);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer source\n");
            goto end;
        }

/* output */
        ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                                           NULL, NULL, filter_graph);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer sink\n");
            goto end;
        }

        ret = av_opt_set_bin(buffersink_ctx, "sample_fmts",
                             (uint8_t*)&enc_ctx->sample_fmt, sizeof(enc_ctx->sample_fmt),
                             AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot set output sample format\n");
            goto end;
        }

        ret = av_opt_set_bin(buffersink_ctx, "channel_layouts",
                             (uint8_t*)&enc_ctx->channel_layout,
                             sizeof(enc_ctx->channel_layout), AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot set output channel layout\n");
            goto end;
        }

        ret = av_opt_set_bin(buffersink_ctx, "sample_rates",
                             (uint8_t*)&enc_ctx->sample_rate, sizeof(enc_ctx->sample_rate),
                             AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Cannot set output sample rate\n");
            goto end;
        }


        /* Endpoints for the filter graph. */
        outputs->name = av_strdup("in");
        outputs->filter_ctx = buffersrc_ctx;
        outputs->pad_idx = 0;
        outputs->next = NULL;

        inputs->name = av_strdup("out");
        inputs->filter_ctx = buffersink_ctx;
        inputs->pad_idx = 0;
        inputs->next = NULL;

        if (!outputs->name || !inputs->name) {
            ret = AVERROR(ENOMEM);
            goto end;
        }

        if ((ret = avfilter_graph_parse_ptr(filter_graph, filter_spec,
                                            &inputs, &outputs, NULL)) < 0)
            goto end;

        if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
            goto end;

        /* Fill FilteringContext */


        end:
        avfilter_inout_free(&inputs);
        avfilter_inout_free(&outputs);

        return ret;
    }

    int AudioDealProduce::init_resampler(AVCodecContext *input_codec_context,
                                           AVCodecContext *output_codec_context,
                                           SwrContext **resample_context)
    {
        int error;
        SwrContext *swr_ctx = NULL;
        swr_ctx = swr_alloc();
        if (swr_ctx == NULL) {
            fprintf(stderr, "Could not allocate resample context\n");
            return AVERROR(ENOMEM);
        }

        if (av_opt_set_int(swr_ctx, "ocl", av_get_default_channel_layout(output_codec_context->channels), 0) < 0) {
            fprintf(stderr, "Could not set ocl\n");
            goto fail;
        }
        if (av_opt_set_int(swr_ctx, "osf", output_codec_context->sample_fmt, 0) < 0) {
            fprintf(stderr, "Could not set osf\n");
            goto fail;
        }
        if (av_opt_set_int(swr_ctx, "osr", output_codec_context->sample_rate, 0) < 0) {
            fprintf(stderr, "Could not set ocl\n");
            goto fail;
        }

        if (av_opt_set_int(swr_ctx, "icl", av_get_default_channel_layout(input_codec_context->channels), 0) < 0) {
            fprintf(stderr, "Could not set icl\n");
            goto fail;
        }
        if (av_opt_set_int(swr_ctx, "isf", input_codec_context->sample_fmt, 0) < 0) {
            fprintf(stderr, "Could not set isf\n");
            goto fail;
        }
        if (av_opt_set_int(swr_ctx, "isr", input_codec_context->sample_rate, 0) < 0) {
            fprintf(stderr, "Could not set isr\n");
            goto fail;
        }

        if (av_opt_set_int(swr_ctx, "ich", input_codec_context->channels, 0) < 0) {
            fprintf(stderr, "Could not set ich\n");
            goto fail;
        }
        if (av_opt_set_int(swr_ctx, "och", output_codec_context->channels, 0) < 0) {
            fprintf(stderr, "Could not set och\n");
            goto fail;
        }
        /**
        * Perform a sanity check so that the number of converted samples is
        * not greater than the number of samples to be converted.
        * If the sample rates differ, this case has to be handled differently
        */
        //       av_assert0(output_codec_context->sample_rate == input_codec_context->sample_rate);

        /** Open the resampler with the specified parameters. */
        if ((error = swr_init(swr_ctx)) < 0) {
            fprintf(stderr, "Could not open resample context\n");
            swr_free(&swr_ctx);
            return error;
        }

        *resample_context = swr_ctx;
        return 0;
        fail:
        swr_free(&swr_ctx);
        return -1;
    }

    int AudioDealProduce::init_fifo(AVAudioFifo **fifo, AVCodecContext *output_codec_context)
    {
        /** Create the FIFO buffer based on the specified output sample format. */
        if (!(*fifo = av_audio_fifo_alloc(output_codec_context->sample_fmt,
                                          output_codec_context->channels, 1))) {
            fprintf(stderr, "Could not allocate FIFO\n");
            return AVERROR(ENOMEM);
        }
        return 0;
    }

    int AudioDealProduce::write_output_file_header(AVFormatContext *output_format_context)
    {
        int error;

        if ((error = avformat_write_header(output_format_context, NULL)) < 0) {
            fprintf(stderr, "Could not write output file header (error '%s')\n",
                    av_err2str(error));
            return error;
        }
        return 0;
    }

    int AudioDealProduce::init_converted_samples(uint8_t ***converted_input_samples,
                                                   AVCodecContext *output_codec_context,
                                                   int nb_samples)
    {
        int error;
        int line_size = -1;

        error = av_samples_alloc_array_and_samples(converted_input_samples, &line_size, output_codec_context->channels,
                                                   nb_samples, output_codec_context->sample_fmt, 1);
        if (error < 0) {
            fprintf(stderr,
                    "Could not allocate converted input samples (error '%s-%d')\n",
                    av_err2str(error), error);
            fprintf(stderr,"channels = %d, frame_size = %d, sample_fmt = %d, line_size = %d\n",
                    output_codec_context->channels, nb_samples, output_codec_context->sample_fmt, line_size);
            return error;
        }

        return 0;
    }

    int AudioDealProduce::convert_samples(SwrContext *resample_context,
                        uint8_t **converted_data,	int out_samples,
                        const uint8_t **input_data,	int in_samples)
    {
        int error;

        /** Convert the samples using the resampler. */
        if ((error = swr_convert(resample_context,
                                 converted_data, out_samples,
                                 input_data    , in_samples)) < 0) {
            fprintf(stderr, "Could not convert input samples (error '%s')\n",
                    av_err2str(error));
            return error;
        }

        return 0;
    }

    int AudioDealProduce::add_samples_to_fifo(AVAudioFifo *fifo,
                            uint8_t **converted_input_samples,
                            const int nb_samples) {
        int error;

        if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + nb_samples)) < 0) {
            fprintf(stderr, "Could not reallocate FIFO\n");
            return error;
        }

        error = av_audio_fifo_write(fifo, (void **)converted_input_samples, nb_samples);
        if (error < nb_samples) {
            fprintf(stderr, "Could not write data to FIFO\n");
            return AVERROR_EXIT;
        }
        return 0;
    }

    int AudioDealProduce::init_output_frame(AVFrame **frame,
                         AVCodecContext *output_codec_context,
                         int frame_size)
    {
        int error;
        if (!(*frame = av_frame_alloc())) {
            fprintf(stderr, "Could not allocate output frame\n");
            return AVERROR_EXIT;
        }

        (*frame)->nb_samples     = frame_size;
        (*frame)->channel_layout = output_codec_context->channel_layout;
        (*frame)->format         = output_codec_context->sample_fmt;
        (*frame)->sample_rate    = output_codec_context->sample_rate;

        if ((error = av_frame_get_buffer(*frame, 0)) < 0) {
            fprintf(stderr, "Could not allocate output frame samples (error '%s')\n",
                    av_err2str(error));
            av_frame_free(frame);
            return error;
        }

        return 0;
    }

    int AudioDealProduce::encode_audio_frame(AVFrame *frame,
                           AVFormatContext *output_format_context,
                           AVCodecContext *output_codec_context,
                           int *data_present, int *finish)
    {
        /** Packet used for temporary storage. */
        AVPacket output_packet;
        int error;
        *data_present = 0;

        init_packet(&output_packet);
        output_packet.stream_index = 0;

        if (frame) {
            frame->pts = pts;
            LOGI("frame.pts = %lld\n", frame->pts);
            pts += frame->nb_samples;
        }

        error = avcodec_send_frame(output_codec_context, frame);
        if (error == AVERROR_EOF) {
            fprintf(stderr, "(error '%s')\n",
                    av_err2str(error));
            av_packet_unref(&output_packet);
            return 0;
        }

        if (error < 0) {
            fprintf(stderr, "Could not avcodec_send_frame (error '%s')\n",
                    av_err2str(error));
            av_packet_unref(&output_packet);
            return error;
        }

        out_nb_frames++;
        while (1) {
            error = avcodec_receive_packet(output_codec_context, &output_packet);
            if (error == AVERROR(EAGAIN) || error == AVERROR_EOF) {
                return 0;
            }
            else if (error < 0) {
                printf("avcodec_receive_packet error = %d\n", error);
                return error;
            }

            output_packet.stream_index = 0;
            error = av_write_frame(output_format_context, &output_packet);
            if (error < 0) {
                fprintf(stderr, "Could not write frame (error '%s')\n",
                        av_err2str(error));
                av_packet_unref(&output_packet);
                return error;
            }
            *data_present = 1;
            opkt_size += output_packet.size;

            double currtime = av_q2d(output_codec_context->time_base) * output_packet.pts * 1000;
            if (currtime > params->rangeEnd) {
                (*finish) = 1;
                LOGI("AudioDealProduce:: currtime = %lf rangeEnd = %ld", currtime,  params->rangeEnd);
                return 0;
            }
        }
        return 0;
    }

    int AudioDealProduce::load_encode_and_write(AVAudioFifo *fifo,
                              AVFormatContext *output_format_context,
                              AVCodecContext *output_codec_context,
                              int *finish )
    {
        AVFrame *output_frame;
        const int frame_size = FFMIN(av_audio_fifo_size(fifo),
                                     g_enc_frame_size);

        int data_written;
        if (init_output_frame(&output_frame, output_codec_context, frame_size))
            return AVERROR_EXIT;
        if (av_audio_fifo_read(fifo, (void **)output_frame->data, frame_size) < frame_size) {
            fprintf(stderr, "Could not read data from FIFO\n");
            av_frame_free(&output_frame);
            return AVERROR_EXIT;
        }

        if (encode_audio_frame(output_frame, output_format_context,
                               output_codec_context, &data_written, finish)) {
            av_frame_free(&output_frame);
            return AVERROR_EXIT;
        }
        av_frame_free(&output_frame);
        return 0;
    }

    void AudioDealProduce::flush_encoders(AVFormatContext *output_format_context, AVCodecContext *output_codec_context)
    {
        AVPacket output_packet;
        int error;
        init_packet(&output_packet);
        avcodec_send_frame(output_codec_context, NULL);
        while (1) {
            error = avcodec_receive_packet(output_codec_context, &output_packet);
            if (error == 0) {
                error = av_write_frame(output_format_context, &output_packet);
            }
            if (error != 0) {
                break;
            }
        }
        opkt_size += output_packet.size;
        printf("write end packet ok,packet.size = %d\n", output_packet.size);
        return;
    }

    int AudioDealProduce::write_output_file_trailer(AVFormatContext *output_format_context)
    {
        int error;

        if ((error = av_write_trailer(output_format_context)) < 0) {
            fprintf(stderr, "Could not write output file trailer (error '%s')\n",
                    av_err2str(error));
            return error;
        }
        return 0;
    }

    void AudioDealProduce::get_total_Package()
    {
        total_package_count = 0;
        AVFormatContext *ifmt_ctx = NULL;
        AVPacket pkt;
        int ret = -1;

        string file = params->srcFile;
        LOGI("AudioDealProduce::get_total_Package = %s", file.c_str());
        if ((ret = avformat_open_input(&ifmt_ctx, file.c_str(), 0, 0)) < 0) {
            LOGE("Could not open input file '%s'", file.c_str());
            return ;
        }
        LOGI("AudioDealProduce::get_total_Package = 11");
        if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
            LOGE("Failed to retrieve input stream information");
            return ;
        }
        LOGI("AudioDealProduce::get_total_Package = 22");
        int _audio_index = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
        if (_audio_index == -1) {
            LOGD("can't find audio stream in %s\n", ifmt_ctx->filename);
            return ;
        }
        double currTime = 0;
        if (params->rangeStart > 0) {
            av_seek_frame(ifmt_ctx, _audio_index, params->rangeStart / 1000 * AV_TIME_BASE,
                          AVSEEK_FLAG_BACKWARD);
        }
        LOGI("AudioDealProduce::get_total_Package = 33");
        LOGI("AudioDealProduce::get_total_Package starttime = %d", params->rangeStart);
        LOGI("AudioDealProduce::get_total_Package endtime = %d", params->rangeEnd);

        while (1) {
            ret = av_read_frame(ifmt_ctx, &pkt);
            if (ret < 0) {
                break;
            }
            currTime = av_q2d(ifmt_ctx->streams[_audio_index]->time_base) * pkt.pts * 1000;
            if (currTime > params->rangeEnd) {
                break;
            }
            //LOGI("AudioDealProduce::GetFrameCount currTime = %lf", currTime);
            if (pkt.stream_index == _audio_index) {
                total_package_count++;
            }
            av_packet_unref(&pkt);
        }
        avio_closep(&ifmt_ctx->pb);
    }

    int AudioDealProduce::convert_frame_to_fifo(AVAudioFifo *fifo, AVFrame *frame, AVFilterContext *sink, AVFilterContext *src)
    {
        int ret;
        AVFrame *filt_frame;

        av_log(NULL, AV_LOG_INFO, "Pushing decoded frame to filters\n");
        /* push the decoded frame into the filtergraph */
        ret = av_buffersrc_add_frame_flags(src, frame, AV_BUFFERSRC_FLAG_PUSH);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
            av_frame_free(&frame);
            return ret;
        }

        /* pull filtered frames from the filtergraph */
        while (1) {
            filt_frame = av_frame_alloc();
            if (!filt_frame) {
                ret = AVERROR(ENOMEM);
                break;
            }
            av_log(NULL, AV_LOG_INFO, "Pulling filtered frame from filters\n");
            ret = av_buffersink_get_frame(sink, filt_frame);
            if (ret < 0) {
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    ret = 0;
                av_frame_free(&filt_frame);
                break;
            }

            filt_frame->pict_type = AV_PICTURE_TYPE_NONE;
            ret = add_samples_to_fifo(fifo, filt_frame->extended_data, filt_frame->nb_samples);
            if (ret != 0) {
                av_frame_free(&filt_frame);
                break;
            }
            av_frame_free(&filt_frame);
        }

        return ret;
    }

    void AudioDealProduce::NoDecodeEncode() {
        AVFormatContext *ifmt_ctx = NULL;
        AVFormatContext *ofmt_ctx = NULL;
        AVPacket pkt;
        int ret = -1;
        AVStream *in_stream, *out_stream;
        int64_t dts_start_from;
        int64_t pts_start_from;
        OutputStreamContext *a_out_ctx = NULL;
        double currtime= 0;
        const char *srcFile =  params->srcFile.c_str();
        const char *outFile =  params->outFile.c_str();
        int tmp_package_count = 0;

        av_register_all();
        get_total_Package();
        if (total_package_count <= 0) {
            LOGI("AudioDealProduce  NoDecodeEncodetotal_package_count = %d", total_package_count);
            goto cleanUp;
        }
        LOGI("AudioDealProduce  NoDecodeEncode  11");
        if ((ret = avformat_open_input(&ifmt_ctx, srcFile, 0, 0)) < 0) {
            LOGE("Could not open input file '%s'", srcFile);
            goto cleanUp;
        }
        LOGI("AudioDealProduce  NoDecodeEncode  22");
        if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
            LOGE("Failed to retrieve input stream information");
            goto cleanUp;
        }
        LOGI("AudioDealProduce  NoDecodeEncode  33");
        a_out_ctx = applyOutputStreamContext(ifmt_ctx, outFile, AVMEDIA_TYPE_AUDIO);
        if (a_out_ctx == NULL) {
            LOGD("apply audio OutputStreamContext fail");
            ret = -1;
            goto cleanUp;
        }
        LOGI("AudioDealProduce  NoDecodeEncode  44");
        dts_start_from = 0;
        pts_start_from = 0;
        if (params->effect) {
            params->effect->onProgress(100, 0);
        }
        LOGI("AudioDealProduce  NoDecodeEncode total_package_count = %d", total_package_count);

        while (1) {
            ret = av_read_frame(ifmt_ctx, &pkt);
            if (ret < 0) {
                break;
            }
            if (pkt.stream_index != a_out_ctx->streamIndex) {
                continue;
            }
            //LOGI("AudioDealProduce  NoDecodeEncode 5555555");
            in_stream = ifmt_ctx->streams[pkt.stream_index];
            out_stream = a_out_ctx->stream;
            ofmt_ctx = a_out_ctx->fmt_ctx;
            currtime = av_q2d(in_stream->time_base) * pkt.pts * 1000;
            if (currtime > params->rangeEnd) {
                av_packet_unref(&pkt);
                break;
            }

            tmp_package_count++;
            LOGI("AudioDealProduce  NoDecodeEncode tmp_package_count = %d", tmp_package_count);
            if (params->effect) {
                params->effect->onProgress(100, (tmp_package_count*100 / total_package_count));
            }

            if (dts_start_from == 0) {
                dts_start_from= pkt.dts;
            }
            if (pts_start_from == 0) {
                pts_start_from = pkt.pts;
            }

            int64_t tempPts = pkt.pts - pts_start_from;
            int64_t tempDts = pkt.dts - dts_start_from;
            pkt.pts = av_rescale_q_rnd(tempPts, in_stream->time_base, out_stream->time_base,
                                       (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pkt.dts = av_rescale_q_rnd(tempDts, in_stream->time_base, out_stream->time_base,
                                       (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            if (pkt.pts < 0) {
                pkt.pts = 0;
            }
            if (pkt.dts < 0) {
                pkt.dts = 0;
            }
            pkt.duration = (int) av_rescale_q((int64_t) pkt.duration, in_stream->time_base,
                                              out_stream->time_base);
            pkt.pos = -1;
            pkt.stream_index = 0;
            //LOGI("AudioDealProduce  NoDecodeEncode 555555555555555");
            ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
            if (ret < 0) {
                av_packet_unref(&pkt);
                goto cleanUp;
            }
            av_packet_unref(&pkt);
        }
        av_write_trailer(ofmt_ctx);
        ret = 0;

        cleanUp:
        if (params->effect != NULL) {
            if (ret == 0) {
                params->effect->onProgress(100, 100);
                params->effect->onSuccess();
            } else {
                params->effect->onFail();
            }
        }
        avformat_close_input(&ifmt_ctx);
        avio_closep(&ofmt_ctx->pb);
        avformat_free_context(ofmt_ctx);

        LOGI("AudioDealProduce  NoDecodeEncode ret = %d", ret);
    }

    void AudioDealProduce::DoDeal() {

        int ret = AVERROR_EXIT;
        const char *intputfilename = params->srcFile.c_str();
        const char *outputfilename = params->outFile.c_str();
        int finished                = 0;
        AVFrame *input_frame = NULL;
        double currtime = 0;
        char filter_spec[64];
        long seektime = 0;
        string tmps;

        LOGI("AudioDealProduce   AudioDealProduce::DoDeal() intputfilename = %s", intputfilename);
        LOGI("AudioDealProduce   AudioDealProduce::DoDeal() outputfilename = %s", outputfilename);
        long starttime = params->rangeStart;
        long endtime = params->rangeEnd;
        LOGI("AudioDealProduce   AudioDealProduce::DoDeal() starttime = %ld", starttime);
        LOGI("AudioDealProduce   AudioDealProduce::DoDeal() endtime = %ld", endtime);

        av_register_all();
        avfilter_register_all();

        get_total_Package();
        if (total_package_count <= 0) {
            LOGI("AudioDealProduce   AudioDealProduce::DoDeal() total_package_count", total_package_count);
            goto cleanup;
        }

        if (open_input_file(intputfilename, &input_format_context,
                            &input_codec_context))
            goto cleanup;

        if (open_output_file(outputfilename, input_codec_context,
                             &output_format_context, &output_codec_context))
            goto cleanup;


        if (init_fifo(&fifo, output_codec_context))
            goto cleanup;

        if (write_output_file_header(output_format_context))
            goto cleanup;

        if (init_input_frame(&input_frame) != 0)
            goto cleanup;

        if (params->rangeStart > 0) {
            av_seek_frame(input_format_context, g_audio_index, params->rangeStart / 1000 * AV_TIME_BASE,
                          AVSEEK_FLAG_BACKWARD);
        }
        if (params->effect) {
            params->effect->init(input_format_context, output_format_context, NULL, NULL,
                                 input_format_context->streams[g_audio_index], 0, params->rangeStart, params->rangeEnd);
        }

        sprintf(filter_spec, "anull");
        if (params->effect) {
            tmps = params->effect->filterString();
            if (tmps.c_str() != NULL) {
                sprintf(filter_spec, "%s", tmps.c_str());
            }
        }
        if (init_filter(input_codec_context, output_codec_context, filter_spec))
            goto cleanup;

        while (1) {
            finished                =  0;
            for (;;) {
                int ret;
                AVPacket in_pkt;
                init_packet(&in_pkt);

                ret = av_read_frame(input_format_context, &in_pkt);
                if (ret < 0) {
                    finished = 1;
                    break;
                }

                if (in_pkt.size <= 0 ) {
                    LOGI("AudioDealProduce in_pkt.size");
                    av_packet_unref(&in_pkt);
                    continue;
                }
                AVStream *st = input_format_context->streams[in_pkt.stream_index];
                if (in_pkt.stream_index != g_audio_index) {
                    //printf("Found another stream#%d:", in_pkt.stream_index);
                    //printf("codec_id=%d#codec_type=%d#", st->codecpar->codec_id, st->codecpar->codec_type);
                    if (st->disposition & AV_DISPOSITION_ATTACHED_PIC && st->discard < AVDISCARD_ALL) {
                        //printf("found APIC\n");
                    }
                    av_packet_unref(&in_pkt);
                    continue;
                }

                if (!decode_pkt_flag) {
                    package_count++;
                }
                if (params->effect) {
                    params->effect->onProgress(100, (package_count * 100 / total_package_count));
                }
                currtime = av_q2d(input_codec_context->time_base) * in_pkt.pts * 1000;

                if (currtime > 1.0*params->rangeEnd) {
                    if (params->effect) {
                        seektime = params->effect->getNeedSeekTime(in_pkt);
                        LOGI("AudioDealProduce 0000000000000 currtime = %lf", currtime);
                        LOGI("AudioDealProduce 0000000000000 seektime = %ld", seektime);
                        if (seektime >= 0) {
                            int seekret = av_seek_frame(input_format_context, -1,
                                          seektime / 1000 * AV_TIME_BASE,
                                          AVSEEK_FLAG_BACKWARD);
                            LOGI("AudioDealProduce seekret = %d", seekret);

                            continue;
                        }

                    }
                }
                    //g_audio_index
                if (params->effect ) {
                    seektime = params->effect->getNeedSeekTime(in_pkt);
                    //LOGI("AudioDealProduce 11111111111 seektime = %ld", seektime);
                    if (seektime >= 0) {
                        decode_pkt_flag = true;
                        LOGI("AudioDealProduce 111111111112 currtime = %lf", currtime);
                        LOGI("AudioDealProduce 111111111112 seektime = %ld", seektime);
                        int seekret = av_seek_frame(input_format_context, -1, seektime / 1000 * AV_TIME_BASE,
                                      AVSEEK_FLAG_BACKWARD);
                        LOGI("AudioDealProduce seekret = %d", seekret);
                        continue;
                    }
                }

                if (params->effect != NULL) {
                    bool newfilter = params->effect->getNeedNewFilter(in_pkt);
                    if (newfilter) {
                        string s = params->effect->filterString();
                        if (s.c_str() != NULL) {
                            sprintf(filter_spec, "%s", s.c_str());
                            init_filter(input_codec_context, output_codec_context, filter_spec);
                        }
                        if (decode_pkt_flag) {
                            decode_pkt_flag = false;
                        }
                    }
                }
                LOGI("AudioDealProduce currtime = %lf", currtime);

                ret = avcodec_send_packet(input_codec_context, &in_pkt);
                if (ret == AVERROR_EOF) {
                    av_packet_unref(&in_pkt);
                    continue;
                }
                if (ret < 0 && ret != AVERROR(EAGAIN)) {
                    av_packet_unref(&in_pkt);
                    //finished = 1;
                    LOGI("AudioDealProduce avcodec_send_packet error");
                    break;
                }

                while (1) {
                    ret = avcodec_receive_frame(input_codec_context, input_frame);
                    if (ret < 0)
                        break;

                    ret = convert_frame_to_fifo(fifo, input_frame, buffersink_ctx, buffersrc_ctx);
                    if (ret < 0)
                        break;
                }

                av_packet_unref(&in_pkt);
                if (av_audio_fifo_size(fifo) >= g_enc_frame_size) {
                    break;
                }
            }
//=========================================================//
//=========================================================//

            while (av_audio_fifo_size(fifo) >= g_enc_frame_size || (finished && av_audio_fifo_size(fifo) > 0)) {
                if (load_encode_and_write(fifo, output_format_context,
                                          output_codec_context, &finished))
                    goto cleanup;
                if (finished) {
                    LOGI("AudioDealProduce load_encode_and_write finish");
                    break;
                }
            }
            if (finished) {
                flush_encoders(output_format_context, output_codec_context);
                break;
            }
        }

        av_dict_copy(&output_format_context->metadata, input_format_context->metadata, AV_DICT_DONT_OVERWRITE);
        if (write_output_file_trailer(output_format_context))
            goto cleanup;
        ret = 0;

        goto cleanup;
        cleanup:
        av_frame_free(&input_frame);

        LOGI("AudioDealProduce finish ret = %d", ret);
        if (fifo)
            av_audio_fifo_free(fifo);
//    swr_free(&resample_context);
        if (output_codec_context)
            avcodec_free_context(&output_codec_context);
        if (output_format_context) {
            avio_closep(&output_format_context->pb);
            avformat_free_context(output_format_context);
        }
        if (input_codec_context)
            avcodec_free_context(&input_codec_context);
        if (filter_graph)
            avfilter_graph_free(&filter_graph);

        if (input_format_context)
            avformat_close_input(&input_format_context);

        if (filter_graph) {
            avfilter_graph_free(&filter_graph);
        }
        if (params->effect != NULL) {
            if (ret == 0) {
                params->effect->onProgress(100, 100);
                params->effect->onSuccess();
            } else {
                params->effect->onFail();
            }
        }
    }

    void AudioDealProduce::Start() {
        if (params->effect) {
            params->effect->onProgress(100, 0);
        }
        LOGI("AudioDealProduce   AudioDealProduce::Start()");
        if (this->params->effect->needDecode()) {  /*   true  */
            LOGI("AudioDealProduce   Start needDecode");
            ThreadExecuteAudioParams1 *executeParams = new ThreadExecuteAudioParams1();
            executeParams->methodId = AUDIO_METHOD_ID_START;
            executeParams->produce = this;
            pthread_create(&mutex_start_thread, NULL, threatExecute, executeParams);
            return ;
        } else {
            LOGI("AudioDealProduce   Start NoNeedDecode");
            ThreadExecuteAudioParams1 *executeParams = new ThreadExecuteAudioParams1();
            executeParams->methodId = AUDIO_METHOD_ID_NODECODE_ENCODE;
            executeParams->produce = this;
            pthread_create(&mutex_start_thread, NULL, threatExecute, executeParams);
            return ;
        }
    }

    static void *threatExecute(void *arg) {
        ThreadExecuteAudioParams1 *params = (ThreadExecuteAudioParams1 *) arg;
        switch (params->methodId) {
            case AUDIO_METHOD_ID_START:
                params->produce->DoDeal();
                break;
            case AUDIO_METHOD_ID_NODECODE_ENCODE:
                params->produce->NoDecodeEncode();
                break;
        }

        return NULL;
    }
}
