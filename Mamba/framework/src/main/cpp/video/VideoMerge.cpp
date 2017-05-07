#include <unistd.h>
#include "VideoMerge.h"
//
// Created by walljiang on 2017/03/06.
//

AVFormatContext *in1_fmtctx = NULL, *in2_fmtctx = NULL, *out_fmtctx = NULL;
AVStream *out_video_stream = NULL, *out_audio_stream = NULL;
int video_stream_index = -1, audio_stream_index = -1;


int open_input(const char *in1_name, const char *in2_name) {
    int ret = -1;
    if ((access(in1_name, F_OK)) == -1 || (access(in2_name, F_OK)) == -1) {
        return ret;
    }
    LOGI("open_input in1_name:%s,in2_name:%s", in1_name, in2_name);
    if ((ret = avformat_open_input(&in1_fmtctx, in1_name, NULL, NULL)) < 0) {
        LOGI("can not open the first input context!\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(in1_fmtctx, NULL)) < 0) {
        LOGI("can not find the first input stream info!\n");
        return ret;
    }

    if ((ret = avformat_open_input(&in2_fmtctx, in2_name, NULL, NULL)) < 0) {
        LOGI("can not open the second input context!\n");
        return ret;
    }
    if ((ret = avformat_find_stream_info(in2_fmtctx, NULL)) < 0) {
        LOGI("can not find the second input stream info!\n");
        return ret;
    }
    return ret;
}

int open_output(AVFormatContext *fmtctx, const char *out_name) {
    int ret = -1;
    if ((ret = avformat_alloc_output_context2(&out_fmtctx, NULL, NULL, out_name)) < 0) {
        LOGI("can not alloc context for output!,out_name: %s\n", out_name);
        return ret;
    }

    //new stream for out put
    for (int i = 0; i < fmtctx->nb_streams; i++) {
        if (fmtctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            out_video_stream = avformat_new_stream(out_fmtctx, NULL);
            if (!out_video_stream) {
                LOGI("Failed allocating output1 video stream\n");
                ret = AVERROR_UNKNOWN;
                return ret;
            }
            if ((ret = avcodec_copy_context(out_video_stream->codec, fmtctx->streams[i]->codec)) <
                0) {
                LOGI("can not copy the video codec context!\n");
                return ret;
            }
            out_video_stream->codec->codec_tag = 0;
            if (out_fmtctx->oformat->flags & AVFMT_GLOBALHEADER) {
                out_video_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
            }
        } else if (fmtctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream_index = i;
            out_audio_stream = avformat_new_stream(out_fmtctx, NULL);

            if (!out_audio_stream) {
                LOGI("Failed allocating output1 video stream\n");
                ret = AVERROR_UNKNOWN;
                return ret;
            }
            if ((ret = avcodec_copy_context(out_audio_stream->codec, fmtctx->streams[i]->codec)) <
                0) {
                LOGI("can not copy the video codec context!\n");
                return ret;
            }
            out_audio_stream->codec->codec_tag = 0;
            if (out_fmtctx->oformat->flags & AVFMT_GLOBALHEADER) {
                out_audio_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
            }
        }
    }

    //open output file
    if (!(out_fmtctx->oformat->flags & AVFMT_NOFILE)) {
        if ((ret = avio_open(&out_fmtctx->pb, out_name, AVIO_FLAG_WRITE)) < 0) {
            LOGI("can not open the out put file handle!\n");
            return ret;
        }
    }

    //write out  file header
    if ((ret = avformat_write_header(out_fmtctx, NULL)) < 0) {
        LOGI("Error occurred when opening video output file\n");
        return ret;
    }
    return ret;
}

AVFormatContext *chooseInFmt() {
    int videoIndex1 = av_find_best_stream(in1_fmtctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    int frameRate1 = (int) (((float) in1_fmtctx->streams[videoIndex1]->avg_frame_rate.num /
                             (float) in1_fmtctx->streams[videoIndex1]->avg_frame_rate.den) + 0.5);
    int videoIndex2 = av_find_best_stream(in2_fmtctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    int frameRate2 = (int) (((float) in2_fmtctx->streams[videoIndex2]->avg_frame_rate.num /
                             (float) in2_fmtctx->streams[videoIndex2]->avg_frame_rate.den) + 0.5);
    return frameRate1 > frameRate2 ? in2_fmtctx : in1_fmtctx;
}

/**
 * 将两个视频合成为一个
 * @param srcFile1
 * @param srcFile2
 * @param outFile
 * @return
 */
int videoMerge(const char *srcFile1, const char *srcFile2, const char *outFile) {
    int result = 0;
    AVFormatContext *input_ctx;
    AVPacket pkt;
    int pts_v, pts_a, dts_v, dts_a;
    av_register_all();
    if (0 > (result = open_input(srcFile1, srcFile2))) {
        LOGI("result:%d", result);
        goto video_merge_end;
    }
    if (0 > (result = open_output(chooseInFmt(), outFile))) {
        LOGI("result:%d", result);
        goto video_merge_end;
    }
    input_ctx = in1_fmtctx;
    while (1) {
        if (0 > av_read_frame(input_ctx, &pkt)) {
            if (input_ctx == in1_fmtctx) {
                float vedioDuraTime, audioDuraTime;
                //calc the first media dura time
                vedioDuraTime = ((float) input_ctx->streams[video_stream_index]->time_base.num /
                                 (float) input_ctx->streams[video_stream_index]->time_base.den) *
                                ((float) pts_v);
                audioDuraTime = ((float) input_ctx->streams[audio_stream_index]->time_base.num /
                                 (float) input_ctx->streams[audio_stream_index]->time_base.den) *
                                ((float) pts_a);

                //calc the pts and dts end of the first media
                if (audioDuraTime > vedioDuraTime) {
                    dts_v = pts_v = audioDuraTime /
                                    ((float) input_ctx->streams[video_stream_index]->time_base.num /
                                     (float) input_ctx->streams[video_stream_index]->time_base.den);
                    dts_a++;
                    pts_a++;
                } else {
                    dts_a = pts_a = vedioDuraTime /
                                    ((float) input_ctx->streams[audio_stream_index]->time_base.num /
                                     (float) input_ctx->streams[audio_stream_index]->time_base.den);
                    dts_v++;
                    pts_v++;
                }
                input_ctx = in2_fmtctx;
                continue;
            }
            break;
        }

        if (pkt.stream_index == video_stream_index) {
            if (input_ctx == in2_fmtctx) {
                pkt.pts += pts_v;
                pkt.dts += dts_v;
            } else {
                pts_v = pkt.pts;
                dts_v = pkt.dts;
            }
        } else if (pkt.stream_index == audio_stream_index) {
            if (input_ctx == in2_fmtctx) {
                pkt.pts += pts_a;
                pkt.dts += dts_a;
            } else {
                pts_a = pkt.pts;
                dts_a = pkt.dts;
            }
        }

        pkt.pts = av_rescale_q_rnd(pkt.pts, input_ctx->streams[pkt.stream_index]->time_base,
                                   out_fmtctx->streams[pkt.stream_index]->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, input_ctx->streams[pkt.stream_index]->time_base,
                                   out_fmtctx->streams[pkt.stream_index]->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.pos = -1;

        if (av_interleaved_write_frame(out_fmtctx, &pkt) < 0) {
            LOGI("Error muxing packet\n");
            //break;
        }
        av_free_packet(&pkt);
    }

    av_write_trailer(out_fmtctx);

    video_merge_end:
    avformat_close_input(&in1_fmtctx);
    avformat_close_input(&in2_fmtctx);
    /* close output */
    if (out_fmtctx && !(out_fmtctx->oformat->flags & AVFMT_NOFILE))
        avio_close(out_fmtctx->pb);
    avformat_free_context(out_fmtctx);
    return result;
}


