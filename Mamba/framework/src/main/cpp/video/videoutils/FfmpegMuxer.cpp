//
// Created by jakechen on 2017/5/10.
//


#include "FfmpegMuxer.h"

namespace video {

    int muxing(const char *in_filename_v, const char *in_filename_a, const char *out_filename) {
        return muxing(in_filename_v, in_filename_a, out_filename, NULL);
    }

    int muxing(const char *in_filename_v, const char *in_filename_a, const char *out_filename,
               const char *rotate) {
        if ((access(in_filename_v, F_OK)) == -1 || (access(in_filename_a, F_OK)) == -1) {
            return -1;
        }
        AVOutputFormat *ofmt = NULL;
        AVFormatContext *ifmt_ctx_v = NULL, *ifmt_ctx_a = NULL, *ofmt_ctx = NULL;
        AVPacket pkt;
        int ret, i;
        int videoindex_v = -1, videoindex_out = -1;
        int audioindex_a = -1, audioindex_out = -1;
        int frame_index = 0;
        int64_t cur_pts_v = 0, cur_pts_a = 0;
        av_register_all();
        if ((ret = avformat_open_input(&ifmt_ctx_v, in_filename_v, 0, 0)) < 0) {
            LOGD("Could not open input file.");
            goto end;
        }
        if ((ret = avformat_find_stream_info(ifmt_ctx_v, 0)) < 0) {
            LOGD("Failed to retrieve input stream information");
            goto end;
        }

        if ((ret = avformat_open_input(&ifmt_ctx_a, in_filename_a, 0, 0)) < 0) {
            LOGD("Could not open input file.");
            goto end;
        }
        if ((ret = avformat_find_stream_info(ifmt_ctx_a, 0)) < 0) {
            LOGD("Failed to retrieve input stream information");
            goto end;
        }
        //Output
        avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
        if (!ofmt_ctx) {
            LOGD("Could not create output context\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }
        ofmt = ofmt_ctx->oformat;

        for (i = 0; i < ifmt_ctx_v->nb_streams; i++) {
            //Create output AVStream according to input AVStream
            if (ifmt_ctx_v->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
                AVStream *in_stream = ifmt_ctx_v->streams[i];
                AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
                videoindex_v = i;
                if (!out_stream) {
                    LOGD("Failed allocating output stream\n");
                    ret = AVERROR_UNKNOWN;
                    goto end;
                }
                videoindex_out = out_stream->index;
                //Copy the settings of AVCodecContext
                if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
                    LOGD("Failed to copy context from input to output stream codec context\n");
                    goto end;
                }
                if (rotate != NULL) {
                    av_dict_set(&out_stream->metadata, "rotate", rotate, 0);
                }
                out_stream->codec->codec_tag = 0;
                if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                    out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
                break;
            }
        }

        for (i = 0; i < ifmt_ctx_a->nb_streams; i++) {
            //Create output AVStream according to input AVStream
            if (ifmt_ctx_a->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
                AVStream *in_stream = ifmt_ctx_a->streams[i];
                AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
                audioindex_a = i;
                if (!out_stream) {
                    LOGD("Failed allocating output stream\n");
                    ret = AVERROR_UNKNOWN;
                    goto end;
                }
                audioindex_out = out_stream->index;
                //Copy the settings of AVCodecContext
                if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
                    LOGD("Failed to copy context from input to output stream codec context\n");
                    goto end;
                }
                out_stream->codec->codec_tag = 0;
                if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                    out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

                break;
            }
        }
        //Open output file
        if (!(ofmt->flags & AVFMT_NOFILE)) {
            if (avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE) < 0) {
                LOGD("Could not open output file '%s'", out_filename);
                goto end;
            }
        }
        //Write file header
        if (avformat_write_header(ofmt_ctx, NULL) < 0) {
            LOGD("Error occurred when opening output file\n");
            goto end;
        }

