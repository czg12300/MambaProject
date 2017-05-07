//
// Created by walljiang on 2017/04/27.
//

#include "videoFilter.h"
#include "Log.h"

extern "C" {
#include "libavformat/avformat.h"
#include <libavfilter/avfilter.h>
#include <libavutil/opt.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
}

typedef struct Filter_Args {
    int width;
    int height;
    enum AVPixelFormat pix_fmt;
    AVRational time_base;
    AVRational sample_aspect_ratio;
} FilterArgs;

static AVFormatContext *ifmt_ctx;
static AVFormatContext *ofmt_ctx;
static int videoStreamIdx, audioStreamIdx;
static AVFilterGraph *filter_graph;
static AVFilterContext *buffersrc_ctx;
static AVFilterContext *buffersink_ctx;

int init_filters(const char *filters_descr, FilterArgs *filter_args) {
    char args[512];
    int ret = 0;
    AVFilter *buffersrc = avfilter_get_by_name("buffer");
    AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();

    enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_YUV420P,
                                     AV_PIX_FMT_NONE};//modify by elesos.com ?OEAE????oe??????
    if (filters_descr == NULL) {
        printf("%s\nLine %d:%s : filter_args == NULL or filter_descr == NULL\n", __FILE__, __LINE__,
               __func__);
        goto end;
    }
    filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    if (filter_args) {
        LOGI("");
        snprintf(args, sizeof(args),
                 "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                 filter_args->width, filter_args->height, filter_args->pix_fmt,
                 filter_args->time_base.num, filter_args->time_base.den,
                 filter_args->sample_aspect_ratio.num, filter_args->sample_aspect_ratio.den);
    } else {
        AVCodecContext *pCodecCtx = ifmt_ctx->streams[videoStreamIdx]->codec;
        LOGI("outframerate:%d", pCodecCtx->time_base.den);
        snprintf(args, sizeof(args),
                 "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
                 pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                 pCodecCtx->time_base.num, pCodecCtx->time_base.den,
                 pCodecCtx->sample_aspect_ratio.num, pCodecCtx->sample_aspect_ratio.den);
    }
    puts(args);
    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
                                       args, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
        goto end;
    }
    /* buffer video sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                                       NULL, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
        goto end;

    }
    ret = av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE,
                              AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
        goto end;
    }

    /* Endpoints for the filter graph. */
    outputs->name = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = NULL;
    inputs->name = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = NULL;

    if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr, &inputs, &outputs, NULL)) < 0)
        goto end;

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)

        goto end;
    end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    return ret;
}

