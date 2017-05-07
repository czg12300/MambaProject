//
// Created by walljiang on 2017/04/07.
//

#include <HandleProgressCallback.h>
#include "audioRate.h"
#include "AudioRecord.h"
#include "audioTranscode.h"

static AVFormatContext *ifmt_ctx;
static AVCodecContext *decodeCtx = NULL;
static int _index_first = -1;
static AVFilterGraph *_filter_graph = NULL;
static AVFilterContext *_filter_ctx_src_first = NULL;
static AVFilterContext *_filter_ctx_sink = NULL;
static int64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
static enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
static int out_sample_rate = 44100;
static int swrInitFlag = 0;

static int open_input_file(const char *filename, const char *dst_filename) {
    LOGI("open_input_file %s\n", filename);
    int ret;
    unsigned int i;
    AVCodec *codec;
    ifmt_ctx = NULL;
    if ((ret = avformat_open_input(&ifmt_ctx, filename, NULL, NULL)) < 0) {
        LOGE("Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
        LOGE("Cannot find stream information\n");
        return -1;
    }

    _index_first = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    if (ret < 0) {
        LOGD("Cannot find a video stream in the input file\n");
        return -1;
    }
    AVCodecParameters *pAVCodecParameters = ifmt_ctx->streams[_index_first]->codecpar;
    //根据编解码信息查找解码器
    AVCodec *pCodec = avcodec_find_decoder(pAVCodecParameters->codec_id);
    if (pCodec == NULL) {
        LOGI("open_decodec_context()无法根据avstream找到decoder");
        return NULL;
    }

    decodeCtx = avcodec_alloc_context3(pCodec);
    if ((avcodec_parameters_to_context(decodeCtx, pAVCodecParameters)) < 0) {
        LOGI("open_decodec_context()无法根据pCodec分配AVCodecContext");
        return NULL;
    }
    //打开解码器
    if (avcodec_open2(decodeCtx, pCodec, NULL) < 0) {
        LOGI("open_decodec_context()无法打开编码器");
        return NULL;
    }
    if (decodeCtx == NULL) {
        LOGE("Could not open codec!");
        return -1;
    }
    if (swrInitFlag == 0) {
        initSwr(decodeCtx->channel_layout, decodeCtx->sample_fmt, decodeCtx->sample_rate);
        swrInitFlag = swrInitFlag + 1;
    }
    av_dump_format(ifmt_ctx, 0, filename, 0);
    return 0;
}

int init_filter(char *filter_desc) {
    LOGI("init_filter %s \n", filter_desc);
    char args_spk[512];
    char *pad_name_first = "in";
    AVFilter *filter_src_first = avfilter_get_by_name("abuffer");
    AVFilter *filter_sink = avfilter_get_by_name("abuffersink");
    AVFilterInOut *filter_output_first = avfilter_inout_alloc();
    AVFilterInOut *filter_input = avfilter_inout_alloc();
    _filter_graph = avfilter_graph_alloc();

    snprintf(args_spk, sizeof(args_spk),
             "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%llu",
             decodeCtx->time_base.num,
             decodeCtx->time_base.den,
             decodeCtx->sample_rate,
             av_get_sample_fmt_name(decodeCtx->sample_fmt),
             decodeCtx->channel_layout
    );
    int ret = 0;
    ret = avfilter_graph_create_filter(&_filter_ctx_src_first, filter_src_first, pad_name_first,
                                       args_spk, NULL, _filter_graph);
    if (ret < 0) {
        LOGE("Filter: failed to call avfilter_graph_create_filter _filter_ctx_src_first");
        return -1;
    }

    ret = avfilter_graph_create_filter(&_filter_ctx_sink, filter_sink, "out", NULL, NULL,
                                       _filter_graph);
    if (ret < 0) {
        LOGE("Filter: failed to call avfilter_graph_create_filter");
        return -1;
    }
    AVCodecContext *encodec_ctx = decodeCtx;
    ret = av_opt_set_bin(_filter_ctx_sink, "sample_fmts", (uint8_t *) &encodec_ctx->sample_fmt,
                         sizeof(encodec_ctx->sample_fmt), AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        LOGE("Filter: failed to call av_opt_set_bin sample_fmts");
        return -1;
    }
    ret = av_opt_set_bin(_filter_ctx_sink, "channel_layouts",
                         (uint8_t *) &encodec_ctx->channel_layout,
                         sizeof(encodec_ctx->channel_layout), AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        LOGE("Filter: failed to call av_opt_set_bin -- channel_layouts");
        return -1;
    }
    ret = av_opt_set_bin(_filter_ctx_sink, "sample_rates", (uint8_t *) &encodec_ctx->sample_rate,
                         sizeof(encodec_ctx->sample_rate), AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        LOGE("Filter: failed to call av_opt_set_bin -- sample_rates");
        return -1;
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
        return -1;
    }
    ret = avfilter_graph_config(_filter_graph, NULL);
    if (ret < 0) {
        LOGE("Filter: failed to call avfilter_graph_config");
        return -1;
    }
    avfilter_inout_free(&filter_input);
    av_free(filter_src_first);
    avfilter_inout_free(filter_outputs);
    return 0;
}

static int getFrameCount(const char *file) {
    AVFormatContext *ifmt_ctx = NULL;
    AVPacket pkt;
    int ret = -1;
    int frameCount = 0;
    if ((ret = avformat_open_input(&ifmt_ctx, file, 0, 0)) < 0) {
        LOGE("Could not open input file '%s'", file);
        return 0;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        LOGE("Failed to retrieve input stream information");
        return 0;
    }
    int audio_index = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0); 
    if (audio_index == -1) {
        LOGD("can't find video stream in %s\n", ifmt_ctx->filename); 
        return 0;
    }
    while (1) {
          ret = av_read_frame(ifmt_ctx, &pkt); 
        if (ret < 0) {
            break; 
        } 
        if (pkt.stream_index == audio_index) {
            frameCount++;
        } 
        av_packet_unref(&pkt);
    } 
    avio_closep(&ifmt_ctx->pb);
    return frameCount;
}