        while (1) {
            AVFormatContext *ifmt_ctx;
            int stream_index = 0;
            AVStream *in_stream, *out_stream;

            //Get an AVPacket
            if (av_compare_ts(cur_pts_v, ifmt_ctx_v->streams[videoindex_v]->time_base, cur_pts_a,
                              ifmt_ctx_a->streams[audioindex_a]->time_base) <= 0) {
                ifmt_ctx = ifmt_ctx_v;
                stream_index = videoindex_out;

                if (av_read_frame(ifmt_ctx, &pkt) >= 0) {
                    do {
                        in_stream = ifmt_ctx->streams[pkt.stream_index];
                        out_stream = ofmt_ctx->streams[stream_index];

                        if (pkt.stream_index == videoindex_v) {
                            //FIX£∫No PTS (Example: Raw H.264)
                            //Simple Write PTS
                            if (pkt.pts == AV_NOPTS_VALUE) {
                                //Write PTS
                                AVRational time_base1 = in_stream->time_base;
                                //Duration between 2 frames (us)
                                int64_t calc_duration =
                                        (double) AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
                                //Parameters
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
                ifmt_ctx = ifmt_ctx_a;
                stream_index = audioindex_out;
                if (av_read_frame(ifmt_ctx, &pkt) >= 0) {
                    do {
                        in_stream = ifmt_ctx->streams[pkt.stream_index];
                        out_stream = ofmt_ctx->streams[stream_index];

                        if (pkt.stream_index == audioindex_a) {

                            //FIX£∫No PTS
                            //Simple Write PTS
                            if (pkt.pts == AV_NOPTS_VALUE) {
                                //Write PTS
                                AVRational time_base1 = in_stream->time_base;
                                //Duration between 2 frames (us)
                                int64_t calc_duration =
                                        (double) AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
                                //Parameters
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
            //Write
            if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
                LOGD("Error muxing packet\n");
                break;
            }
            av_free_packet(&pkt);

        }
        //Write file trailer
        av_write_trailer(ofmt_ctx);
        ret = 1;
        end:
        if (ret < 0) {
            remove(out_filename);
        }
        avformat_close_input(&ifmt_ctx_v);
        avformat_close_input(&ifmt_ctx_a);
        LOGD("muxing finish 00000\n");
        /* close output */
        if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
            avio_close(ofmt_ctx->pb);
        LOGD("muxing finish 111111\n");
        avformat_free_context(ofmt_ctx);
        LOGD("muxing finish 222222\n");
        if (ret < 0 && ret != AVERROR_EOF) {
            LOGD("Error occurred.\n");
            return -1;
        }

        LOGD("muxing finish\n");
        return ret;
    }

    int h264ToFormat(const char *in_filename_v, const char *out_filename) {
        return h264ToFormat(in_filename_v, out_filename, NULL);
    }

    int h264ToFormat(const char *in_filename_v, const char *out_filename, const char *rotate) {
        LOGD("h264ToFormat in_filename_v = %s out_filename = %s\n", in_filename_v, out_filename);
        if ((access(in_filename_v, F_OK)) == -1) {
            LOGD("h264ToFormat in_filename_v err\n");
            return -1;
        }
        if (access(out_filename, F_OK) != -1) {
            remove(out_filename);
        }
        AVOutputFormat *ofmt = NULL;
        AVFormatContext *ifmt_ctx_v = NULL, *ofmt_ctx = NULL;
        AVPacket pkt;
        int ret = -1, i;
        int videoindex_v = -1, videoindex_out = -1;
        int frame_index = 0;
        av_register_all();
        if ((ret = avformat_open_input(&ifmt_ctx_v, in_filename_v, 0, 0)) < 0) {
            LOGD("Could not open input file.");
            goto end;
        }
        if ((ret = avformat_find_stream_info(ifmt_ctx_v, 0)) < 0) {
            LOGD("Failed to retrieve input stream information");
            goto end;
        }

        //Output
        avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
        if (!ofmt_ctx) {
            LOGD("Could not create output context\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }
        ofmt = ofmt_ctx->oformat;

        for (i = 0; i < ifmt_ctx_v->nb_streams; i++) {
            if (ifmt_ctx_v->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
                AVStream *in_stream = ifmt_ctx_v->streams[i];
                AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
                videoindex_v = i;
                if (!out_stream) {
                    LOGD("Failed allocating output stream\n");
                    ret = AVERROR_UNKNOWN;
                    goto end;
                }
                videoindex_out = out_stream->index;
                //Copy the settings of AVCodecContext
                if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
                    LOGD("Failed to copy context from input to output stream codec context\n");
                    ret = -1;
                    goto end;
                }
                if (rotate != NULL) {
                    av_dict_set(&out_stream->metadata, "rotate", rotate, 0);
                }
                out_stream->codec->codec_tag = 0;
                if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                    out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
                break;
            }
        }
        //Open output file
        if (!(ofmt->flags & AVFMT_NOFILE)) {
            if (avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE) < 0) {
                LOGD("Could not open output file '%s'", out_filename);
                ret = -1;
                goto end;
            }
        }
        //Write file header
        if (avformat_write_header(ofmt_ctx, NULL) < 0) {
            LOGD("Error occurred when opening output file\n");
            ret = -1;
            goto end;
        }


        while (1) {
            int stream_index = 0;
            AVStream *in_stream, *out_stream;
            //Get an AVPacket
            stream_index = videoindex_out;
            if (av_read_frame(ifmt_ctx_v, &pkt) >= 0) {
                do {
                    in_stream = ifmt_ctx_v->streams[pkt.stream_index];
                    out_stream = ofmt_ctx->streams[stream_index];

                    if (pkt.stream_index == videoindex_v) {
                        if (pkt.pts == AV_NOPTS_VALUE) {
                            //Write PTS
                            AVRational time_base1 = in_stream->time_base;
                            //Duration between 2 frames (us)
                            int64_t calc_duration =
                                    (double) AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
                            //Parameters
                            pkt.pts = (double) (frame_index * calc_duration) /
                                      (double) (av_q2d(time_base1) * AV_TIME_BASE);
                            pkt.dts = pkt.pts;
                            pkt.duration = (double) calc_duration /
                                           (double) (av_q2d(time_base1) * AV_TIME_BASE);
                            frame_index++;
                        }
                        break;
                    }
                } while (av_read_frame(ifmt_ctx_v, &pkt) >= 0);
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
            //Write
            if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
                LOGE("Error muxing packet\n");
                ret = -1;
                av_free_packet(&pkt);
                goto end;
                //break;
            }
            av_free_packet(&pkt);
        }
        //Write file trailer
        av_write_trailer(ofmt_ctx);
        ret = 1;
        end:
        if (ret < 0) {
            remove(out_filename);
        }
        avformat_close_input(&ifmt_ctx_v);
        /* close output */
        if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
            avio_close(ofmt_ctx->pb);
        avformat_free_context(ofmt_ctx);
        if (ret < 0 && ret != AVERROR_EOF) {
            LOGD("Error occurred.\n");
            return -1;
        }
        return ret;
    }

    int remuxing(const char *srcFile, const char *outFile) {
        return remuxing(srcFile, outFile, NULL);
    }

    int remuxing(const char *srcFile, const char *outFile, const char *rotate) {
        if ((access(srcFile, F_OK)) == -1) {
            return -1;
        }
        AVOutputFormat *ofmt = NULL;
        AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
        AVPacket pkt;
        int ret, i;
        av_register_all();
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
        ofmt = ofmt_ctx->oformat;
        for (i = 0; i < ifmt_ctx->nb_streams; i++) {
            AVStream *in_stream = ifmt_ctx->streams[i];
            AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
            if (!out_stream) {
                LOGE("Failed allocating output stream\n");
                ret = AVERROR_UNKNOWN;
                goto end;
            }

            ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
            if (ret < 0) {
                LOGE("Failed to copy context from input to output stream codec context\n");
                goto end;
            }
            out_stream->codec->codec_tag = 0;
            if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
            if (out_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                if (rotate != NULL) {
                    av_dict_set(&out_stream->metadata, "rotate", rotate, 0);
                }
            }
        }
        if (!(ofmt->flags & AVFMT_NOFILE)) {
            ret = avio_open(&ofmt_ctx->pb, outFile, AVIO_FLAG_WRITE);
            if (ret < 0) {
                LOGE("Could not open output file '%s'", outFile);
                goto end;
            }
        }
        ret = avformat_write_header(ofmt_ctx, NULL);
        if (ret < 0) {
            LOGE("Error occurred when opening output file\n");
            goto end;
        }
        while (1) {
            AVStream *in_stream, *out_stream;

            ret = av_read_frame(ifmt_ctx, &pkt);
            if (ret < 0) {
                break;
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
                LOGE("Error muxing packet\n");
                break;
            }
            av_packet_unref(&pkt);
        }
        av_write_trailer(ofmt_ctx);
        ret = 1;
        end:
        if (ret < 0) {
            remove(outFile);
        }
        avformat_close_input(&ifmt_ctx);
        /* close output */
        if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE)) {
            avio_closep(&ofmt_ctx->pb);
        }
        avformat_free_context(ofmt_ctx);

        return ret;
    }
}