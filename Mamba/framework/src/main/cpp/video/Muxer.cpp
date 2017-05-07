//
// Created by jakechen on 2017/1/16.
//

#include "Muxer.h"
#include "Demuxer.h"

/**
 *  根据输入文件和类型，生成输入文件上下文
 * @param ofmt_ctx 输出文件的上下文
 * @param file  输入的媒体文件
 * @param type 输入的媒体文件类型
 * @return
 */
static InputStreamContext *
applyInputStreamContext(AVFormatContext *ofmt_ctx, const char *file, AVMediaType type);

static InputStreamContext *
applyInputStreamContext(AVFormatContext *ofmt_ctx, const char *file, AVMediaType type) {
    InputStreamContext *ctx = (InputStreamContext *) malloc(sizeof(InputStreamContext));
    if (ctx == NULL) {
        return NULL;
    }
    ctx->fmt_ctx = NULL;
    if ((avformat_open_input(&ctx->fmt_ctx, file, NULL, NULL)) < 0) {
        LOGE("Could not open input file.");
        free(ctx);
        return NULL;
    }
    if ((avformat_find_stream_info(ctx->fmt_ctx, 0)) < 0) {
        LOGE("Failed to retrieve input stream information");
        avformat_close_input(&ctx->fmt_ctx);
        free(ctx);
        return NULL;
    }
    ctx->in_stream_index = av_find_best_stream(ctx->fmt_ctx, type, -1, -1, NULL, 0);
    if (ctx->in_stream_index == -1) {
        LOGD("can't find video stream in %s\n", file);
        avformat_close_input(&ctx->fmt_ctx);
        free(ctx);
        return NULL;
    }
    AVCodec *codec = avcodec_find_decoder(
            ctx->fmt_ctx->streams[ctx->in_stream_index]->codecpar->codec_id);
    if (codec == NULL) {
        LOGD("input codec is null");
        avformat_close_input(&ctx->fmt_ctx);
        free(ctx);
        return NULL;
    }
    ctx->in_stream = ctx->fmt_ctx->streams[ctx->in_stream_index];
    ctx->out_stream = avformat_new_stream(ofmt_ctx, codec);
    if (ctx->out_stream == NULL) {
        LOGD("avformat_new_stream is null");
        avformat_close_input(&ctx->fmt_ctx);
        free(ctx);
        return NULL;
    }
    ctx->out_stream_index = ctx->out_stream->index;
    int ret = avcodec_parameters_copy(ctx->out_stream->codecpar, ctx->in_stream->codecpar);
    if (ret < 0) {
        LOGD("avformat_new_stream is null");
        avformat_close_input(&ctx->fmt_ctx);
        free(ctx);
        return NULL;
    }
    return ctx;
}

int muxing(const char *in_filename_v, const char *in_filename_a, const char *out_filename) {
    if ((access(in_filename_v, F_OK)) == -1 || (access(in_filename_a, F_OK)) == -1) {
        return -1;
    }
    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ofmt_ctx = NULL;
    AVPacket pkt;
    int ret = -1;
    int frame_index = 0;
    int64_t cur_pts_v = 0, cur_pts_a = 0;
    AVStream *in_stream;
    AVStream *out_stream;
    AVFormatContext *ifmt_ctx;
    int stream_index = 0;
    AVRational time_base1;
    InputStreamContext *v_in_ctx = NULL;
    InputStreamContext *a_in_ctx = NULL;
    av_register_all();
    //初始化输出文件的context
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if (!ofmt_ctx) {
        LOGE("Could not create output context\n");
        ret = -1;
        goto end;
    }
    ofmt = ofmt_ctx->oformat;
    v_in_ctx = applyInputStreamContext(ofmt_ctx, in_filename_v, AVMEDIA_TYPE_VIDEO);
    if (v_in_ctx == NULL) {
        ret = -1;
        goto end;
    }
    a_in_ctx = applyInputStreamContext(ofmt_ctx, in_filename_a, AVMEDIA_TYPE_AUDIO);
    if (a_in_ctx == NULL) {
        ret = -1;
        goto end;
    }

    if (!(ofmt->flags & AVFMT_NOFILE)) {
        if (avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE) < 0) {
            LOGE("Could not open output file '%s'", out_filename);
            ret = -1;
            goto end;
        }
    }
