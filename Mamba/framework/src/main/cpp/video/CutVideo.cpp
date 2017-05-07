//
// Created by jakechen on 2017/1/17.
//


#include "CutVideo.h"


int cut_video1(double from_seconds, double end_seconds, const char *in_filename,
               const char *out_filename) {
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;
    int ret = -1, i, video_index;
    AVStream *in_stream, *out_stream;
    int64_t *dts_start_from;
    int64_t *pts_start_from;
    double startDecodecKeyFrameTime;
    av_register_all();
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        LOGE("Could not open input file '%s'", in_filename);
        goto end;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        LOGE("Failed to retrieve input stream information");
        goto end;
    }
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
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
    ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
    if (ret < 0) {
        LOGE("Could not open output file '%s'", out_filename);
        goto end;
    }

    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        LOGE("Error occurred when opening output file\n");
        ret = -1;
        goto end;
    }
    while (1) {
          ret = av_read_frame(ifmt_ctx, &pkt); 
        if (ret < 0) {
            break; 
        } 
        if (pkt.stream_index == video_index && pkt.flags == AV_PKT_FLAG_KEY) {
            int pts = av_q2d(in_stream->time_base) * pkt.pts; 
            if (pts > from_seconds) {
                  break; 
            } 
            startDecodecKeyFrameTime = pts; 
        } 
    } 
    ret = av_seek_frame(ifmt_ctx, -1, startDecodecKeyFrameTime * AV_TIME_BASE, AVSEEK_FLAG_ANY);
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
        if (av_q2d(in_stream->time_base) * pkt.pts > end_seconds) {
            av_packet_unref(&pkt);
            break;
        }
        if (dts_start_from[pkt.stream_index] == 0) {
            dts_start_from[pkt.stream_index] = pkt.dts;
        }
        if (pts_start_from[pkt.stream_index] == 0) {
            pts_start_from[pkt.stream_index] = pkt.pts;
        }
        /* copy packet */
        int64_t tempPts = pkt.pts - pts_start_from[pkt.stream_index];
        int64_t tempDts = pkt.dts - dts_start_from[pkt.stream_index];
        pkt.pts = av_rescale_q_rnd(tempPts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(tempDts, in_stream->time_base, out_stream->time_base,
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


int cut_audio(double from_seconds, double end_seconds, const char *in_filename,
              const char *out_filename) {
    if ((access(in_filename, F_OK)) == -1 ) {
        return -1;
    }
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;
    int ret = -1;
    AVStream *in_stream, *out_stream;
    int64_t *dts_start_from;
    int64_t *pts_start_from;
    OutputStreamContext *a_out_ctx = NULL;
    av_register_all();
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        LOGE("Could not open input file '%s'", in_filename);
        goto end;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        LOGE("Failed to retrieve input stream information");
        goto end;
    }
    //生成音频流上下文
    a_out_ctx = applyOutputStreamContext(ifmt_ctx, out_filename,
                                         AVMEDIA_TYPE_AUDIO);
    if (a_out_ctx == NULL) {
        LOGD("apply audio OutputStreamContext fail");
        ret = -1;
        goto end;
    }
    dts_start_from = (int64_t *) malloc(sizeof(int64_t) * ifmt_ctx->nb_streams);
    memset(dts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);
    pts_start_from = (int64_t *) malloc(sizeof(int64_t) * ifmt_ctx->nb_streams);
    memset(pts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);
    while (1) {
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0) {
            break;
        }

        in_stream = ifmt_ctx->streams[pkt.stream_index];
        out_stream = a_out_ctx->stream;
        ofmt_ctx = a_out_ctx->fmt_ctx;
        double time = av_q2d(in_stream->time_base) * pkt.pts;
        LOGD("audio time=%lf", time);
        if (pkt.stream_index != a_out_ctx->streamIndex) {
            continue;
        }
        if (time < from_seconds) {
            continue;
        }
        if (time >= end_seconds) {
            av_packet_unref(&pkt);
            break;
        }

        if (dts_start_from[pkt.stream_index] == 0) {
            dts_start_from[pkt.stream_index] = pkt.dts;
        }
        if (pts_start_from[pkt.stream_index] == 0) {
            pts_start_from[pkt.stream_index] = pkt.pts;
        }
        /* copy packet */
        int64_t tempPts = pkt.pts - pts_start_from[pkt.stream_index];
        int64_t tempDts = pkt.dts - dts_start_from[pkt.stream_index];
        pkt.pts = av_rescale_q_rnd(tempPts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(tempDts, in_stream->time_base, out_stream->time_base,
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
        pkt.stream_index = 0;
        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0) {
            LOGE("av_write_frame packet fail ret=%d", ret);
            continue;
        }
        av_packet_unref(&pkt);
    }
    av_write_trailer(ofmt_ctx);
    ret = 1;
    end:
    if(ret<0){
        remove(out_filename);
    }
    avformat_close_input(&ifmt_ctx);
    avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    return ret;
}


int cut_video(double from_seconds, double end_seconds, const char *in_filename,
              const char *out_filename) {
    VideoOptHandler *handler = new CutVideoOptHandler(from_seconds, end_seconds);
    VideoRecodeFactory *factory = new VideoRecodeFactory(in_filename, out_filename);
    string outOptAac = factory->tempDir + "/outOpt.aac";
    int ret = factory->demuxerAudio();
    if (ret < 0) {
        goto end;
    }
    ret = factory->optVideo(handler);
    if (ret < 0) {
        goto end;
    }
    ret = cut_audio(from_seconds, end_seconds, factory->outAac.c_str(), outOptAac.c_str());
    if (ret < 0) {
        goto end;
    }
    ret = factory->muxer(factory->outOptH264, outOptAac, factory->outFile);
    if (ret < 0) {
        goto end;
    }
    remove(outOptAac.c_str());
    ret = 1;
    end:
    delete (handler);
    delete (factory);
    return ret;
}


CutVideoOptHandler::CutVideoOptHandler(double from_seconds, double end_seconds) {
    this->from_seconds = from_seconds;
    this->end_seconds = end_seconds;

}

AVFrame *CutVideoOptHandler::optAvFrame(AVFrame *frame, int frameIndex) {
    return frame;
}

int CutVideoOptHandler::changeFrameRate(int frameRate) {
    return frameRate;
}

int CutVideoOptHandler::init(AVFormatContext *ifmt_ctx, AVCodecContext *decodecCtx,
                             int video_index, int frameCount) {
    dts_start_from = (int64_t *) malloc(sizeof(int64_t) * ifmt_ctx->nb_streams);
    memset(dts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);
    pts_start_from = (int64_t *) malloc(sizeof(int64_t) * ifmt_ctx->nb_streams);
    memset(pts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);
    return 1;
}

void CutVideoOptHandler::changeTimebase(AVStream *in_stream, AVStream *out_stream, AVPacket pkt) {
    if (dts_start_from[pkt.stream_index] == 0) {
        dts_start_from[pkt.stream_index] = pkt.dts;
    }
    if (pts_start_from[pkt.stream_index] == 0) {
        pts_start_from[pkt.stream_index] = pkt.pts;
    }
    /* copy packet */
    int64_t tempPts = pkt.pts - pts_start_from[pkt.stream_index];
    int64_t tempDts = pkt.dts - dts_start_from[pkt.stream_index];
    pkt.pts = av_rescale_q_rnd(tempPts, in_stream->time_base, out_stream->time_base,
                               (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
    pkt.dts = av_rescale_q_rnd(tempDts, in_stream->time_base, out_stream->time_base,
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
}

void CutVideoOptHandler::releaseInEnd() {
    free(dts_start_from);
    free(pts_start_from);
}

AllowEncodeAndWriteFrameState
CutVideoOptHandler::allowEncodeAndWriteFrame(AVStream *in_stream, AVPacket pkt, int frameIndex) {
    double time = av_q2d(in_stream->time_base) * pkt.pts;
    AllowEncodeAndWriteFrameState state = STATE_ALLOW;
    if (time < this->from_seconds) {
        state = STATE_NOT_ALLOW;
    } else if (time >= this->from_seconds && time < end_seconds) {
        state = STATE_ALLOW;
    } else {
        state = STATE_FINISH_WRITE;
    }
    return state;
}

//
//in_stream = ifmt_ctx->streams[audio_index];
//if (in_stream == NULL) {
//LOGD("audio stream is null "); 
//ret = -1;
//goto end;
//}
//
//out_stream->codecpar->codec_tag = 0;
//