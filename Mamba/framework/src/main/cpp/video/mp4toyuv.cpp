
#include "reverse.h"
#include <stack>
#include "Log.h"
using namespace std;

AVFormatContext *ifmt_ctx;
static AVFormatContext *ofmt_ctx_a;

#define Stack_data char*
static stack<Stack_data> G_stack;
static int frame_count;
static FILE *yuvfp; 
#define MAX_SIZE 10485760

static int open_input_file(const char *filename, int *video_index, int *audio_index)
{
    int ret;
    unsigned int i;
    ifmt_ctx = NULL;

    if ((ret = avformat_open_input(&ifmt_ctx, filename, NULL, NULL)) < 0) {
        LOGE("Cannot open input file\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
        LOGE("Cannot find stream information\n");
        return ret;
    }
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *stream;
        AVCodecContext *codec_ctx;
        stream = ifmt_ctx->streams[i];
        codec_ctx = stream->codec;
        /* Reencode video & audio and remux subtitles etc. */
        if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO
                || codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
            /* Open decoder */
            ret = avcodec_open2(codec_ctx,
                    avcodec_find_decoder(codec_ctx->codec_id), NULL);
            if (ret < 0) {
                LOGE("Failed to open decoder for stream #%u\n", i);
                return ret;
            }
        }
		if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) 
			*video_index = i;
		else if (codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) 
			*audio_index = i;
    }
    av_dump_format(ifmt_ctx, 0, filename, 0);

    return 0;
}


static int open_output_file(const char *filename)
{
	int i;
	AVStream *out_stream;

	avformat_alloc_output_context2(&ofmt_ctx_a, NULL, NULL, filename);
	if (!ofmt_ctx_a) {
		LOGE("Could not create output context \n");
		return AVERROR_UNKNOWN;
	}
    
	for (i = 0; i < ifmt_ctx->nb_streams; i++) {
		AVStream *in_stream = ifmt_ctx->streams[i];
		if (in_stream->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			continue;
		}
		else if (in_stream->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
			out_stream = avformat_new_stream(ofmt_ctx_a, NULL);
			if (!out_stream) {
				LOGE("Failed allocating output stream\n");
				return AVERROR_UNKNOWN;
			}
			
			if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
				LOGE("Failed to copy context from input to output stream codec context\n");
				return -1;
			}
			out_stream->codec->codec_tag = 0;

			if (ofmt_ctx_a->oformat->flags & AVFMT_GLOBALHEADER)
				out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;   
        }
	}

	if (!(ofmt_ctx_a->flags & AVFMT_NOFILE)) {
		if (avio_open(&ofmt_ctx_a->pb, filename, AVIO_FLAG_WRITE) < 0) {
			LOGE("Could not open output file '%s'\n", filename);
			return -1;
		}
	}
	if (avformat_write_header(ofmt_ctx_a, NULL) < 0) {
		LOGE("Error occurred when opening audio output file");
		return -1;
	}
	return 0;
}