//写入输出文件的头文件
    if (avformat_write_header(ofmt_ctx, NULL) < 0) {
        LOGE("Error occurred when opening output file\n");
        ret = -1;
        goto end;
    }
    while (1) {
//通过时间判断要写入视频帧还是音频帧
        if (av_compare_ts(cur_pts_v, v_in_ctx->in_stream->time_base, cur_pts_a,
                          a_in_ctx->in_stream->time_base) <= 0) {
            ifmt_ctx = v_in_ctx->fmt_ctx;//ifmt_ctx_v;
            stream_index = v_in_ctx->out_stream_index;
            if (av_read_frame(ifmt_ctx, &pkt) >= 0) {
                do {
                    in_stream = v_in_ctx->in_stream;
                    out_stream = v_in_ctx->out_stream;
                    if (pkt.stream_index == v_in_ctx->in_stream_index) {
                        if (pkt.pts == AV_NOPTS_VALUE) {
                            time_base1 = in_stream->time_base;
                            int64_t calc_duration =
                                    (double) AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
                            pkt.pts = (double) (frame_index * calc_duration) /
                                      (double) (av_q2d(time_base1) * AV_TIME_BASE);
                            pkt.dts = pkt.pts;
                            pkt.duration = (double) calc_duration /
                                           (double) (av_q2d(time_base1) * AV_TIME_BASE);
                            frame_index++;
                        }
                        cur_pts_v = pkt.pts;
                        break;
                    }
                } while (av_read_frame(ifmt_ctx, &pkt) >= 0);
            } else {
                break;
            }
        } else {
            ifmt_ctx = a_in_ctx->fmt_ctx;//ifmt_ctx_a;
            stream_index = a_in_ctx->out_stream_index;
            if (av_read_frame(ifmt_ctx, &pkt) >= 0) {
                do {
                    in_stream = ifmt_ctx->streams[pkt.stream_index];
                    out_stream = a_in_ctx->out_stream;
                    if (pkt.stream_index == a_in_ctx->in_stream_index) {
                        if (pkt.pts == AV_NOPTS_VALUE) {
                            time_base1 = in_stream->time_base;
                            int64_t calc_duration =
                                    (double) AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
                            pkt.pts = (double) (frame_index * calc_duration) /
                                      (double) (av_q2d(time_base1) * AV_TIME_BASE);
                            pkt.dts = pkt.pts;
                            pkt.duration = (double) calc_duration /
                                           (double) (av_q2d(time_base1) * AV_TIME_BASE);
                            frame_index++;
                        }
                        cur_pts_a = pkt.pts;

                        break;
                    }
                } while (av_read_frame(ifmt_ctx, &pkt) >= 0);
            } else {
                break;
            }
        }
//Convert PTS/DTS
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        pkt.stream_index = stream_index;
        if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
            LOGE("Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);

    }
//将流预告片写入输出媒体文件并释放文件私有数据。
    av_write_trailer(ofmt_ctx);
    ret = 1;
    end:
    if (ret < 0) {
        remove(out_filename);
    }
    avformat_close_input(&v_in_ctx->fmt_ctx);
    avformat_close_input(&a_in_ctx->fmt_ctx);
    avformat_free_context(v_in_ctx->fmt_ctx);
    avformat_free_context(a_in_ctx->fmt_ctx);
    free(v_in_ctx);
    free(a_in_ctx);
    avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);

    return ret;
}


