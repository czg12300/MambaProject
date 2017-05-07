//
// Created by jakechen on 2017/2/5.
//

#include "ChangeFrameRate.h"

int changeFrameRate(const char *inFile, const char *outFile, double frameRate) {
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;
    int ret = -1, i, video_index, audio_index, audio, video;
    AVStream *in_stream, *out_stream;
    int64_t *dts_start_from;
    int64_t *pts_start_from;
    av_register_all();
    if ((ret = avformat_open_input(&ifmt_ctx, inFile, 0, 0)) < 0) {
        LOGE("Could not open input file '%s'", inFile);
        goto end;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        LOGE("Failed to retrieve input stream information");
        goto end;
    }
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, outFile);
    if (!ofmt_ctx) {
        LOGE("Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    video_index = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0); 
    if (video_index == -1) {
        LOGD("can't find video stream in %s\n", ifmt_ctx->filename); 
        ret = -1; 
        goto end; 
    }
    audio_index = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0); 
    if (audio_index == -1) {
        LOGD("can't find video stream in %s\n", ifmt_ctx->filename); 
        ret = -1; 
        goto end; 
    }
    LOGD("ifmt_ctx->nb_streams=%d",ifmt_ctx->nb_streams);
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        in_stream = ifmt_ctx->streams[i];
        AVCodec *in_codec = avcodec_find_decoder(in_stream->codecpar->codec_id);
        if (in_codec == NULL) {
            LOGD("input codec is null");
            ret = -1;
            goto end;
        }
        AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_codec);
        if (!out_stream) {
            LOGE("Failed allocating output stream\n");
            ret = -1;
            goto end;
        }
        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if (ret < 0) {
            LOGE("Failed to copy context from input to output stream codec context\n");
            ret = -1;;
            goto end;
        }
        out_stream->codecpar->codec_tag = 0;
    }
    ret = avio_open(&ofmt_ctx->pb, outFile, AVIO_FLAG_WRITE);
    if (ret < 0) {
        LOGE("Could not open output file '%s'", outFile);
        goto end;
    }

    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        LOGE("Error occurred when opening output file\n");
        ret = -1;
        goto end;
    }
     if (ret < 0) {
        LOGE("Error seek\n"); 
        ret = -1; 
        goto end; 
    } 
    dts_start_from = (int64_t *) malloc(sizeof(int64_t) * ifmt_ctx->nb_streams);
    memset(dts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);
    pts_start_from = (int64_t *) malloc(sizeof(int64_t) * ifmt_ctx->nb_streams);
    memset(pts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);

    while (1) {
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
            break;
        in_stream = ifmt_ctx->streams[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];
        if (dts_start_from[pkt.stream_index] == 0) {
            dts_start_from[pkt.stream_index] = pkt.dts;
        }
        if (pts_start_from[pkt.stream_index] == 0) {
            pts_start_from[pkt.stream_index] = pkt.pts;
        }
//        if ( pkt.flags != AV_PKT_FLAG_KEY){
//            if (pkt.stream_index == video_index) {
//
//                video++;
//                if (frameRate>1&&video%3==0){
//                    continue;
//                }
//
//            }else if (pkt.stream_index == audio_index) {
//                audio++;
//                if (frameRate>1&&audio%3==0&&pkt.flags){
//                    continue;
//                }
//            }
//        }

        if (pkt.stream_index == video_index) {

            LOGD(" avg_frame_rate.den：%d", in_stream->avg_frame_rate.den);
            LOGD("in_stream->duration / AV_TIME_BASE：%d", in_stream->duration / AV_TIME_BASE);
            LOGD(" avg_frame_rate.num：%d", in_stream->avg_frame_rate.num);
            LOGD(" 帧率：%d",in_stream->avg_frame_rate.num/ in_stream->avg_frame_rate.den);
            LOGD(" time_basduration / AV_TIME_BASEe.den：%d", in_stream->time_base.den);
            LOGD(" time_base.num：%d", in_stream->time_base.num);
            LOGD(" 时间基：%d",in_stream->time_base.num/ in_stream->time_base.den);
//            out_stream->avg_frame_rate.den = 30;
            int duration=(in_stream->duration / AV_TIME_BASE)*in_stream->time_base.den;
            int s=in_stream->time_base.den * frameRate;
            if(s>duration){
                s=duration;
            }
            out_stream->time_base.den =s;
        } else{
            continue;
        }

        /* copy packet */
        int64_t tempPts = pkt.pts - pts_start_from[pkt.stream_index];
        int64_t tempDts = pkt.dts - dts_start_from[pkt.stream_index];
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
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

        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0) {
            LOGE("Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);
    }
    free(dts_start_from);
    free(pts_start_from);

    av_write_trailer(ofmt_ctx);
    ret = 1;
    end:
    avformat_close_input(&ifmt_ctx);
    avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    return ret;
}