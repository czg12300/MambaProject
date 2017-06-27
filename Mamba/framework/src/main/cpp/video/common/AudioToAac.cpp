//
// Created by bearshi on 2017/6/13.
//

#include "AudioToAac.h"
#include "log.h"

namespace video {

    AudioToAac::AudioToAac() {
        ifmt_ctx = NULL;
        ofmt_ctx = NULL;
        _index_in_audio = -1;
        _index_out_audio = 0;
        _fifo_src = NULL;
        _filter_graph = NULL;
        _filter_ctx_src = NULL;
        _filter_ctx_sink = NULL;
    }

    AudioToAac::~AudioToAac() {
        if (_fifo_src) {
            av_audio_fifo_free(_fifo_src);
        }
        if (_filter_ctx_src) {
            avfilter_free(_filter_ctx_src);
        }
        if (_filter_ctx_sink) {
            avfilter_free(_filter_ctx_sink);
        }
        if (_filter_graph) {
            avfilter_graph_free(&_filter_graph);
        }
        if (ofmt_ctx) {
            avio_closep(&ofmt_ctx->pb);
            avformat_free_context(ofmt_ctx);
        }
        if (ifmt_ctx) {
            avformat_close_input(&ifmt_ctx);
        }
    }

    int AudioToAac::open_input_file(const char *filename)
    {
        int ret;
        unsigned int i;
        ifmt_ctx = NULL;
        if ((ret = avformat_open_input(&ifmt_ctx, filename, NULL, NULL)) < 0) {
            LOGI("AudioToAac     Cannot open input file\n");
            return ret;
        }
        if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
            LOGI("AudioToAac     Cannot find stream information\n");
            return ret;
        }
        for (i = 0; i < ifmt_ctx->nb_streams; i++) {
            AVStream *stream;
            AVCodecContext *codec_ctx;
            stream = ifmt_ctx->streams[i];
            codec_ctx = stream->codec;

            if (codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
                _index_in_audio = i;

                ret = avcodec_open2(codec_ctx,
                                    avcodec_find_decoder(codec_ctx->codec_id), NULL);
                if (ret < 0) {
                    LOGI("AudioToAac     Failed to open decoder for stream #%u\n", i);
                    return ret;
                }
            }
        }
        av_dump_format(ifmt_ctx, 0, filename, 0);