int Filter_One_Frame(AVFrame *frame, AVFrame *filt_frame) {
    int ret;
    frame->pts = av_frame_get_best_effort_timestamp(frame);
    /* push the decoded frame into the filtergraph */
    if ((ret = av_buffersrc_add_frame(buffersrc_ctx, frame)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
        return ret;
    }
    /* pull filtered pictures from the filtergraph */
    while (1) {
        ret = av_buffersink_get_frame(buffersink_ctx, filt_frame);//过滤镜
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        if (ret < 0)
            return ret;
    }
    ret = 0;
    if (ret < 0 && ret != AVERROR_EOF) {
        char buf[1024];
        av_strerror(ret, buf, sizeof(buf));
        fprintf(stderr, "Error occurred: %s\n", buf);
        return ret;
    }
    return 0;
}

static int open_input_file(const char *filename, int *video_index, int *audio_index) {
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

static int open_output_video(const char *filename) {
    AVStream *out_stream;
    AVStream *in_stream;
    AVCodecContext *dec_ctx, *enc_ctx;
    AVCodec *encoder;
    int ret;
    unsigned int i;
    ofmt_ctx = NULL;
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);
    if (!ofmt_ctx) {
        LOGE("Could not create output context\n");
        return AVERROR_UNKNOWN;
    }
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            LOGE("Failed allocating output stream\n");
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
                enc_ctx->qmin = 3;//dec_ctx->qmin;
                enc_ctx->qmax = 30;//dec_ctx->qmax;
                enc_ctx->qcompress = 1;//dec_ctx->qcompress;
                enc_ctx->time_base.num = 1;
                LOGI("encoder.timebase.den:%d", dec_ctx->time_base.den);
                enc_ctx->time_base.den = 14;
            } else {
                enc_ctx->sample_rate = dec_ctx->sample_rate;
                enc_ctx->channel_layout = dec_ctx->channel_layout;
                enc_ctx->channels = av_get_channel_layout_nb_channels(enc_ctx->channel_layout);
                /* take first format from list of supported formats */
                enc_ctx->sample_fmt = encoder->sample_fmts[0];
                AVRational time_base = {1, enc_ctx->sample_rate};
                enc_ctx->time_base = time_base;
            }
            /* Third parameter can be used to pass settings to encoder */
            ret = avcodec_open2(enc_ctx, encoder, NULL);
            if (ret < 0) {
                LOGE("Cannot open video encoder for stream #%u\n", i);
                return ret;
            }
        } else if (dec_ctx->codec_type == AVMEDIA_TYPE_UNKNOWN) {
            av_log(NULL, AV_LOG_FATAL, "Elementary stream #%d is of unknown type, cannot proceed\n",
                   i);
            return AVERROR_INVALIDDATA;
        } else {
            /* if this stream must be remuxed */
            ret = avcodec_copy_context(ofmt_ctx->streams[i]->codec,
                                       ifmt_ctx->streams[i]->codec);
            if (ret < 0) {
                LOGE("Copying stream context failed\n");
                return ret;
            }
        }
    }

    av_dump_format(ofmt_ctx, 0, filename, 1);
    if (!(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        LOGI("ofmt_ctx->oformat->flags：%d\n", ofmt_ctx->oformat->flags);
        ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            LOGE("Could not open output file '%s'", filename);
            return ret;
        }
    }
    /* init muxer, write output file header */
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        LOGE("Error occurred when opening output file\n");
        return ret;
    }
    return 0;
}

