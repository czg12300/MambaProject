//
// Created by jakechen on 2017/2/23.
//

#include <FFmpegUtil.h>
#include <Demuxer.h>
#include "Filter.h"


static AVFilterContext *buffersink_ctx;
static AVFilterContext *buffersrc_ctx;
static AVFilterGraph *filter_graph;


static int
init_filters(AVCodecContext *decodecCtx, AVStream *avStream, const char *filters_descr) {
    char args[512];
    int ret = 0;
    AVFilter *buffersrc = avfilter_get_by_name("buffer");
    AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();
    AVRational time_base = avStream->time_base;
    enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE};
    filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             decodecCtx->width, decodecCtx->height, decodecCtx->pix_fmt,
             time_base.num, time_base.den,
             decodecCtx->sample_aspect_ratio.num, decodecCtx->sample_aspect_ratio.den);

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

    ret = av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts,
                              AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
        goto end;
    }

    /*
     * Set the endpoints for the filter graph. The filter_graph will
     * be linked to the graph described by filters_descr.
     */

    /*
     * The buffer source output must be connected to the input pad of
     * the first filter described by filters_descr; since the first
     * filter input label is not specified, it is set to "in" by
     * default.
     */
    outputs->name = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    /*
     * The buffer sink input must be connected to the output pad of
     * the last filter described by filters_descr; since the last
     * filter output label is not specified, it is set to "out" by
     * default.
     */
    inputs->name = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = NULL;
    if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr,
                                        &inputs, &outputs, NULL)) < 0)
        goto end;

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        goto end;
    end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
}

static AVPacket
encoder_frame(AVCodecContext *ecodecCtx, AVFrame *frame) {
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL; // packet data will be allocated by the encoder
    pkt.size = 0;
    if (encodec_video_frame(ecodecCtx, frame, &pkt) < 0) {
        av_packet_unref(&pkt);
        LOGE("Error muxing packet\n");
    }
    return pkt;
}

static int
addWatermarkToH264(const char *watermarkCommand, const char *srcFile, const char *outFile) {
    AVFormatContext *ifmt_ctx = NULL;
    AVPacket pkt;
    int video_index = -1, ret = -1;
    AVCodecContext *decodecCtx = NULL;
    AVCodecContext *ecodecCtx = NULL;
    AVFrame *filt_frame;
    AVFrame *frame;
    int frameRate = 0;
    FILE *fp_out;
    av_register_all();
    avfilter_register_all();
    if ((ret = avformat_open_input(&ifmt_ctx, srcFile, 0, 0)) < 0) {
        LOGE("Could not open input file '%s'", srcFile);
        goto end;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        LOGE("Failed to retrieve input stream information");
        goto end;
    }
    if ((fp_out = fopen(outFile, "wb")) == NULL) {
        LOGD("open file fail");
        return -1;
    }
    video_index = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0); 
    if (video_index == -1) {
        LOGD("can't find video stream in %s\n", ifmt_ctx->filename); 
        ret = -1; 
        goto end; 
    }
    decodecCtx = open_decodec_context(ifmt_ctx->streams[video_index]);
    if (decodecCtx == NULL) {
        LOGE("Could not open codec!");
        goto end;
    }
    if ((ret = init_filters(decodecCtx, ifmt_ctx->streams[video_index], watermarkCommand)) < 0) {
        LOGE("init_filters fail");
        goto end;
    }
    frameRate = ifmt_ctx->streams[video_index]->avg_frame_rate.num /
                ifmt_ctx->streams[video_index]->avg_frame_rate.den;

    LOGD("帧率：%d", frameRate);
    ecodecCtx = applay_encoder_form_decoder(ifmt_ctx->streams[video_index], frameRate);
    //给解码帧申请内存
    filt_frame = av_frame_alloc();
    if (!filt_frame) {
        ret = -1;
        goto end;
    }
    frame = av_frame_alloc();
    if (!frame) {
        ret = -1;
        goto end;
    }
    while (1) {
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0) {
            break;
        }
        if (pkt.stream_index == video_index) {
//的作用是解码一帧视频数据。输入一个压缩编码的结构体AVPacket，输出一个解码后的结构体AVFrame。该函数的声明位于libavcodec\avcodec.h
            ret = avcodec_send_packet(decodecCtx, &pkt);
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                LOGE("avcodec_send_packet fail");
                continue;
            }
            //从解码器返回解码输出数据
            ret = avcodec_receive_frame(decodecCtx, frame);
            if (ret < 0 && ret != AVERROR_EOF) {
                LOGE("avcodec_receive_frame fail");
                continue;
            }
            frame->pts = av_frame_get_best_effort_timestamp(frame);
            /* push the decoded frame into the filtergraph */
            if (av_buffersrc_add_frame_flags(buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_KEEP_REF) <
                0) {
                LOGE(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
                continue;
            }
            /* pull filtered frames from the filtergraph */
            LOGD("while (1)");
            ret = av_buffersink_get_frame(buffersink_ctx, filt_frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF || ret < 0) {
                LOGD("av_buffersink_get_frame fail");
                continue;
            }
            LOGD("av_buffersink_get_frame");
//            if (filt_frame->format == AV_PIX_FMT_YUV420P) {
//                //Y, U, V
//                for (int i = 0; i < filt_frame->height; i++) {
//                    fwrite(filt_frame->data[0] + filt_frame->linesize[0] * i, 1,
//                           filt_frame->width, fp_yuv);
//                }
//                for (int i = 0; i < filt_frame->height / 2; i++) {
//                    fwrite(filt_frame->data[1] + filt_frame->linesize[1] * i, 1,
//                           filt_frame->width / 2, fp_yuv);
//                }
//                for (int i = 0; i < filt_frame->height / 2; i++) {
//                    fwrite(filt_frame->data[2] + filt_frame->linesize[2] * i, 1,
//                           filt_frame->width / 2, fp_yuv);
//                }
//            }
            LOGD("av_init_packet");
            av_init_packet(&pkt);
            pkt.data = NULL; // packet data will be allocated by the encoder
            pkt.size = 0;
            LOGD("encodec_video_frame");
            if (encodec_video_frame(ecodecCtx, frame, &pkt) < 0) {
                av_packet_unref(&pkt);
                LOGE("Error muxing packet\n");
            }
            LOGD("fwrite(pkt.data, 1, pkt.size, fp_out)");
            fwrite(pkt.data, 1, pkt.size, fp_out);
            if (ret < 0) {
                av_packet_unref(&pkt);
                LOGE("Error muxing packet\n");
                break;
            }
            av_packet_unref(&pkt);
        }
    }
    while (1) {//flush encoder
        AVPacket avPacket;
        av_init_packet(&avPacket);
        avPacket.data = NULL; // packet data will be allocated by the encoder
        avPacket.size = 0;
        int ret = encodec_video_frame(ecodecCtx, NULL, &avPacket);
        if (ret < 0) {
            av_packet_unref(&avPacket);
            break;
        }
        fwrite(avPacket.data, 1, avPacket.size, fp_out);
        av_packet_unref(&avPacket);
    }
    ret = 1;
    end:
