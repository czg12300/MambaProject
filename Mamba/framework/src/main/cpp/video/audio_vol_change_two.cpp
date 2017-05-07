
#include "audioTranscode.h"
#include "audioVolumn.h"
static AVFormatContext *ifmt_ctx;
static AVCodecContext *pDecodeCtx;
static int _index_first = -1;
static AVFilterGraph *_filter_graph = NULL;
static AVFilterContext *_filter_ctx_src_first = NULL;
static AVFilterContext *_filter_ctx_sink = NULL;

static int64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
static enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
static int out_sample_rate = 44100;
static int swrInitFlag = 0;
#define AV_LOG av_log


static int open_input_file(const char *filename) {
    int ret;
    unsigned int i;
    AVCodecParameters *pAVCodecParameters;
    AVCodec *codec;
    ifmt_ctx = NULL;

    if ((ret = avformat_open_input(&ifmt_ctx, filename, NULL, NULL)) < 0) {
        LOGE("Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
        LOGE("Cannot find stream information\n");
        return ret;
    }

    _index_first = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    if (ret < 0) {
        LOGD("Cannot find a video stream in the input file\n");
        return -1;
    }
    pAVCodecParameters = avcodec_parameters_alloc();
    if (!pAVCodecParameters) {
        LOGE("无法avcodec_parameters_alloc");
        return ret;
    }
    if ((ret = avcodec_parameters_copy(pAVCodecParameters,
                                       ifmt_ctx->streams[_index_first]->codecpar)) < 0) {
        LOGE("avcodec_parameters_copy error \n");
        avcodec_parameters_free(&pAVCodecParameters);
        return ret;
    }
    pDecodeCtx = avcodec_alloc_context3(codec);
    if (!pDecodeCtx) {
        avcodec_parameters_free(&pAVCodecParameters);
        LOGE("avcodec_alloc_context3 ERROR");
        return ret;
    }
    if ((ret = avcodec_parameters_to_context(pDecodeCtx, pAVCodecParameters)) < 0) {
        LOGE("avcodec_parameters_to_context < 0");
        avcodec_parameters_free(&pAVCodecParameters);
        return ret;
    }
    avcodec_parameters_free(&pAVCodecParameters);
    if ((ret = avcodec_open2(pDecodeCtx, codec, NULL)) < 0) {
        LOGI("无法打开解码器");
        return ret;
    }
    av_dump_format(ifmt_ctx, 0, filename, 0);
    if (swrInitFlag == 0) {
        initSwr(pDecodeCtx->channel_layout, pDecodeCtx->sample_fmt, pDecodeCtx->sample_rate);
        swrInitFlag = swrInitFlag + 1;
    }
    return 0;
}

static int init_filter(char *filter_desc, AVCodecContext *encodec_ctx) {
    char args_spk[512];
    char *pad_name_first = "in";

    AVFilter *filter_src_first = avfilter_get_by_name("abuffer");
    AVFilter *filter_sink = avfilter_get_by_name("abuffersink");
    AVFilterInOut *filter_output_first = avfilter_inout_alloc();
    AVFilterInOut *filter_input = avfilter_inout_alloc();
    _filter_graph = avfilter_graph_alloc();

    snprintf(args_spk, sizeof(args_spk),
             "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%llu",
             encodec_ctx->time_base.num,
             encodec_ctx->time_base.den,
             encodec_ctx->sample_rate,
             av_get_sample_fmt_name(encodec_ctx->sample_fmt),
             encodec_ctx->channel_layout);

    int ret = 0;
    ret = avfilter_graph_create_filter(&_filter_ctx_src_first, filter_src_first, pad_name_first,
                                       args_spk, NULL, _filter_graph);
    if (ret < 0) {
        LOGE("Filter: failed to call avfilter_graph_create_filter");
        return ret;
    }

    ret = avfilter_graph_create_filter(&_filter_ctx_sink, filter_sink, "out", NULL, NULL,
                                       _filter_graph);
    if (ret < 0) {
        LOGE("Filter: failed to call avfilter_graph_create_filter");
        return ret;
    }
    ret = av_opt_set_bin(_filter_ctx_sink, "sample_fmts", (uint8_t *) &encodec_ctx->sample_fmt,
                         sizeof(encodec_ctx->sample_fmt), AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        LOGE("Filter: failed to call av_opt_set_bin");
        return ret;
    }
    ret = av_opt_set_bin(_filter_ctx_sink, "channel_layouts",
                         (uint8_t *) &encodec_ctx->channel_layout,
                         sizeof(encodec_ctx->channel_layout), AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        LOGE("Filter: failed to call av_opt_set_bin -- channel_layouts");
        return ret;
    }
    ret = av_opt_set_bin(_filter_ctx_sink, "sample_rates", (uint8_t *) &encodec_ctx->sample_rate,
                         sizeof(encodec_ctx->sample_rate), AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        LOGE("Filter: failed to call av_opt_set_bin -- sample_rates");
        return ret;
    }

    filter_output_first->name = av_strdup(pad_name_first);
    filter_output_first->filter_ctx = _filter_ctx_src_first;
    filter_output_first->pad_idx = 0;
    filter_output_first->next = NULL;

    filter_input->name = av_strdup("out");
    filter_input->filter_ctx = _filter_ctx_sink;
    filter_input->pad_idx = 0;
    filter_input->next = NULL;

    AVFilterInOut *filter_outputs[1];
    filter_outputs[0] = filter_output_first;

    ret = avfilter_graph_parse_ptr(_filter_graph, filter_desc, &filter_input, filter_outputs, NULL);
    if (ret < 0) {
        LOGE("Filter: failed to call avfilter_graph_parse_ptr");
        return ret;
    }

    ret = avfilter_graph_config(_filter_graph, NULL);
    if (ret < 0) {
        LOGE("Filter: failed to call avfilter_graph_config");
        return ret;
    }

    avfilter_inout_free(&filter_input);
    av_free(filter_src_first);
    avfilter_inout_free(filter_outputs);

    return 0;
}

int audio_vol_change_Init(AVCodecContext *encodec_ctx, double proportion = 1.0) {
    char filter_desc[24];
    int ret = 0;
    sprintf(filter_desc, "volume=%.2lf", proportion);
    ret = init_filter(filter_desc, encodec_ctx);
    return ret;
}

int audio_vol_change_DeInit() {
    if (_filter_ctx_src_first != NULL)
        avfilter_free(_filter_ctx_src_first);
    if (_filter_ctx_sink != NULL)
        avfilter_free(_filter_ctx_sink);
    if (_filter_graph != NULL)
        avfilter_graph_free(&_filter_graph);
    return 0;
}

int audio_vol_change2(AVFrame *in_frame, AVFrame *out_frame) {
    int ret = av_buffersrc_add_frame(_filter_ctx_src_first, in_frame);
    if (ret < 0) {
        LOGE("Mixer: failed to call av_buffersrc_add_frame");
        return ret;
    }
    ret = av_buffersink_get_frame_flags(_filter_ctx_sink, out_frame, 0);
    if (ret < 0) {
        LOGE("Mixer: failed to call av_buffersink_get_frame_flags");
    }
    return ret;
}

int audio_vol_change(const char *src_filename, const char *dst_filename, double proportion = 1.0) {
    int ret = -1;
    int got_sound;
    AVPacket packet;
    int64_t frame_count = 0;
    static AVFrame *pFrame;
    static AVFrame *transcodeFrame;
    static FILE *fp0;
    av_register_all();
    avfilter_register_all();
    fp0 = fopen(dst_filename, "wb+");
    ret = open_input_file(src_filename);
    if (ret < 0) {
        goto Fished;
    }
    if ((ret = audio_vol_change_Init(pDecodeCtx, proportion)) < 0) {
        goto Fished;
    }
    LOGI("11111111111111111");
    pFrame = av_frame_alloc();
    transcodeFrame = av_frame_alloc();
    av_init_packet(&packet);
    LOGI("22222222222222");
    while (true) {
        packet.data = NULL;
        packet.size = 0;
        if (av_read_frame(ifmt_ctx, &packet) < 0) {
            LOGI("333333333333333");
            break;
        }

        if (packet.stream_index == _index_first) {
            if (avcodec_decode_audio4(pDecodeCtx, pFrame, &got_sound,
                                      &packet) < 0) {
                av_free_packet(&packet);
                LOGI("4444444444444");
                break;
            }
            av_free_packet(&packet);
            if (!got_sound) {
                LOGI("555555555555555");
                continue;
            }
            AVFrame *pFrame_out = av_frame_alloc();
            if (pFrame) {
                audio_vol_change2(pFrame, pFrame_out);
            }
            if (pFrame_out) {
                if (av_sample_fmt_is_planar((AVSampleFormat) pDecodeCtx->sample_fmt)) {
                    pFrame_out->pts = av_frame_get_best_effort_timestamp(pFrame_out);
                    if (0 !=
                        TransSample(pFrame_out, transcodeFrame, pDecodeCtx->sample_fmt)) {
                        LOGE("can not swr the audio data!\n");
                        goto Fished;
                    }
                    if (transcodeFrame) {
                        fwrite(transcodeFrame->data[0], 1, transcodeFrame->linesize[0], fp0);
                        LOGI("fwrite %d", transcodeFrame->linesize[0]);
                    }
                } else {
                    fwrite(pFrame_out->data[0], 1, pFrame_out->linesize[0], fp0);
                    LOGI("fwrite %d", pFrame_out->linesize[0]);
                }
            }
            av_frame_free(&pFrame_out);
        }
    }
    Fished:
    if (swrInitFlag == 1) {
        deInitSwr();
        swrInitFlag = 0;
    }
    if (pFrame != NULL) {
        av_frame_free(&pFrame);
    }
    if (transcodeFrame != NULL) {
        av_frame_free(&transcodeFrame);
    }
    if (fp0 != NULL) {
        fclose(fp0);
    }
    audio_vol_change_DeInit();
    avformat_close_input(&ifmt_ctx);
    return 0;
}