int
muxingVideoFile(const char *in_filename_v, const char *in_filename_a, const char *out_filename) {
    string temp = in_filename_v;
    temp += ".h264";
    int ret = demuxingVideo(in_filename_v, temp.c_str());
    if (ret < 0) {
        remove(temp.c_str());
        return ret;
    }
    ret = muxing(temp.c_str(), in_filename_a, out_filename);
    remove(temp.c_str());
    return ret;
}


int h264ToFormat(const char *in_filename_v, const char *out_filename) {
    return h264ToFormat(in_filename_v, out_filename, NULL);
}

int h264ToFormat(const char *in_filename_v, const char *out_filename, const char *rotate) {
    if ((access(in_filename_v, F_OK)) == -1) {
        return -1;
    }
    AVOutputFormat *ofmt = NULL;
    AVFormatContext *ofmt_ctx = NULL;
    AVPacket pkt;
    int ret = -1, frameRate;
    int frame_index = 0;
    AVStream *in_stream;
    AVStream *out_stream;
    AVFormatContext *ifmt_ctx;
    int stream_index = 0;
    AVRational time_base1;
    InputStreamContext *v_in_ctx = NULL;
    av_register_all();
    //初始化输出文件的context
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if (!ofmt_ctx) {
        LOGE("Could not create output context\n");
        ret = -1;
        goto end;
    }
    ofmt = ofmt_ctx->oformat;
    v_in_ctx = applyInputStreamContext(ofmt_ctx, in_filename_v, AVMEDIA_TYPE_VIDEO);
    if (v_in_ctx == NULL) {
        ret = -1;
        goto end;
    }

    if (rotate != NULL) {
        av_dict_set(&v_in_ctx->out_stream->metadata, "rotate", rotate, 0);
    }

    if (!(ofmt->flags & AVFMT_NOFILE)) {
        if (avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE) < 0) {
            LOGE("Could not open output file '%s'", out_filename);
            ret = -1;
            goto end;
        }
    }
//写入输出文件的头文件
    if (avformat_write_header(ofmt_ctx, NULL) < 0) {
        LOGE("Error occurred when opening output file\n");
        ret = -1;
        goto end;
    }
    frameRate = v_in_ctx->fmt_ctx->streams[v_in_ctx->in_stream_index]->avg_frame_rate.num /
                v_in_ctx->fmt_ctx->streams[v_in_ctx->in_stream_index]->avg_frame_rate.den;
    LOGD("frameRate=%d", frameRate);
    while (1) {
//通过时间判断要写入视频帧还是音频帧
        ifmt_ctx = v_in_ctx->fmt_ctx;//ifmt_ctx_v;
        stream_index = v_in_ctx->out_stream_index;
        if (av_read_frame(ifmt_ctx, &pkt) >= 0) {
            do {
                in_stream = v_in_ctx->in_stream;
                out_stream = v_in_ctx->out_stream;
                if (pkt.stream_index == v_in_ctx->in_stream_index) {
                    if (pkt.pts == AV_NOPTS_VALUE) {
                        time_base1 = in_stream->time_base;
                        int64_t calc_duration =
                                (double) AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
                        pkt.pts = (double) (frame_index * calc_duration) /
                                  (double) (av_q2d(time_base1) * AV_TIME_BASE);
                        pkt.dts = pkt.pts;
                        pkt.duration = (double) calc_duration /
                                       (double) (av_q2d(time_base1) * AV_TIME_BASE);
                        frame_index++;
                    }
                    break;
                }
            } while (av_read_frame(ifmt_ctx, &pkt) >= 0);
        } else {
            break;
        }
//Convert PTS/DTS
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        pkt.stream_index = stream_index;
        if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
            LOGE("Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);

    }
//将流预告片写入输出媒体文件并释放文件私有数据。
    av_write_trailer(ofmt_ctx);
    ret = 1;
    end:
    if (ret < 0) {
        remove(out_filename);
    }
    avformat_close_input(&v_in_ctx->fmt_ctx);
    avformat_free_context(v_in_ctx->fmt_ctx);
    free(v_in_ctx);
    avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);

    return ret;
}