//    fclose(fp_yuv);
    fclose(fp_out);
    avcodec_close(decodecCtx);
    avcodec_close(ecodecCtx);
    av_frame_free(&filt_frame);
    av_frame_free(&frame);
    avfilter_graph_free(&filter_graph);
    avio_closep(&ifmt_ctx->pb);
    avformat_close_input(&ifmt_ctx);
    avformat_free_context(ifmt_ctx);
    return ret;
}

int addWatermark1(const char *watermarkCommand, const char *srcFile, const char *out_file) {
    LOGD("demuxing start");
    char *out_dir = (char *) malloc(
            sizeof(char) * strlen(out_file) + sizeof(char) * strlen("temp"));
    strcpy(out_dir, out_file);
    strcat(out_dir, "temp");
    if (opendir(out_dir) == NULL) {
        mkdir(out_dir, 0);
    }

    LOGD("demuxing success");
    char *inH264 = (char *) malloc(
            sizeof(char) * strlen(out_dir) + sizeof(char) * strlen("/out.h264"));
    strcpy(inH264, out_dir);
    strcat(inH264, "/out.h264");
    char *inAac = (char *) malloc(
            sizeof(char) * strlen(out_dir) + sizeof(char) * strlen("/out.aac"));
    strcpy(inAac, out_dir);
    strcat(inAac, "/out.aac");
    char *outH264 = (char *) malloc(
            sizeof(char) * strlen(out_dir) + sizeof(char) * strlen("/temp.h264"));
    strcpy(outH264, out_dir);
    strcat(outH264, "/temp.h264");
    int ret = demuxing(srcFile, inH264, inAac);
    if (ret < 0) {
        return -1;
    }
    ret = addWatermarkToH264(watermarkCommand, inH264, outH264);
    if (ret < 0) {
        free(inH264);
        free(outH264);
        return -1;
    }
    LOGD("addWatermarkToH264 success");
    ret = muxing(outH264, inAac, out_file);
    LOGD("muxing success");
    remove(inH264);
    remove(inAac);
    remove(outH264);
    remove(out_dir);
    free(out_dir);
    free(inH264);
    free(inAac);
    free(outH264);
    return ret;
}


