#include "AudioRecord.h"
#include "audioTranscode.h"
#include "audioMix.h"
#include "audioVolumn.h"
#define AV_LOG av_log
static AVFormatContext *ifmt_first_ctx;
static AVFormatContext *ifmt_second_ctx;
static AVCodecContext *pDecodeCtx1;
static AVCodecContext *pDecodeCtx2;
static AVFormatContext *ofmt_ctx;
static int _index_first = -1;
static int _index_second = -1;
static int _index_out = -1;
static AVFilterGraph* _filter_graph = NULL;
static AVFilterContext* _filter_ctx_src_first = NULL;
static AVFilterContext* _filter_ctx_src_second = NULL;
static AVFilterContext* _filter_ctx_sink = NULL;

static int64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
static enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
static int out_sample_rate = 44100;
static int swrInitFlag = 0;

static int open_input_file1(const char *filename)
{
    LOGI("filename:%s",filename);
    AVCodec *codec;
    int ret;
    ifmt_first_ctx = NULL;
    if ((ret = avformat_open_input(&ifmt_first_ctx, filename, NULL, NULL)) < 0) {
        LOGE( "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(ifmt_first_ctx, NULL)) < 0) {
        LOGE("Cannot find stream information\n");
        return ret;
    }
    _index_first = av_find_best_stream(ifmt_first_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    if (ret < 0) {
        LOGD("Cannot find a video stream in the input file\n");
        return -1;
    }
    AVCodecParameters *pAVCodecParameters = (ifmt_first_ctx)->streams[_index_first]->codecpar;
    pDecodeCtx1 = avcodec_alloc_context3(codec);
    if ((ret = avcodec_parameters_to_context(pDecodeCtx1, pAVCodecParameters)) < 0) {
        LOGI("open_decodec_context()无法根据pCodec分配AVCodecContext");
        return ret;
    }
    //打开解码器
    if ( (ret = avcodec_open2(pDecodeCtx1, codec, NULL)) < 0) {
        LOGI("open_decodec_context()无法打开编码器");
        return ret;
    }
    av_dump_format(ifmt_first_ctx, 0, filename, 0);
    return 0;
}

static int open_input_file2(const char *filename)
{
    LOGI("filename:%s",filename);
    AVCodec *codec;
    int ret;
    unsigned int i;
    ifmt_second_ctx = NULL;
    if ((ret = avformat_open_input(&ifmt_second_ctx, filename, NULL, NULL)) < 0) {
        LOGE( "Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(ifmt_second_ctx, NULL)) < 0) {
        LOGE("Cannot find stream information\n");
        return ret;
    }
    _index_second = av_find_best_stream(ifmt_second_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    if (ret < 0) {
        LOGD("Cannot find a video stream in the input file\n");
        return -1;
    }
    AVCodecParameters *pAVCodecParameters = (ifmt_second_ctx)->streams[_index_second]->codecpar;
    pDecodeCtx2 = avcodec_alloc_context3(codec);
    if ((ret = avcodec_parameters_to_context(pDecodeCtx2, pAVCodecParameters)) < 0) {
        LOGI("open_decodec_context()无法根据pCodec分配AVCodecContext");
        return ret;
    }
    //打开解码器
    if ((ret = avcodec_open2(pDecodeCtx2, codec, NULL)) < 0) {
        LOGI("open_decodec_context()无法打开编码器");
        return ret;
    }
    av_dump_format(ifmt_second_ctx, 0, filename, 0);
    return 0;
}

static int init_filter(char* filter_desc)
{
	char args_spk[512];
	char* pad_name_first = "in0";
	char args_mic[512];
	char* pad_name_second = "in1";

	AVFilter* filter_src_first = avfilter_get_by_name("abuffer");
	AVFilter* filter_src_second = avfilter_get_by_name("abuffer");
	AVFilter* filter_sink = avfilter_get_by_name("abuffersink");
	AVFilterInOut* filter_output_first = avfilter_inout_alloc();
	AVFilterInOut* filter_output_second = avfilter_inout_alloc();
	AVFilterInOut* filter_input = avfilter_inout_alloc();
	_filter_graph = avfilter_graph_alloc();

	snprintf(args_spk, sizeof(args_spk), "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%llu",
		pDecodeCtx1->time_base.num,
             pDecodeCtx1->time_base.den,
             pDecodeCtx1->sample_rate,
		av_get_sample_fmt_name(pDecodeCtx1->sample_fmt),
             pDecodeCtx1->channel_layout);

	snprintf(args_mic, sizeof(args_mic), "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%llu",
		pDecodeCtx2->time_base.num,
             pDecodeCtx2->time_base.den,
             pDecodeCtx2->sample_rate,
		av_get_sample_fmt_name(pDecodeCtx2->sample_fmt),
             pDecodeCtx2->channel_layout);

	int ret = 0;
	ret = avfilter_graph_create_filter(&_filter_ctx_src_first, filter_src_first, pad_name_first, args_spk, NULL, _filter_graph);
	if (ret < 0) {
		LOGE( "Filter: failed to call avfilter_graph_create_filter");
		return -1;
	}
	ret = avfilter_graph_create_filter(&_filter_ctx_src_second, filter_src_second, pad_name_second, args_mic, NULL, _filter_graph);
	if (ret < 0) {
		LOGE( "Filter: failed to call avfilter_graph_create_filter");
		return -1;
	}

	ret = avfilter_graph_create_filter(&_filter_ctx_sink, filter_sink, "out", NULL, NULL, _filter_graph);
	if (ret < 0) {
		LOGE( "Filter: failed to call avfilter_graph_create_filter");
		return -1;
	}

	AVCodecContext* encodec_ctx = pDecodeCtx1;
	ret = av_opt_set_bin(_filter_ctx_sink, "sample_fmts", (uint8_t*)&encodec_ctx->sample_fmt, sizeof(encodec_ctx->sample_fmt), AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		LOGE( "Filter: failed to call av_opt_set_bin");
		return -1;
	}
	ret = av_opt_set_bin(_filter_ctx_sink, "channel_layouts", (uint8_t*)&encodec_ctx->channel_layout, sizeof(encodec_ctx->channel_layout), AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		LOGE( "Filter: failed to call av_opt_set_bin -- channel_layouts");
		return -1;
	}
	ret = av_opt_set_bin(_filter_ctx_sink, "sample_rates", (uint8_t*)&encodec_ctx->sample_rate, sizeof(encodec_ctx->sample_rate), AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		LOGE( "Filter: failed to call av_opt_set_bin -- sample_rates");
		return -1;
	}

	filter_output_first->name = av_strdup(pad_name_first);
	filter_output_first->filter_ctx = _filter_ctx_src_first;
	filter_output_first->pad_idx = 0;
	filter_output_first->next = filter_output_second;

	filter_output_second->name = av_strdup(pad_name_second);
	filter_output_second->filter_ctx = _filter_ctx_src_second;
	filter_output_second->pad_idx = 0;
	filter_output_second->next = NULL;

	filter_input->name = av_strdup("out");
	filter_input->filter_ctx = _filter_ctx_sink;
	filter_input->pad_idx = 0;
	filter_input->next = NULL;

	AVFilterInOut* filter_outputs[2];
	filter_outputs[0] = filter_output_first;
	filter_outputs[1] = filter_output_second;

	ret = avfilter_graph_parse_ptr(_filter_graph, filter_desc, &filter_input, filter_outputs, NULL);
	if (ret < 0) {
		LOGE( "Filter: failed to call avfilter_graph_parse_ptr");
		return -1;
	}

	ret = avfilter_graph_config(_filter_graph, NULL);
	if (ret < 0) {
		LOGE( "Filter: failed to call avfilter_graph_config");
		return -1;
	}

	avfilter_inout_free(&filter_input);
	av_free(filter_src_first);
	av_free(filter_src_second);
	avfilter_inout_free(filter_outputs);
	return 0;
}

int audio_mix_new(const char *input1_filename, const char *input2_filename, const char *output_filename,double vol1 = 1.0,double vol2 = 1.0)
{
    int volInit = 0;
	int ret = -1;
	char *filter_desc = "[in0][in1]amix=inputs=2[out]";
    AVPacket packet2;
    int got_sound;
    float volRatio = vol1 /vol2*1.0f;
    AVPacket packet1;
	av_register_all();
    avfilter_register_all();
    AVFrame* pFrame1 = av_frame_alloc();
    AVFrame* transcodeFrame = av_frame_alloc();
    AVFrame* pFrame2 = av_frame_alloc();
    AVFrame* pFrame_out = av_frame_alloc();
    AVFrame* volFrame;
	if ((ret = open_input_file1(input1_filename) )< 0) {
		goto FINISH;
	}
    if ((ret = open_input_file2(input2_filename) )< 0) {
        goto FINISH;
    }
    if(volRatio>0 && volRatio < 1){
        audio_vol_change_Init(pDecodeCtx1,volRatio);
        volFrame = av_frame_alloc();
        volInit = 1;
    }else if(volRatio > 1){
        volRatio = vol2/vol1 * 1.0f;
        audio_vol_change_Init(pDecodeCtx2,volRatio);
        volFrame = av_frame_alloc();
        volInit = 2;
    }
    if (swrInitFlag == 0) {
        initSwr(pDecodeCtx1->channel_layout, pDecodeCtx1->sample_fmt, pDecodeCtx1->sample_rate);
        swrInitFlag = swrInitFlag + 1;
    }
    audio_encode_init(output_filename, av_get_channel_layout_nb_channels(out_ch_layout), 57000,
                      out_sample_rate);
	init_filter(filter_desc);
	av_init_packet(&packet1);
	av_init_packet(&packet2);
	while (true) {
		while (true) {
			packet1.data = NULL;
			packet1.size = 0;
			if ((ret = av_read_frame(ifmt_first_ctx, &packet1)) < 0) {
                LOGI("av_read_frame < 0");
				av_free_packet(&packet1);
				goto FINISH;
			}

			if (packet1.stream_index == _index_first) {
				if ((ret = avcodec_decode_audio4(pDecodeCtx1, pFrame1, &got_sound, &packet1) )< 0) {
                    LOGE("avcodec_decode_audio4 failed");
                    continue;
				}

				av_free_packet(&packet1);
				if (got_sound) {
                    if(volInit == 1){
                        audio_vol_change2(pFrame1,volFrame);
                        LOGI("audio_vol_change2 success");
                        pFrame1 = volFrame;
                    }

					ret = av_buffersrc_add_frame(_filter_ctx_src_first, pFrame1);

					if (ret < 0) {
						LOGE( "av_buffersrc_add_frame1 error");
						continue;
					}
					break;
				}
			}else{
                LOGI("GGGGGGGGGGG %d,%d",packet1.stream_index,_index_first);
            }
		}

		while (true) {
			packet2.data = NULL;
			packet2.size = 0;
			if (av_read_frame(ifmt_second_ctx, &packet2) < 0) {
				av_free_packet(&packet2);
				goto FINISH;
			}
			if (packet2.stream_index == _index_second) {
				if (avcodec_decode_audio4(pDecodeCtx2, pFrame2, &got_sound, &packet2) < 0) {
                    LOGE("avcodec_decode_audio4 failed");
                    continue;
				}
				av_free_packet(&packet2);
				if (got_sound) {
                    if(volInit == 2){
                        audio_vol_change2(pFrame2,volFrame);
                        LOGI("audio_vol_change2 success");
                        pFrame2 = volFrame;
                    }
					ret = av_buffersrc_add_frame(_filter_ctx_src_second, pFrame2);
					if (ret < 0) {
						LOGE( "av_buffersrc_add_frame2 error");
						continue;
					}
					break;
				}
			}
		}

		while (true) {
			ret = av_buffersink_get_frame_flags(_filter_ctx_sink, pFrame_out, 0);
			if (ret < 0) {
				LOGE( "av_buffersink_get_frame_flags error");
				break;
			}
			if (pFrame_out->data[0] != NULL) {
                if (av_sample_fmt_is_planar((AVSampleFormat) pDecodeCtx1->sample_fmt)) {
                    pFrame_out->pts = av_frame_get_best_effort_timestamp(pFrame_out);
                    if (0 !=
                        TransSample(pFrame_out, transcodeFrame,pDecodeCtx1->sample_fmt)) {
                        LOGE("can not swr the audio data!\n");
                        goto FINISH;
                    }
                }
                if (audio_encoding(transcodeFrame->data[0], transcodeFrame->linesize[0]) <
                    0) {
                    LOGE("audio_encoding < 0");
                    break;
                }
                transcodeFrame->data[0] = NULL;
            }
		}
	}

FINISH:
    if(pFrame_out != NULL){
        av_frame_free(&pFrame_out);
    }
    if(volInit >= 1){
        audio_vol_change_DeInit();
        av_frame_free(&volFrame);
    }
    if (swrInitFlag == 1) {
        deInitSwr();
        swrInitFlag = 0;
    }
    if(pFrame1 != NULL && volInit != 1){
        av_frame_free(&pFrame1);
    }
	if(pFrame2 != NULL && volInit != 2){
        av_frame_free(&pFrame2);
    }
    if(pDecodeCtx1 != NULL){
        avcodec_free_context(&pDecodeCtx1);
    }
    if(pDecodeCtx2 != NULL){
        avcodec_free_context(&pDecodeCtx2);
    }
    if(_filter_ctx_src_first != NULL){
        avfilter_free(_filter_ctx_src_first);
    }
	if(_filter_ctx_src_second != NULL){
        avfilter_free(_filter_ctx_src_second);
    }
    if(_filter_ctx_sink != NULL){
        avfilter_free(_filter_ctx_sink);
    }
    if(_filter_graph != NULL){
        avfilter_graph_free(&_filter_graph);
    }
    if(ifmt_first_ctx != NULL){
        avformat_close_input(&ifmt_first_ctx);
    }
	if(ifmt_second_ctx != NULL){
        avformat_close_input(&ifmt_second_ctx);
    }
    audio_encode_end();
	return ret;
}