int audio_speed(const char *src_filename, const char *dst_filename, float rate,
                ProgressCallback *callback) {
    if (src_filename == NULL || dst_filename == NULL || callback == NULL) {
        LOGE("输入参数错误");
        return -1;
    }
    LOGI("audio_speed:src_filename:%s,dst_filename:%s,rate:%f", src_filename, dst_filename, rate);
    int ret = -1;
    char filter_desc[128];
    int speedLevel = (rate <= 0.25 ? -2 : rate <= 0.5 ? -1 : rate <= 1 ? 1 : 2);
    switch (speedLevel) {
        case -2:
            strcpy(filter_desc, "atempo=0.5,atempo=0.5");  //man 4 bei
            break;
        case -1:
            strcpy(filter_desc, "atempo=0.5");
            break;
        case 1:
            strcpy(filter_desc, "atempo=2.0");
            break;
        case 2:
            strcpy(filter_desc, "atempo=2.0,atempo=2.0");
            break;
    }
    av_register_all();

    int totalFrame = getFrameCount(src_filename);
    avfilter_register_all();
    open_input_file(src_filename, dst_filename);
    audio_encode_init(dst_filename, av_get_channel_layout_nb_channels(out_ch_layout), 57000,
                      out_sample_rate);
    init_filter(filter_desc);

    AVFrame *pFrame = av_frame_alloc();
    AVFrame *transcodeFrame = av_frame_alloc();
    AVPacket packet;
    av_init_packet(&packet);
    int got_sound;
    int frame_count = 0;
    while (true) {
        packet.data = NULL;
        packet.size = 0;
        if (av_read_frame(ifmt_ctx, &packet) < 0) {
            av_free_packet(&packet);
            LOGE("av_read_frame < 0");
            goto Fished;
        }
        frame_count++;
        if (packet.stream_index == _index_first) {
            if (avcodec_decode_audio4(decodeCtx, pFrame, &got_sound, &packet) < 0) {
                LOGE("avcodec_decode_audio4  failed");
                av_free_packet(&packet);
                break;
            }
            if (!got_sound) {
                LOGI("can't got sound");
                continue;
            }
            AVFrame *pFrame_first = pFrame;
            ret = av_buffersrc_add_frame(_filter_ctx_src_first, pFrame_first);
            if (ret < 0) {
                LOGE("Mixer: failed to call av_buffersrc_add_frame (speaker) %d", ret);
                continue;
            }

            while (1) {
                AVFrame *pFrame_out = av_frame_alloc();
                ret = av_buffersink_get_frame_flags(_filter_ctx_sink, pFrame_out, 0);
                if (ret < 0) {
                    LOGE("Mixer: failed to call av_buffersink_get_frame_flags %d", ret);
                    break;
                }
                if (pFrame_out->data[0] != NULL) {
                    if (pFrame_out) {
                        if (av_sample_fmt_is_planar((AVSampleFormat) decodeCtx->sample_fmt)) {
                            pFrame_out->pts = av_frame_get_best_effort_timestamp(pFrame_out);
                            if (0 !=
                                TransSample(pFrame_out, transcodeFrame, decodeCtx->sample_fmt)) {
                                LOGE("can not swr the audio data!\n");
                                goto Fished;
                            }
                        }
                        if (audio_encoding(transcodeFrame->data[0], transcodeFrame->linesize[0]) <
                            0) {
                            LOGE("audio_encoding < 0");
                            goto Fished;
                        }
                    }
                    av_frame_free(&pFrame_out);
                }
            }
            if (callback != NULL) {
                callback->onProgress(frame_count, totalFrame);
            }
        }
    }
    LOGD("progress Fished");
//    if (callback != NULL) {
//        callback->onFinish(totalFrame, totalFrame);
//    }
    Fished:
    av_packet_unref(&packet);
    if (swrInitFlag == 1) {
        deInitSwr();
        swrInitFlag = 0;
    }
    LOGI("FISHEDFISHED");
    av_frame_free(&pFrame);
    if (transcodeFrame != NULL) {
        av_frame_free(&transcodeFrame);
    }
    audio_encode_end();
    avfilter_free(_filter_ctx_src_first);
    avfilter_free(_filter_ctx_sink);
    avfilter_graph_free(&_filter_graph);
    avformat_close_input(&ifmt_ctx);
    return 0;
}