int decode_packet(AVPacket *packet, int *got_frame, const int video_index, const int audio_index, AVFormatContext *ofmt_ctx_a)
{
	*got_frame = 0;
	AVFrame *frame = NULL;
	int ret = 0;
	
	frame = av_frame_alloc();
	if (!frame) {
		LOGE("Alloc frame fail!\n");
		return -1;
	}

	if (packet->stream_index == video_index) {
		ret = avcodec_decode_video2(ifmt_ctx->streams[(AVMediaType)video_index]->codec , frame, got_frame, packet);
		if ( ret < 0 ) {
			LOGE("Error: decodec video frame failed\n");
			return ret;
		}
		
		if ( *got_frame ) {
			frame_count ++;

			Stack_data tmp_stk;
			tmp_stk = new char[frame->width * frame->height * 3 / 2];
			memset(tmp_stk, 0, frame->width * frame->height * 3 / 2);
			int n_step = 0;

			for(int i = 0; i < frame->height; i++) {  				
				memcpy(tmp_stk + n_step, frame->data[0] + i * frame->linesize[0], frame->width);
				n_step += frame->width;
			}  
			for(int i = 0; i < frame->height/2; i++) { 			
				memcpy(tmp_stk + n_step, frame->data[1] + i * frame->linesize[1], frame->width/2);
				n_step += frame->width/2;
			}  
			for(int i = 0; i < frame->height/2; i++) {  
				memcpy(tmp_stk + n_step, frame->data[2] + i * frame->linesize[2], frame->width/2);
				n_step += frame->width/2;
			}

			G_stack.push(tmp_stk);
		}
	} else {
		AVStream *in_stream = ifmt_ctx->streams[(AVMediaType)audio_index];
		AVStream *out_stream = ofmt_ctx_a->streams[0];
		packet->pts = av_rescale_q_rnd(packet->pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		packet->dts = av_rescale_q_rnd(packet->dts, in_stream ->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		packet->duration = av_rescale_q(packet->duration, in_stream->time_base, out_stream->time_base);
		packet->pos = -1;
		packet->stream_index = 0;  // ??????????????????

		if ((ret = av_interleaved_write_frame(ofmt_ctx_a, packet)) < 0) {
			LOGE("av_interleaved_write_frame failed\n");
			return -1;
		}

		//av_free_packet(packet);
	}
	av_frame_free(&frame);
	return  packet->size;
}


static int write_yuvdatato_file(const char *filename, int width, int height)
{
	if (G_stack.empty()) {
        LOGE("G_stack.empty \n");
		return 0;
	}
	if ((yuvfp = fopen(filename, "wb+")) == NULL) {  
		LOGE("Open yuv file error\n");
        return -1;  
    }
	while (!G_stack.empty()) {
		char *buf = G_stack.top();
		fwrite(buf, 1, width * height * 3 / 2, yuvfp);
		delete buf;
		buf = NULL;

		G_stack.pop();
	}
    fclose(yuvfp);
	return 0;
}


int mp4_reverse_yuv(const char *src_filename, const char *yuv_dst_filename, const char *aac_dst_filename, int *tmp_frame_count, int *tmp_per_count, int *width, int *height)
{
	int video_index = -1;
	int audio_index = -1;
	int ret = -1;
	AVPacket packet;
	int got_frame;
	char tmp_file_name[100];

	av_register_all();
	if (open_input_file(src_filename, &video_index, &audio_index) < 0) {
		LOGE("Open input file error\n");
		return -1;
	}
	if (open_output_file(aac_dst_filename) < 0) {
		LOGE("Open aac file error\n");
		return -1;
	}

	*width =  ifmt_ctx->streams[video_index]->codec->width;
	*height =  ifmt_ctx->streams[video_index]->codec->height;
	int per_count = MAX_SIZE / ((*width) * (*height) * 3 / 2) - 1;
	*tmp_per_count = per_count;

	while (1) {
		if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0){
            LOGE("av_read_frame failed:%d\n",ret);
            break;
        }
		do {
			ret = decode_packet(&packet, &got_frame, video_index, audio_index, ofmt_ctx_a);
			if (ret < 0) {
                LOGE("decode_packet < 0 :%d \n",ret);
				break;
			}
			
			if (frame_count != 0 && frame_count%per_count == 0) {
				sprintf(tmp_file_name, "%s%d", yuv_dst_filename, frame_count/per_count);
                LOGI("tmp_file_name:%s",tmp_file_name);
				write_yuvdatato_file(tmp_file_name, *width, *height);
                LOGI("write_yuvdatato_file");
			}
			packet.data += ret;
			packet.size -= ret;	
		}while (packet.size > 0);
		av_free_packet(&packet);
	}
	packet.data = NULL;
	packet.size = 0;
	do {
		ret = decode_packet(&packet, &got_frame, video_index, audio_index, ofmt_ctx_a);
		if (ret < 0) {
			break;
		}
		packet.data += ret;
		packet.size -= ret;
	} while (got_frame);
	if (frame_count%per_count != 0) {
		sprintf(tmp_file_name, "%s%d", yuv_dst_filename, frame_count/per_count + 1);
		write_yuvdatato_file(tmp_file_name, *width, *height);
	}
	*tmp_frame_count = frame_count;
end:
	av_write_trailer(ofmt_ctx_a);

	avcodec_close(ifmt_ctx->streams[video_index]->codec);
	//avformat_close_input(&ifmt_ctx);
	//fclose(yuvfp);
	return 0;
}