int addWatermark0(const char *watermarkCommand, const char *srcFile, const char *outFile) {
    LOGD("addWatermark start");
    int ret;

    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *dec_ctx = NULL;
    int video_stream_index = -1;
    AVPacket packet;
    AVFrame *frame = av_frame_alloc();
    AVFrame *filt_frame = av_frame_alloc();
    FILE *fp_yuv = fopen(outFile, "wb+");
    int got_frame;
    if (!frame || !filt_frame) {
        LOGE("Could not allocate frame");
    }
//    fprintf(stderr, "Usage: %s file\n", srcFile);

    av_register_all();
    avfilter_register_all();
    LOGD("1");
    AVCodec *dec;

    if ((ret = avformat_open_input(&fmt_ctx, srcFile, NULL, NULL)) < 0) {
        LOGD("Cannot open input file\n");
        return ret;
    }

    if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
        LOGD("Cannot find stream information\n");
        return ret;
    }

    /* select the video stream */
    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (ret < 0) {
        LOGD("Cannot find a video stream in the input file\n");
        return ret;
    }
    video_stream_index = ret;
    dec_ctx = fmt_ctx->streams[video_stream_index]->codec;
    av_opt_set_int(dec_ctx, "refcounted_frames", 1, 0);

    /* init the video decoder */
    if ((ret = avcodec_open2(dec_ctx, dec, NULL)) < 0) {
        LOGD("Cannot open video decoder\n");
        return ret;
    }
    LOGD("1");
    if ((ret = init_filters(dec_ctx, fmt_ctx->streams[video_stream_index], watermarkCommand)) < 0) {
        LOGE("init_filters fail");
        goto end;
    }
    LOGD("1");
    /* read all packets */
    while (1) {
        if ((ret = av_read_frame(fmt_ctx, &packet)) < 0)
            break;

        if (packet.stream_index == video_stream_index) {
            //avcodec_get_frame_defaults(frame);
            got_frame = 0;
            ret = avcodec_decode_video2(dec_ctx, frame, &got_frame, &packet);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Error decoding video\n");
                break;
            }

            if (got_frame) {
                frame->pts = av_frame_get_best_effort_timestamp(frame);

                /* push the decoded frame into the filtergraph */
                if (av_buffersrc_add_frame_flags(buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_KEEP_REF) <
                    0) {
                    av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
                    break;
                }

                /* pull filtered frames from the filtergraph */
                while (1) {
                    LOGD("av_buffersink_get_frame while (1)");
                    ret = av_buffersink_get_frame(buffersink_ctx, filt_frame);
                    //ret = av_buffersink_get_frame_flags(buffersink_ctx, frame, 0);
                    //ret = av_buffersink_get_buffer_ref(buffersink_ctx, &picref, 0);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        break;
                    if (ret < 0)
                        goto end;
                    printf("Process 1 frame!\n");

                    if (filt_frame->format == AV_PIX_FMT_YUV420P) {
                        //Y, U, V
                        for (int i = 0; i < filt_frame->height; i++) {
                            fwrite(filt_frame->data[0] + filt_frame->linesize[0] * i, 1,
                                   filt_frame->width, fp_yuv);
                        }
                        for (int i = 0; i < filt_frame->height / 2; i++) {
                            fwrite(filt_frame->data[1] + filt_frame->linesize[1] * i, 1,
                                   filt_frame->width / 2, fp_yuv);
                        }
                        for (int i = 0; i < filt_frame->height / 2; i++) {
                            fwrite(filt_frame->data[2] + filt_frame->linesize[2] * i, 1,
                                   filt_frame->width / 2, fp_yuv);
                        }
                    }

                    //display_frame(filt_frame, buffersink_ctx->inputs[0]->time_base);
                    //av_frame_unref(filt_frame);
                }
                av_frame_unref(frame);
            }
        }
        av_packet_unref(&packet);
    }

    fclose(fp_yuv);
    ret = 1;

    end:

    avfilter_graph_free(&filter_graph);
    avcodec_close(dec_ctx);
    avformat_close_input(&fmt_ctx);
    av_frame_free(&frame);
    av_frame_free(&filt_frame);

    return ret;
}

