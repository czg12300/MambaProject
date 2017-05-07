
#include "reverse.h"
#include "Log.h"
extern AVFormatContext *ifmt_ctx;
static AVFormatContext *ofmt_ctx;

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
        LOGE( "Could not create output context\n");
        return AVERROR_UNKNOWN;
    }
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            LOGE( "Failed allocating output stream\n");
            return AVERROR_UNKNOWN;
        }
        in_stream = ifmt_ctx->streams[i];
        dec_ctx = in_stream->codec;
        enc_ctx = out_stream->codec;

        if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
                || dec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            /* in this example, we choose transcoding to same codec */
            encoder = avcodec_find_encoder(dec_ctx->codec_id);
            /* In this example, we transcode to same properties (picture size,
             * sample rate etc.). These properties can be changed for output
             * streams easily using filters */

			if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
				enc_ctx->flags |= CODEC_FLAG_GLOBAL_HEADER;

            if (dec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
                enc_ctx->height = dec_ctx->height;
                enc_ctx->width = dec_ctx->width;
                enc_ctx->sample_aspect_ratio = dec_ctx->sample_aspect_ratio;
                /* take first format from list of supported formats */
                enc_ctx->pix_fmt = encoder->pix_fmts[0];
                /* video time_base can be set to whatever is handy and supported by encoder */
                enc_ctx->time_base = dec_ctx->time_base;
				
				//enc_ctx->me_range = 16; 
                //enc_ctx->max_qdiff = 4;
				enc_ctx->qmin = 3;//dec_ctx->qmin;
				enc_ctx->qmax = 30;//dec_ctx->qmax;
				enc_ctx->qcompress = 1;//dec_ctx->qcompress;
				enc_ctx->width = dec_ctx->width;
				enc_ctx->height = dec_ctx->height;
            } else {
                enc_ctx->sample_rate = dec_ctx->sample_rate;
                enc_ctx->channel_layout = dec_ctx->channel_layout;
                enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);
                /* take first format from list of supported formats */
                enc_ctx->sample_fmt = encoder->sample_fmts[0];
				AVRational time_base={1, enc_ctx->sample_rate};
                enc_ctx->time_base = time_base;
            }
            /* Third parameter can be used to pass settings to encoder */
            ret = avcodec_open2(enc_ctx, encoder, NULL);
            if (ret < 0) {
                LOGE( "Cannot open video encoder for stream #%u\n", i);
                return ret;
            }
        } else if (dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN) {
            av_log(NULL, AV_LOG_FATAL, "Elementary stream #%d is of unknown type, cannot proceed\n", i);
            return AVERROR_INVALIDDATA;
        } else {
            /* if this stream must be remuxed */
            ret = avcodec_copy_context(ofmt_ctx->streams[i]->codec,
                    ifmt_ctx->streams[i]->codec);
            if (ret < 0) {
                LOGE( "Copying stream context failed\n");
                return ret;
            }
        } 
    }

    av_dump_format(ofmt_ctx, 0, filename, 1);
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            LOGE( "Could not open output file '%s'", filename);
            return ret;
        }
    }
    /* init muxer, write output file header */
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        LOGE( "Error occurred when opening output file\n");
        return ret;
    }
    return 0;
}