int audioRate(const char *srcFile1, const char *outFile, float rate, ProgressCallback *callback) {
    int ret = -1;
    ret = audio_speed(srcFile1, outFile, rate, callback);
    return ret;
}
int audioRateFast(const char *srcFile1, const char *outFile, float rate, ProgressCallback *callback){
    int ret = -1;
    if(rate<=0.5){
        rate=1;
    } else{
        rate=2;
    }
    LOGD("audioRateFast rate %f",rate);
    ret = audio_speed(srcFile1, outFile, rate, callback);
    return ret;
}
int
audioRate(const char *srcFile1, const char *outFile, float rate, HandleProgressCallback *callback) {
    OptAudioCallback *audioCallback = new OptAudioCallback(callback);
    int ret = -1;
    ret = audio_speed(srcFile1, outFile, rate, audioCallback);
    delete (audioCallback);
    return ret;
}

OptAudioCallback::OptAudioCallback(HandleProgressCallback *callback) {
    this->callback = callback;
}

void OptAudioCallback::onProgress(int progress, int total) {
    if (this->callback != NULL) {
        this->callback->onHandleProgress(progress * 100 / total, 100);
    }
}

void OptAudioCallback::onFinish(int progress, int total) {
    if (this->callback != NULL) {
        this->callback->onHandleProgress(progress * 100 / total, 100);
    }
}