int addWatermark3(const char *watermarkCommand, const char *srcFile, const char *outFile) {
    AVFormatContext *ifmt_ctx = NULL;
    AVFormatContext *ofmt_ctx = NULL;
    AVPacket pkt;
    int ret = -1, video_index, got_frame, frameRate = 0;
    int vFrameCount = 0;
    AVStream *in_stream, *out_stream;
    AVCodecContext *decodecCtx = NULL;
    AVCodecContext *ecodecCtx = NULL;
    AVFrame *filt_frame;
    AVFrame *frame;
    AVCodec *pCodec;
    av_register_all();
    avfilter_register_all();
    if ((ret = avformat_open_input(&ifmt_ctx, srcFile, 0, 0)) < 0) {
        LOGE("Could not open input file '%s'", srcFile);
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
    video_index = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &pCodec, 0);
    if (ret < 0) {
        LOGD("Cannot find a video stream in the input file\n");
        goto end;
    }
    decodecCtx = ifmt_ctx->streams[video_index]->codec;
    av_opt_set_int(decodecCtx, "refcounted_frames", 1, 0);
    if (avcodec_open2(decodecCtx, pCodec, NULL) < 0) {
        goto end;
    }
//    video_index = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0); 
//    if (video_index == -1) {
//        LOGD("can't find video stream in %s\n", ifmt_ctx->filename); 
//        ret = -1; 
//        goto end; 
//    }
    for (int i = 0; i < ifmt_ctx->nb_streams; i++) {
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
    LOGD("start open_decodec_context");
    decodecCtx = open_decodec_context(ifmt_ctx->streams[video_index]);
    if (decodecCtx == NULL) {
        LOGE("Could not open codec!");
        goto end;
    }
    LOGD("start init_filters");
    if ((ret = init_filters(decodecCtx, ifmt_ctx->streams[video_index], watermarkCommand)) < 0) {
        LOGE("init_filters fail");
        goto end;
    }
    frameRate = ifmt_ctx->streams[video_index]->avg_frame_rate.num /
                ifmt_ctx->streams[video_index]->avg_frame_rate.den;

    LOGD("帧率：%d", frameRate);
    ecodecCtx = applay_encoder_form_decoder(ifmt_ctx->streams[video_index], frameRate);
    if (ecodecCtx == NULL) {
        LOGE("applay_common_encoder fail");
        goto end;
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
    //给解码帧申请内存
    filt_frame = av_frame_alloc();
    if (!filt_frame) {
        ret = -1;
        goto end;
    }
    frame = av_frame_alloc();
    if (!frame) {
        ret = -1;
        goto end;
    }
    LOGD("start av_read_frame");
    while (1) {
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0) {
            break;
        }

        if (pkt.stream_index == video_index) {
            got_frame = 0;
            ret = avcodec_decode_video2(decodecCtx, frame, &got_frame, &pkt);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Error decoding video\n");
                break;
            }
            if (got_frame) {

                frame->pts = av_frame_get_best_effort_timestamp(frame);
                /* push the decoded frame into the filtergraph */
                if (av_buffersrc_add_frame_flags(buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_KEEP_REF) <
                    0) {
                    LOGE(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
                    continue;
                }
                /* pull filtered frames from the filtergraph */
                ret = av_buffersink_get_frame(buffersink_ctx, filt_frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF || ret < 0) {
                    LOGD("av_buffersink_get_frame fail");
                    continue;
                }
                LOGD("start encode frame");
                av_init_packet(&pkt);
                pkt.data = NULL; // packet data will be allocated by the encoder
                pkt.size = 0;
                LOGD("start encode frame");
                ret = encodec_video_frame(ecodecCtx, filt_frame, &pkt);
                LOGD("start encode frame");
                if (ret < 0) {
                    av_packet_unref(&pkt);
                    LOGE("encodec_video_frame fail");
                    continue;
                }
                pkt.stream_index = video_index;
                LOGD("start encode frame");

            }
            in_stream = ifmt_ctx->streams[pkt.stream_index];
            out_stream = ofmt_ctx->streams[pkt.stream_index];
            /* copy packet */
            pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
                                       (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
                                       (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
            pkt.pos = -1;
            ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
            if (ret < 0) {
                av_packet_unref(&pkt);
                LOGE("Error muxing packet\n");
                continue;
            }
        }
        av_packet_unref(&pkt);

    }
    flushEncoder(ofmt_ctx, ecodecCtx);
    av_write_trailer(ofmt_ctx);
    ret = 1;
    end:

    avfilter_graph_free(&filter_graph);
    av_frame_free(&filt_frame);
    av_frame_free(&frame);
    avcodec_close(decodecCtx);
    avcodec_close(ecodecCtx);
    avio_closep(&ofmt_ctx->pb);
    avio_closep(&ifmt_ctx->pb);
    avformat_close_input(&ifmt_ctx);
    avformat_close_input(&ofmt_ctx);
    avformat_free_context(ifmt_ctx);
    avformat_free_context(ofmt_ctx);
    return ret;
}