        return 0;
    }

    int AudioToAac::open_output_file(const char *filename)
    {
        int ret = 0;
        LOGI("AudioToAac     AudioToAac open_output_file start");
        ret = avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
        if (ret < 0) {
            LOGI("AudioToAac     failed to call avformat_alloc_output_context2");
            return ret;
        }

        AVStream* stream_a = NULL;
        stream_a = avformat_new_stream(ofmt_ctx, NULL);
        if (stream_a == NULL) {
            LOGI("AudioToAac     failed to call avformat_new_stream");
            return ret;
        }

        stream_a->codec->codec_type = AVMEDIA_TYPE_AUDIO;
        AVCodec* codec_aac = avcodec_find_encoder(AV_CODEC_ID_AAC);
        stream_a->codec->codec = codec_aac;
        stream_a->codec->sample_rate = ifmt_ctx->streams[_index_in_audio]->codec->sample_rate;  //44100;//
        stream_a->codec->channels = ifmt_ctx->streams[_index_in_audio]->codec->channels;  //2
        stream_a->codec->channel_layout = av_get_default_channel_layout(stream_a->codec->channels);
        stream_a->codec->sample_fmt = codec_aac->sample_fmts[0];
        stream_a->codec->bit_rate = ifmt_ctx->streams[_index_in_audio]->codec->bit_rate;  //57000;//57000;//
        stream_a->codec->time_base.num = 1;
        stream_a->codec->time_base.den = stream_a->codec->sample_rate;
        stream_a->codec->codec_tag = 0;
        /*
        LOGI("_index_in_audio = %d", _index_in_audio);
        LOGI("stream_a->codec->sample_rate = %d", stream_a->codec->sample_rate);
        LOGI("stream_a->codec->bit_rate = %lld",  stream_a->codec->bit_rate);
        LOGI("stream_a->codec->channels = %d",  stream_a->codec->channels);
        LOGI("stream_a->codec->channels = %lld",  ifmt_ctx->streams[_index_in_audio]->codec->bit_rate);
        */
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            stream_a->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
        LOGI("AudioToAac     avcodec_open2  111\n");
        if (avcodec_open2(stream_a->codec, stream_a->codec->codec, NULL) < 0) {
            LOGI("failed to call avcodec_open2");
            return ret;
        }
        LOGI("AudioToAac     avcodec_open2  222\n");
        if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
            if (avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE) < 0) {
                LOGI("AudioToAac    failed to call avio_open");
                return ret;
            }
        }

        if (avformat_write_header(ofmt_ctx, NULL) < 0) {
            LOGI("AudioToAac    failed to call avformat_write_header");
            return ret;
        }
        _index_out_audio = 0;
        LOGI("AudioToAac     avcodec_open  finish\n");
        av_dump_format(ofmt_ctx, _index_out_audio, filename, 1);
        return 0;
    }

    int AudioToAac::init_filter(char* filter_desc)
    {
        char args_spk[512];
        char pad_name_first[3] = "in";

        AVFilter *filter_src = avfilter_get_by_name("abuffer");
        AVFilter *filter_sink = avfilter_get_by_name("abuffersink");
        AVFilterInOut* filter_output = avfilter_inout_alloc();
        AVFilterInOut* filter_input = avfilter_inout_alloc();
        _filter_graph = avfilter_graph_alloc();

        snprintf(args_spk, sizeof(args_spk), "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%llu",
                 ifmt_ctx->streams[_index_in_audio]->codec->time_base.num,
                 ifmt_ctx->streams[_index_in_audio]->codec->time_base.den,
                 ifmt_ctx->streams[_index_in_audio]->codec->sample_rate,
                 av_get_sample_fmt_name(ifmt_ctx->streams[_index_in_audio]->codec->sample_fmt),
                 ifmt_ctx->streams[_index_in_audio]->codec->channel_layout);

        int ret = 0;
        ret = avfilter_graph_create_filter(&_filter_ctx_src, filter_src, pad_name_first, args_spk, NULL, _filter_graph);
        if (ret < 0) {
            LOGI("AudioToAac    failed to call avfilter_graph_create_filter");
            return ret;
        }

        ret = avfilter_graph_create_filter(&_filter_ctx_sink, filter_sink, "out", NULL, NULL, _filter_graph);
        if (ret < 0) {
            LOGI("AudioToAac    failed to call avfilter_graph_create_filter");
            return ret;
        }
        AVCodecContext* encodec_ctx = ofmt_ctx->streams[_index_out_audio]->codec;
        ret = av_opt_set_bin(_filter_ctx_sink, "sample_fmts", (uint8_t*)&encodec_ctx->sample_fmt,
                             sizeof(encodec_ctx->sample_fmt), AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            LOGI("AudioToAac    failed to call av_opt_set_bin");
            return ret;
        }
        ret = av_opt_set_bin(_filter_ctx_sink, "channel_layouts", (uint8_t*)&encodec_ctx->channel_layout,
                             sizeof(encodec_ctx->channel_layout), AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            LOGI("AudioToAac    failed to call av_opt_set_bin -- channel_layouts");
            return ret;
        }
        ret = av_opt_set_bin(_filter_ctx_sink, "sample_rates", (uint8_t*)&encodec_ctx->sample_rate,
                             sizeof(encodec_ctx->sample_rate), AV_OPT_SEARCH_CHILDREN);
        if (ret < 0) {
            LOGI("AudioToAac    failed to call av_opt_set_bin -- sample_rates");
            return ret;
        }

        filter_output->name = av_strdup(pad_name_first);
        filter_output->filter_ctx = _filter_ctx_src;
        filter_output->pad_idx = 0;
        filter_output->next = NULL;

        filter_input->name = av_strdup("out");
        filter_input->filter_ctx = _filter_ctx_sink;
        filter_input->pad_idx = 0;
        filter_input->next = NULL;

        AVFilterInOut* filter_outputs[1];
        filter_outputs[0] = filter_output;

        ret = avfilter_graph_parse_ptr(_filter_graph, filter_desc, &filter_input, filter_outputs, NULL);
        if (ret < 0) {
            LOGI("AudioToAac    failed to call avfilter_graph_parse_ptr");
            return -1;
        }

        ret = avfilter_graph_config(_filter_graph, NULL);
        if (ret < 0) {
            LOGI("AudioToAac    failed to call avfilter_graph_config");
            return -1;
        }

        avfilter_inout_free(&filter_input);
        avfilter_inout_free(filter_outputs);
        //av_free(filter_src);

        return 0;
    }

    int AudioToAac::init_fifo()
    {
        _fifo_src = av_audio_fifo_alloc(ifmt_ctx->streams[_index_in_audio]->codec->sample_fmt,
                                        ifmt_ctx->streams[_index_in_audio]->codec->channels,
                                        2*ifmt_ctx->streams[_index_in_audio]->codec->frame_size);
        if (!_fifo_src) {
            return AVERROR(ENOMEM);
        }

        return 0;
    }

    int AudioToAac::init_input_frame(AVFrame **frame)
    {
        if (!(*frame = av_frame_alloc())) {
            return AVERROR(ENOMEM);
        }
        return 0;
    }

    int AudioToAac::read_decode_to_fifo(int *finished)
    {
        AVPacket input_packet;
        int ret;
        int data_present;
        AVFrame *input_frame = NULL;
        if (init_input_frame(&input_frame))
            goto cleanup1;

        av_init_packet(&input_packet);

        if ((ret = av_read_frame(ifmt_ctx, &input_packet)) < 0) {
            if (ret == AVERROR_EOF) {
                ret = 0;
                *finished = 1;
            }
            goto cleanup1;
        }

        if (input_packet.stream_index != _index_in_audio) {
            av_packet_unref(&input_packet);
            return 0;
        }

        if ((ret = avcodec_decode_audio4(ifmt_ctx->streams[_index_in_audio]->codec, input_frame,
                                         &data_present, &input_packet)) < 0) {
            av_packet_unref(&input_packet);
            goto cleanup1;
        }
        if (*finished && data_present)
            *finished = 0;
        av_packet_unref(&input_packet);

        if (data_present) {
            int nwritedata = av_audio_fifo_write(_fifo_src, (void**)input_frame->data,
                                                 input_frame->nb_samples);
            if (nwritedata < ifmt_ctx->streams[_index_in_audio]->codec->frame_size) {
                goto cleanup1;
            }
        }
        ret = 0;
        cleanup1:
        av_frame_free(&input_frame);

        return ret;
    }

    int AudioToAac::init_output_frame(AVFrame **frame, int frame_size)
    {
        int error;

        if (!(*frame = av_frame_alloc())) {
            LOGI("AudioToAac    Could not allocate output frame\n");
            return AVERROR_EXIT;
        }

        (*frame)->nb_samples     = frame_size;
        (*frame)->channel_layout = ifmt_ctx->streams[_index_in_audio]->codec->channel_layout;
        (*frame)->format         = ifmt_ctx->streams[_index_in_audio]->codec->sample_fmt;
        (*frame)->sample_rate    = ifmt_ctx->streams[_index_in_audio]->codec->sample_rate;

        if ((error = av_frame_get_buffer(*frame, 0)) < 0) {
            av_frame_free(frame);
            return error;
        }

        return 0;
    }

    int AudioToAac::write_encode_from_fifo(int *frame_count)
    {
        AVFrame *output_frame;
        int ret;
        const int frame_size = FFMIN(av_audio_fifo_size(_fifo_src),
                                     ofmt_ctx->streams[_index_out_audio]->codec->frame_size);

        if (init_output_frame(&output_frame, frame_size)) {
            return AVERROR_EXIT;
        }
        if (av_audio_fifo_read(_fifo_src, (void **)output_frame->data, frame_size) < frame_size) {
            av_frame_free(&output_frame);
            return AVERROR_EXIT;
        }
        output_frame->pts = av_frame_get_best_effort_timestamp(output_frame);

        ret = av_buffersrc_add_frame(_filter_ctx_src, output_frame);
        if (ret < 0) {
            return ret;
        }

        AVPacket packet_out;
        int got_packet_ptr = 0;
        while (1) {
            AVFrame* pframe_result = av_frame_alloc();
            ret = av_buffersink_get_frame(_filter_ctx_sink, pframe_result);
            if (ret < 0) {
                ret = 0;
                break;
            }

            if (pframe_result->data[0] != NULL) {
                av_init_packet(&packet_out);
                packet_out.data = NULL;
                packet_out.size = 0;

                ret = avcodec_encode_audio2(ofmt_ctx->streams[_index_out_audio]->codec,
                                            &packet_out, pframe_result, &got_packet_ptr);
                if (ret < 0) {
                    break;
                }
#if use_write_encode_pcm
                fwrite(pframe_result->data[0] , 1, pframe_result->linesize[0], encode_pcm);
#endif

                if (got_packet_ptr) {
                    packet_out.stream_index = _index_out_audio;
                    packet_out.pts = (*frame_count) * ofmt_ctx->streams[_index_out_audio]->codec->frame_size;
                    packet_out.dts = packet_out.pts;
                    packet_out.duration = ofmt_ctx->streams[_index_out_audio]->codec->frame_size;

                    packet_out.pts = av_rescale_q_rnd(packet_out.pts,
                                                      ofmt_ctx->streams[_index_out_audio]->codec->time_base,
                                                      ofmt_ctx->streams[_index_out_audio]->time_base,
                                                      (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
                    packet_out.dts = packet_out.pts;
                    packet_out.duration = av_rescale_q_rnd(packet_out.duration,
                                                           ofmt_ctx->streams[_index_out_audio]->codec->time_base,
                                                           ofmt_ctx->streams[_index_out_audio]->time_base,
                                                           (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
                    (*frame_count)++;

                    ret = av_interleaved_write_frame(ofmt_ctx, &packet_out);
                    if (ret < 0) {
                        av_packet_unref(&packet_out);
                        break;
                    }
                }
                av_packet_unref(&packet_out);
            }
            av_frame_free(&pframe_result);
        }
        av_frame_free(&output_frame);

        return ret;
    }

    int AudioToAac::audioMp3ToAacBase(const char *inputfilename, const char *outputfilename)
    {
        int ret = -1;
        char filter_desc[128];
        int frame_count = 0;
        int finished = 0;

#if use_write_encode_pcm
        encode_pcm = fopen("/storage/emulated/0/AClip/audioAAC/encode.pcm", "wb+");
#endif
        sprintf(filter_desc, "anull");  //atempo=2.0
        av_register_all();
        avfilter_register_all();
        LOGI("AudioToAac    audioMp3ToAacBase inputfilename = %s\n", inputfilename);
        LOGI("AudioToAac    outputfilename = %s\n", outputfilename);
        ret = open_input_file(inputfilename);
        if (ret < 0) {
            goto cleanup;
        }
        ret = open_output_file(outputfilename);
        if (ret < 0) {
            LOGI("AudioToAac    open_output_file err\n");
            goto cleanup;
        }
        LOGI("AudioToAac    open_output_file\n");
        ret = init_filter(filter_desc);
        if (ret < 0) {
            LOGI("AudioToAac    init_filter err\n");
            goto cleanup;
        }
        ret = init_fifo();
        if (ret < 0) {
            LOGI("init_fifo err\n");
            goto cleanup;
        }
        LOGI("AudioToAac    filter success\n");
        finished = 0;
        while (1) {
            const int output_frame_size = ofmt_ctx->streams[_index_out_audio]->codec->frame_size;
            while (av_audio_fifo_size(_fifo_src) < output_frame_size) {
                if (read_decode_to_fifo(&finished)) {
                    goto cleanup;
                }
                if (finished)
                    break;
            }

            while (av_audio_fifo_size(_fifo_src) >= output_frame_size ||
                   (finished && av_audio_fifo_size(_fifo_src) > 0)) {
                if(write_encode_from_fifo(&frame_count)) {
                    goto cleanup;
                }
            }
            if (finished) {
                break;
            }
            //LOGI("current frame_count = %d\n", frame_count);
        }

        av_write_trailer(ofmt_ctx);
        ret = 0;
        cleanup:

#if use_write_encode_pcm
            fclose(encode_pcm);
#endif

        LOGI("finished frame_count = %d\n", frame_count);
        return ret;
    }


}

