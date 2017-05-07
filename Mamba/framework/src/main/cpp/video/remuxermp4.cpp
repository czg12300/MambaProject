#include "reverse.h"

static AVFormatContext *ifmt_ctx_v;
static AVFormatContext *ifmt_ctx_a;
static AVFormatContext *ofmt_ctx;
static  int video_index_v = -1, video_index_out = -1;
static	int audio_index_a = -1, audio_index_out = -1;
static AVFormatContext *ifmt_ctx;

static int open_video_input_file(const char *filename)
{
    int ret;
    unsigned int i;
    ifmt_ctx = NULL;
    if ((ret = avformat_open_input(&ifmt_ctx_v, filename, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open vedio input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx_v, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }
    for (i = 0; i < ifmt_ctx_v->nb_streams; i++) {
        AVStream *stream;
        AVCodecContext *codec_ctx;
        stream = ifmt_ctx_v->streams[i];
        codec_ctx = stream->codec;
        /* Reencode video & audio and remux subtitles etc. */
        if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
            /* Open decoder */
            ret = avcodec_open2(codec_ctx,
                    avcodec_find_decoder(codec_ctx->codec_id), NULL);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
                return ret;
            }
        }
    }
    av_dump_format(ifmt_ctx_v, 0, filename, 0);

    return 0;
}


static int open_audio_input_file(const char *filename)
{
    int ret;
    unsigned int i;
    ifmt_ctx = NULL;
    if ((ret = avformat_open_input(&ifmt_ctx_a, filename, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open audio input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx_a, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }
    for (i = 0; i < ifmt_ctx_a->nb_streams; i++) {
        AVStream *stream;
        AVCodecContext *codec_ctx;
        stream = ifmt_ctx_a->streams[i];
        codec_ctx = stream->codec;
        /* Reencode video & audio and remux subtitles etc. */
        if (codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            /* Open decoder */
            ret = avcodec_open2(codec_ctx,
                    avcodec_find_decoder(codec_ctx->codec_id), NULL);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Failed to open decoder for stream #%u\n", i);
                return ret;
            }
        }
    }
    av_dump_format(ifmt_ctx_a, 0, filename, 0);

    return 0;
}


static int open_output_file(const char *filename)
{
    AVStream *out_stream;
    AVStream *in_stream;
    AVCodecContext *dec_ctx, *enc_ctx;
    AVCodec *encoder;
    int ret;
    unsigned int i;
    ofmt_ctx = NULL;

    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
    if (!ofmt_ctx) {
        av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
        return -1;
    }

	for (i = 0; i < ifmt_ctx_v->nb_streams; i++) {
		if (ifmt_ctx_v->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			AVStream *in_stream = ifmt_ctx_v->streams[i];
			AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
		    
			if (!out_stream) {
			    av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream");
				return -1;
			}
			video_index_v = i;
			video_index_out = out_stream->index;

			if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
				av_log(NULL, AV_LOG_ERROR, "Failed to copy context from input to output stream codec context");
				return -1;
			}
			out_stream->codec->codec_tag = 0;
			if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
				out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
			break;
		}
	}

	for (i = 0; i < ifmt_ctx_a->nb_streams; i++) {
		if(ifmt_ctx_a->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
			AVStream *in_stream = ifmt_ctx_a->streams[i];
			AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
			
			if (!out_stream) {
				av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream");
				return -1;
			}
			
			audio_index_a = i;
			audio_index_out = out_stream->index;

			if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
				av_log(NULL, AV_LOG_ERROR, "Failed to copy context from input to output stream codec context");
				return -1;
			}
			out_stream->codec->codec_tag = 0;
			if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
				out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
			break;
		}
	}

    av_dump_format(ofmt_ctx, 0, filename, 1);
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "Could not open output file '%s'", filename);
            return ret;
        }
    }
    /* init muxer, write output file header */
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error occurred when opening output file\n");
        return ret;
    }
    return 0;
}


int muxing_mp4(const char *mp4_src_filename, const char *aac_src_filename, const char *dst_filename)
{
	int ret = -1;

	av_register_all();
	ret = open_video_input_file(mp4_src_filename);
	if (ret < 0) {
		return -1;
	}

	ret = open_audio_input_file(aac_src_filename);
	if (ret < 0) {
		return -1;
	}

	ret = open_output_file(dst_filename);
	if (ret < 0) {
		return -1;
	}

	AVBitStreamFilterContext* aacbsfc =  av_bitstream_filter_init("aac_adtstoasc"); 
	int64_t cur_pts_v = 0, cur_pts_a = 0;
	AVFormatContext *ifmt_ctx;
	int stream_index = 0;
	AVPacket pkt;
	AVStream *in_stream, *out_stream;
	int frame_index = 0;

	while (1) {	
		if (av_compare_ts(cur_pts_v, ifmt_ctx_v->streams[video_index_v]->time_base, 
			cur_pts_a, ifmt_ctx_a->streams[audio_index_a]->time_base) <= 0) {
			    ifmt_ctx = ifmt_ctx_v;
				stream_index = video_index_out;

				if (av_read_frame(ifmt_ctx, &pkt) >= 0) {
					do {
						in_stream = ifmt_ctx->streams[pkt.stream_index];
						out_stream = ofmt_ctx->streams[stream_index];

						if (pkt.stream_index == video_index_v) {
							if (pkt.pts == AV_NOPTS_VALUE) {
								AVRational time_base1 = in_stream->time_base;
								//Duration between 2 frames (us)
								int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
								//Parameters
								pkt.pts = (double)(frame_index * calc_duration) / (double)(av_q2d(time_base1) * AV_TIME_BASE);
								pkt.dts = pkt.pts;
								pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1) * AV_TIME_BASE);
								frame_index++;
							}
							cur_pts_v = pkt.pts;
							break;
						} 
					}while(av_read_frame(ifmt_ctx, &pkt) >= 0);
				} else {
					break;
				}
		} else {
			ifmt_ctx = ifmt_ctx_a;
			stream_index = audio_index_out;
			if (av_read_frame(ifmt_ctx, &pkt) >= 0) {
				do{
					in_stream  = ifmt_ctx->streams[pkt.stream_index];
					out_stream = ofmt_ctx->streams[stream_index];

					if(pkt.stream_index==audio_index_a) {
						if(pkt.pts == AV_NOPTS_VALUE) {
							//Write PTS
							AVRational time_base1 = in_stream->time_base;
							//Duration between 2 frames (us)
							int64_t calc_duration=(double)AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
							//Parameters
							pkt.pts = (double)(frame_index * calc_duration) / (double)(av_q2d(time_base1) * AV_TIME_BASE);
							pkt.dts = pkt.pts;
							pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1) * AV_TIME_BASE);
							frame_index++;
						}
						cur_pts_a = pkt.pts;
						break;
					}
				} while(av_read_frame(ifmt_ctx, &pkt) >= 0);
			} else {
				break;
			}
		}

		av_bitstream_filter_filter(aacbsfc, out_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		pkt.stream_index = stream_index;

		printf("Write 1 Packet. size:%5d\tpts:%lld\n", pkt.size, pkt.pts);
		
		if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
			printf( "Error muxing packet\n");
			break;
		}
		av_free_packet(&pkt);
	}
	
	av_write_trailer(ofmt_ctx);
	av_bitstream_filter_close(aacbsfc);
	
end:
	avformat_close_input(&ifmt_ctx_v);
	avformat_close_input(&ifmt_ctx_a);

	/* close output */
	if (ofmt_ctx && !(ofmt_ctx->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);

	return 0;
}


