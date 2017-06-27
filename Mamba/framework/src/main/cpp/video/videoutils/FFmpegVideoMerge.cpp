//
// Created by jakechen on 2017/5/12.
//

#include "FFmpegVideoMerge.h"

namespace video {

    static int open_input(AVFormatContext **ctx, const char *file) {
        int ret = -1;
        if ((access(file, F_OK)) == -1) {
            return ret;
        }
        LOGI("open_input in_name:%s", file);
        if ((ret = avformat_open_input(ctx, file, NULL, NULL)) < 0) {
            LOGI("can not open the first input context!\n");
            return ret;
        }
        if ((ret = avformat_find_stream_info(*ctx, NULL)) < 0) {
            LOGI("can not find the first input stream info!\n");
            return ret;
        }
        LOGI("open_input ctx is null:%d", ctx == NULL);
        return ret;
    }

    static int
    open_output(AVFormatContext *fmtctx, AVFormatContext **out_fmtctx, const char *out_name) {
        AVStream *out_video_stream = NULL, *out_audio_stream = NULL;
        int ret = -1;
        if ((ret = avformat_alloc_output_context2(out_fmtctx, NULL, NULL, out_name)) < 0) {
            LOGI("can not alloc context for output!,out_name: %s\n", out_name);
            return ret;
        }

        //new stream for out put
        for (int i = 0; i < fmtctx->nb_streams; i++) {
            if (fmtctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
                LOGD("open_output v_index%d", i);
                out_video_stream = avformat_new_stream(*out_fmtctx, NULL);
                if (!out_video_stream) {
                    LOGI("Failed allocating output1 video stream\n");
                    ret = AVERROR_UNKNOWN;
                    return ret;
                }
                if ((ret = avcodec_copy_context(out_video_stream->codec,
                                                fmtctx->streams[i]->codec)) <
                    0) {
                    LOGI("can not copy the video codec context!\n");
                    return ret;
                }
                out_video_stream->codec->codec_tag = 0;
                if ((*out_fmtctx)->oformat->flags & AVFMT_GLOBALHEADER) {
                    out_video_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
                }
            } else if (fmtctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
                out_audio_stream = avformat_new_stream((*out_fmtctx), NULL);
                LOGD("open_output a_index%d", i);
                if (!out_audio_stream) {
                    LOGI("Failed allocating output1 video stream\n");
                    ret = AVERROR_UNKNOWN;
                    return ret;
                }
                if ((ret = avcodec_copy_context(out_audio_stream->codec,
                                                fmtctx->streams[i]->codec)) <
                    0) {
                    LOGI("can not copy the video codec context!\n");
                    return ret;
                }
                out_audio_stream->codec->codec_tag = 0;
                if ((*out_fmtctx)->oformat->flags & AVFMT_GLOBALHEADER) {
                    out_audio_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
                }
            }
        }

        //open output file
        if (!((*out_fmtctx)->oformat->flags & AVFMT_NOFILE)) {
            if ((ret = avio_open(&(*out_fmtctx)->pb, out_name, AVIO_FLAG_WRITE)) < 0) {
                LOGI("can not open the out put file handle!\n");
                return ret;
            }
        }

        //write out  file header
        if ((ret = avformat_write_header((*out_fmtctx), NULL)) < 0) {
            LOGI("Error occurred when opening video output file\n");
            return ret;
        }
        return ret;
    }


