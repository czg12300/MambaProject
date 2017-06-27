//
// Created by jakechen on 2017/5/12.
//

#include <VideoCodeProduce.h>
#include "FfmpegVideoCut.h"

namespace video {
    int cutVideo(const char *srcFile, long startMillis, long endMillis, const char *outFile) {
        int ret = 0;
        VideoCodeParams *params = new VideoCodeParams();
        VideoCutHandle *handle = new VideoCutHandle(&ret);
        params->srcFile = srcFile;
        params->outFile = outFile;
        params->rangeStart = startMillis;
        params->rangeEnd = endMillis;
        params->effect = handle;
        VideoCodeProduce *produce = new VideoCodeProduce(params);
        produce->start();
        while (ret != 0) {
            usleep(50);
        }
        return ret;
    }

    VideoCutHandle::VideoCutHandle(int *result) {
        this->ret = result;

    }

    void VideoCutHandle::onFail() {
        *ret = -1;
    }

    void VideoCutHandle::onSuccess() {
        *ret = 1;
    }

    long VideoCutHandle::getNeedSeekTime(AVPacket pkt) {
        return -1;
    }

    void VideoCutHandle::init(AVFormatContext *ifmt_ctx, AVFormatContext *ofmt_ctx,
                              AVCodecContext *decode_ctx, AVCodecContext *encode_ctx,
                              AVStream *in_stream, AVStream *out_stream, long rangeStart,
                              long rangeEnd) {}

    bool VideoCutHandle::needDecode() {
        return rangeStart > 0;
    }

    void VideoCutHandle::onReadPackage(AVPacket pkt) {}

    void VideoCutHandle::optFrame(DecodeFrame *frame) {}

    void VideoCutHandle::release() {}

    int cutAudio(const char *srcFile, long startMillis, long endMillis, const char *outFile) {
        if ((access(srcFile, F_OK)) == -1) {
            return -1;
        }
        AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
        AVPacket pkt;
        int ret = -1;
        AVStream *in_stream, *out_stream;
        int64_t *dts_start_from;
        int64_t *pts_start_from;
        OutputStreamContext *a_out_ctx = NULL;
        LOGD("startMillis %lf endMillis %lf", startMillis, endMillis);
        double from_seconds = (double) startMillis / 1000;
        double end_seconds = (double) endMillis / 1000;
        LOGD("from_seconds %lf end_seconds %lf", from_seconds, end_seconds);
        av_register_all();
        if ((ret = avformat_open_input(&ifmt_ctx, srcFile, 0, 0)) < 0) {
            LOGE("Could not open input file '%s'", srcFile);
            goto end;
        }

        if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
            LOGE("Failed to retrieve input stream information");
            goto end;
        }
        //生成音频流上下文
        a_out_ctx = applyOutputStreamContext(ifmt_ctx, outFile, AVMEDIA_TYPE_AUDIO);
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
        LOGD("cutAudio success");
        end:
        if (ret < 0) {
            remove(outFile);
        }
        avformat_close_input(&ifmt_ctx);
        avio_closep(&ofmt_ctx->pb);
        avformat_free_context(ofmt_ctx);
        return ret;
    }

}