static int decode_packet(AVPacket *packet, AVFrame **frame, int *got_frame) {
    *got_frame = 0;
    int ret = 0;
    if (!frame) {
        LOGE("Alloc frame fail!\n");
        return -1;
    }

    if (packet->stream_index == videoStreamIdx) {
        ret = avcodec_decode_video2(ifmt_ctx->streams[(AVMediaType) videoStreamIdx]->codec, *frame,
                                    got_frame, packet);
        if (ret < 0) {
            LOGE("Error: decodec video frame failed\n");
            return ret;
        }
    } else {
        AVStream *in_stream = ifmt_ctx->streams[(AVMediaType) audioStreamIdx];
        AVStream *out_stream = ofmt_ctx->streams[audioStreamIdx];
        packet->pts = av_rescale_q_rnd(packet->pts, in_stream->time_base, out_stream->time_base,
                                       (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        packet->dts = av_rescale_q_rnd(packet->dts, in_stream->time_base, out_stream->time_base,
                                       (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        packet->duration = av_rescale_q(packet->duration, in_stream->time_base,
                                        out_stream->time_base);
        packet->pos = -1;
        packet->stream_index = audioStreamIdx;  // ??????????????????

        if ((ret = av_interleaved_write_frame(ofmt_ctx, packet)) < 0) {
            LOGE("av_interleaved_write_frame failed\n");
            return -1;
        }

        //av_free_packet(packet);
    }
    return ret;
}

int videoFilter(const char *path, const char *outpath, const char *filterargs) {
    int ret, width, height;
    AVPacket packet, enc_pkt;
    AVFrame *frame;
    int got_frame, got_picture;
    AVCodecContext *pEncodeCtx;
    FilterArgs args;
    av_register_all();
    avfilter_register_all();

    if (open_input_file(path, &videoStreamIdx, &audioStreamIdx) < 0) {
        LOGE("Open input file error\n");
        goto end;
    }
    LOGI("audioStreamIdx:%d，videoStreamIdx:%d\n", audioStreamIdx, videoStreamIdx);
    if (open_output_video(outpath) < 0) {
        goto end;
    }
    pEncodeCtx = ofmt_ctx->streams[videoStreamIdx]->codec;
    args.width = (ifmt_ctx->streams[videoStreamIdx]->codecpar->width);
    args.height = (ifmt_ctx->streams[videoStreamIdx]->codecpar->height);
    args.time_base = (ifmt_ctx->streams[videoStreamIdx]->time_base);
    args.pix_fmt = (AVPixelFormat) (ifmt_ctx->streams[videoStreamIdx]->codecpar->format);
    args.sample_aspect_ratio = (ifmt_ctx->streams[videoStreamIdx]->sample_aspect_ratio);
    if ((
                ret = init_filters(filterargs, &args)
        ) < 0) {
        LOGE("init_filters failed");
        goto
                end;
    }
    av_init_packet(&enc_pkt);
    av_init_packet(&packet);
    frame = av_frame_alloc();
//    av_seek_frame(ifmt_ctx,videoStreamIdx,keyDts[keyDts.size()-2],AVSEEK_FLAG_FRAME);
    while (1) {
//AVSEEK_FLAG_ANY和AVSEEK_FLAG_FRAME都是向后seek，AVSEEK_FLAG_BACKWARD是向前seek
        if ((
                    ret = av_read_frame(ifmt_ctx, &packet)
            ) < 0) {
            LOGE("av_read_frame failed:%d\n", ret);
            ret = 0;
            break;
        }
        ret = decode_packet(&packet, &frame, &got_frame);
        if (ret < 0) {
            LOGE("decode_packet < 0 :%d \n", ret);
            break;
        }
        if (got_frame) {
            AVFrame *filt_frame = av_frame_alloc();
            Filter_One_Frame(frame, filt_frame
            );
            if (filt_frame) {
                int ret = avcodec_encode_video2(pEncodeCtx, &enc_pkt, filt_frame, &got_picture);
                if (ret < 0) {
                    LOGE("Failed to encode!\n");
                    goto
                            end;
                }
                if (!(got_picture)) {
                    continue;
                }
                enc_pkt.
                        stream_index = videoStreamIdx;
                enc_pkt.
                        dts = av_rescale_q_rnd(packet.dts,
                                               ifmt_ctx->streams[videoStreamIdx]->time_base,
                                               ofmt_ctx->streams[videoStreamIdx]->time_base,
                                               (AVRounding) (AV_ROUND_NEAR_INF |
                                                             AV_ROUND_PASS_MINMAX));
                enc_pkt.
                        pts = av_rescale_q_rnd(packet.pts,
                                               ifmt_ctx->streams[videoStreamIdx]->time_base,
                                               ofmt_ctx->streams[videoStreamIdx]->time_base,
                                               (AVRounding) (AV_ROUND_NEAR_INF |
                                                             AV_ROUND_PASS_MINMAX));
                enc_pkt.
                        duration = av_rescale_q(packet.duration,
                                                ifmt_ctx->streams[videoStreamIdx]->time_base,
                                                ofmt_ctx->streams[videoStreamIdx]->time_base);
                LOGI("WRITE PTS22:%lld,%lld", enc_pkt.pts, packet.pts);
                ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
                if (ret < 0) {
                    break;
                }
            }
            av_frame_free(&filt_frame);
        }
    }
    av_write_trailer(ofmt_ctx);
    end:
    if (ifmt_ctx != NULL) {
        avcodec_close(ifmt_ctx
                              ->streams[videoStreamIdx]->codec);
        avcodec_close(ifmt_ctx
                              ->streams[audioStreamIdx]->codec);
        avformat_free_context(ifmt_ctx);
        ifmt_ctx == NULL;
    }
    if (ofmt_ctx != NULL) {
        avcodec_close(ofmt_ctx
                              ->streams[videoStreamIdx]->codec);
        avcodec_close(ofmt_ctx
                              ->streams[audioStreamIdx]->codec);
        avformat_free_context(ofmt_ctx);
        ofmt_ctx == NULL;
    }
    if (packet.size > 0) {
        packet.
                size = 0;
        av_packet_unref(&packet);
    }
    if (enc_pkt.size > 0) {
        enc_pkt.
                size = 0;
        av_packet_unref(&enc_pkt);
    }
    if (frame != NULL) {
        av_frame_free(&frame);
        frame = NULL;
    }
    if (filter_graph != NULL)
        avfilter_graph_free(&filter_graph);//释放
    return
            ret;
}