    int merge(queue<string> srcFiles, const char *outFile) {
        int result = -1;
        AVFormatContext *input_ctx = NULL;
        AVFormatContext *output_ctx = NULL;
        AVPacket pkt;
        queue<AVFormatContext *> *inputCtxs;
        bool isFirst = true;
        int pts_v = 0, pts_a = 0, dts_v = 0, dts_a = 0;
        int pts_v_last = 0, pts_a_last = 0, dts_v_last = 0, dts_a_last = 0;
        LOGD("mergeVideo srcFiles.size()=%d", srcFiles.size());
        if (srcFiles.empty()) {
            return -1;
        }
        av_register_all();
        inputCtxs = new queue<AVFormatContext *>;
        LOGD("mergeVideo start");
        while (srcFiles.size() > 0) {
            string in = srcFiles.front();
            srcFiles.pop();
            AVFormatContext *ac = NULL;
            result = open_input(&ac, in.c_str());
            if (ac != NULL && result >= 0) {
                int audio_stream_index = av_find_best_stream(ac, AVMEDIA_TYPE_AUDIO, -1, -1,
                                                             NULL, 0);
                if (audio_stream_index != -1 && audio_stream_index != AVERROR_STREAM_NOT_FOUND) {
                    input_ctx = ac;
                }
                inputCtxs->push(ac);
            } else {
                LOGE("open_input result:%d", result);
                goto end;
            }
        }
        if (0 > (result = open_output(input_ctx, &output_ctx, outFile))) {
            LOGI("open_output result:%d", result);
            goto end;
        }
        input_ctx = inputCtxs->front();
        inputCtxs->pop();
        while (1) {
            int video_stream_index = av_find_best_stream(input_ctx, AVMEDIA_TYPE_VIDEO, -1, -1,
                                                         NULL, 0);
            int audio_stream_index = av_find_best_stream(input_ctx, AVMEDIA_TYPE_AUDIO, -1, -1,
                                                         NULL, 0);
            if (0 > av_read_frame(input_ctx, &pkt)) {
                pts_v = pts_v_last + 1;
                pts_a = pts_a_last + 1;
                dts_a = dts_a_last + 1;
                dts_v = dts_v_last + 1;
                LOGD(" pts_v %d  pts_a %d", pts_v, pts_a);
                if (audio_stream_index != AVERROR_STREAM_NOT_FOUND) {
                    if (pts_v > pts_a) {
                        pts_a = pts_v;
                        dts_a = dts_v;
                    }
                }
                avformat_close_input(&input_ctx);
                avformat_free_context(input_ctx);
                if (inputCtxs->size() > 0) {
                    isFirst = false;
                    input_ctx = inputCtxs->front();
                    inputCtxs->pop();
                } else {
                    break;
                }
            }
            if (pkt.stream_index == video_stream_index) {
                if (!isFirst) {
                    pkt.pts += pts_v;
                    pkt.dts += dts_v;
                } else {
                    pts_v = pkt.pts;
                    dts_v = pkt.dts;
                }
                pts_v_last = pkt.pts;
                dts_v_last = pkt.dts;
            } else if (pkt.stream_index == audio_stream_index) {
                if (!isFirst) {
                    pkt.pts += pts_a;
                    pkt.dts += dts_a;
                } else {
                    pts_a = pkt.pts;
                    dts_a = pkt.dts;
                }
                pts_a_last = pkt.pts;
                dts_a_last = pkt.dts;
            }
            pkt.pts = av_rescale_q_rnd(pkt.pts, input_ctx->streams[pkt.stream_index]->time_base,
                                       output_ctx->streams[pkt.stream_index]->time_base,
                                       (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pkt.dts = av_rescale_q_rnd(pkt.dts, input_ctx->streams[pkt.stream_index]->time_base,
                                       output_ctx->streams[pkt.stream_index]->time_base,
                                       (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pkt.duration = av_rescale_q(pkt.duration,
                                        input_ctx->streams[pkt.stream_index]->time_base,
                                        output_ctx->streams[pkt.stream_index]->time_base);
            int ret = av_interleaved_write_frame(output_ctx, &pkt);
            if (ret < 0) {
                LOGE("Error muxing packet ret=%d pkt.streamIndex %d  video_stream_index  %d  audio_stream_index  %d   pkt.pts-pkt.dts  %lld ",
                     ret, pkt.stream_index, video_stream_index, audio_stream_index,
                     pkt.pts - pkt.dts);
                //break;
            }
            av_free_packet(&pkt);
        }

        av_write_trailer(output_ctx);
        LOGD("mergeVideo success");
        result = 1;
        end:
        free(inputCtxs);
        if (output_ctx && !(output_ctx->oformat->flags & AVFMT_NOFILE))
            avio_close(output_ctx
                               ->pb);
        avformat_free_context(output_ctx);
        return
                result;
    }
}