static int encode_write_frame(AVFrame *tmp_frame, unsigned int stream_index, int *got_frame) {
    int ret;
    int got_frame_local;
    AVPacket enc_pkt;
    int (*enc_func)(AVCodecContext *, AVPacket *, const AVFrame *, int *) =
        (ifmt_ctx->streams[stream_index]->codec->codec_type ==
         AVMEDIA_TYPE_VIDEO) ? avcodec_encode_video2 : avcodec_encode_audio2;
    if (!got_frame)
        got_frame = &got_frame_local;
    av_log(NULL, AV_LOG_INFO, "Encoding frame\n");
    
    enc_pkt.data = NULL;
    enc_pkt.size = 0;
    av_init_packet(&enc_pkt);
    ret = enc_func(ofmt_ctx->streams[stream_index]->codec, &enc_pkt,
            tmp_frame, got_frame);
    av_frame_free(&tmp_frame);
    if (ret < 0)
        return ret;
    if (!(*got_frame))
        return 0;
    /* prepare packet for muxing */
    enc_pkt.stream_index = stream_index;
    enc_pkt.dts = av_rescale_q_rnd(enc_pkt.dts,
            ofmt_ctx->streams[stream_index]->codec->time_base,
            ofmt_ctx->streams[stream_index]->time_base,
            (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
    enc_pkt.pts = av_rescale_q_rnd(enc_pkt.pts,
            ofmt_ctx->streams[stream_index]->codec->time_base,
            ofmt_ctx->streams[stream_index]->time_base,
            (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
    enc_pkt.duration = av_rescale_q(enc_pkt.duration,
            ofmt_ctx->streams[stream_index]->codec->time_base,
            ofmt_ctx->streams[stream_index]->time_base);
    av_log(NULL, AV_LOG_DEBUG, "Muxing frame\n");
    /* mux encoded frame */
    ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
    return ret;
}


int yuv_to_mp4(const char *vedio_src_filename, const char *audio_src_filename, const char *dst_filename, const int *frame_count, const int *tmp_per_count, const int *width, const int *height)
{
	LOGI("yuv_to_mp4:vedio_src_filename:%s,audio_src_filename:%s,dst_filename:%s,frame_count:%d,tmp_per_count:%d,width:%d,height:%d",vedio_src_filename,audio_src_filename,dst_filename,*frame_count,*tmp_per_count,*width,*height);
	FILE *yuv_file ;
	AVFrame *pFrame;
	AVPacket enc_pkt;

	av_register_all();
	if (open_output_file(dst_filename) < 0) {
		LOGE( "Error occurred when opening output file\n");
		return -1;
	}

	int stream_index = AVMEDIA_TYPE_VIDEO;
	AVCodecContext *pCodecCtx = ofmt_ctx->streams[stream_index]->codec;
	pFrame = av_frame_alloc();
	int picture_size = avpicture_get_size(pCodecCtx->pix_fmt, *width, *height);
	uint8_t *picture_buf = (uint8_t *)av_malloc(picture_size);
	avpicture_fill((AVPicture *)pFrame, picture_buf, pCodecCtx->pix_fmt, *width, *height);
	av_new_packet(&enc_pkt, picture_size);
	int y_size = (*width) * (*height);
	int got_picture = 0;
	int remain_frame = (*frame_count)%(*tmp_per_count);
	int total_file = (*frame_count)/(*tmp_per_count);
	char tmp_name[100];
	
	if (remain_frame == 0) {
		remain_frame = *tmp_per_count;
	} else {
		total_file = total_file + 1;
	}
	int flag_first = 0;

	for (int i = 0; i < *frame_count; i++) {
		if (flag_first == 0) {
			sprintf(tmp_name, "%s%d", vedio_src_filename, total_file);
            LOGI("1 tmp_name:%s",tmp_name);
			if ((yuv_file = fopen(tmp_name, "rb+")) == NULL) {
				LOGE( "Open yuv file error\n");
				return -1;  
			}
			flag_first = 1;
			total_file = total_file - 1;
		}
		else if ( remain_frame == 0) {
			remain_frame = *tmp_per_count;
			sprintf(tmp_name, "%s%d", vedio_src_filename, total_file);
            LOGI("2 tmp_name:%s",tmp_name);
			fclose(yuv_file);
			if ((yuv_file = fopen(tmp_name, "rb+")) == NULL) {
				LOGE( "Open yuv file error\n");
				return -1;
			}
			total_file = total_file - 1;
		}
        remain_frame = remain_frame - 1;

		if (fread(picture_buf, 1, y_size * 3 / 2, yuv_file) <= 0) {
			LOGE( "Failed to read raw data\n");
			return 0;
		}else{
            LOGI("read raw data success\n");
        }

		pFrame->data[0] = picture_buf;                   // Y
		pFrame->data[1] = picture_buf + y_size;          // U 
		pFrame->data[2] = picture_buf + y_size * 5 / 4;  // V
		//int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(ifmt_ctx->streams[stream_index]->r_frame_rate);
		//pFrame->pts = (double)(i * calc_duration) / (double)(av_q2d(ifmt_ctx->streams[stream_index]->time_base) * AV_TIME_BASE);

		pFrame->pts = 2*i;
		pFrame->width = *width;
		pFrame->height = *height;
		pFrame->format = AV_PIX_FMT_YUV420P;

		int ret = avcodec_encode_video2(pCodecCtx, &enc_pkt, pFrame, &got_picture);
		if (ret < 0){
			LOGE( "Failed to encode!\n");
			return -1;
		}
        LOGI("encode success");
		if (!(got_picture)){
           continue;
		}
		
		enc_pkt.stream_index = stream_index;
		enc_pkt.dts = av_rescale_q_rnd(enc_pkt.dts,
				ofmt_ctx->streams[stream_index]->codec->time_base,
				ofmt_ctx->streams[stream_index]->time_base,
				(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		enc_pkt.pts = av_rescale_q_rnd(enc_pkt.pts,
				ofmt_ctx->streams[stream_index]->codec->time_base,
				ofmt_ctx->streams[stream_index]->time_base,
				(AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		enc_pkt.duration = av_rescale_q(enc_pkt.duration,
				ofmt_ctx->streams[stream_index]->codec->time_base,
				ofmt_ctx->streams[stream_index]->time_base);

		ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
		if (ret < 0) {
			break;
		}
        LOGI("write frame success %d",ret);
	}

	av_free(picture_buf); //
	av_frame_free(&pFrame);
	av_free_packet(&enc_pkt);
	av_write_trailer(ofmt_ctx);
	fclose(yuv_file);

	avformat_close_input(&ifmt_ctx);
	if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);

	return 0;
}