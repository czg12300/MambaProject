//
// Created by jakechen on 2017/1/12.
//


#include "Demuxer.h"


int demuxing(const char *filePath, const char *h264, const char *aac) {
    if ((access(filePath, F_OK)) == -1 ) {
        return -1;
    }
    int ret = -1;
    AVPacket pkt;
    OutputStreamContext *v_out_ctx = NULL;
    OutputStreamContext *a_out_ctx = NULL;
    AVStream *out_stream = NULL;
    AVFormatContext *ofmt_ctx = NULL;
    AVStream *in_stream = NULL;
    av_register_all();
    AVFormatContext *ifmt_ctx = NULL; //这个结构体描述了一个媒体文件或媒体流的构成和基本信息
    if (avformat_open_input(&ifmt_ctx, filePath, NULL, NULL) < 0) {
        LOGD("avformat_open_input  error!!!\n");
        ret = -1;
        goto end;
    }
    //查找输入文件信息
    if (avformat_find_stream_info(ifmt_ctx, NULL) < 0) {
        LOGD("avformat_find_stream_info error !!!\n");
        ret = -1;
        goto end;
    }
    //生成视频流上下文
    v_out_ctx = applyOutputStreamContext(ifmt_ctx, h264, AVMEDIA_TYPE_VIDEO);
    if (v_out_ctx == NULL) {
        LOGD("apply video OutputStreamContext fail");
        ret = -1;
        goto end;
    }
    //生成音频流上下文
    a_out_ctx = applyOutputStreamContext(ifmt_ctx, aac, AVMEDIA_TYPE_AUDIO);
    if (a_out_ctx == NULL) {
        LOGD("apply audio OutputStreamContext fail");
        ret = -1;
        goto end;
    }
    //读取帧数据并写入输出文件
    while (av_read_frame(ifmt_ctx, &pkt) >= 0) {
        in_stream = ifmt_ctx->streams[pkt.stream_index];
        if (pkt.stream_index == v_out_ctx->streamIndex) {
            out_stream = v_out_ctx->fmt_ctx->streams[0];
            ofmt_ctx = v_out_ctx->fmt_ctx;
        } else if (pkt.stream_index == a_out_ctx->streamIndex) {
            out_stream = a_out_ctx->fmt_ctx->streams[0];
            ofmt_ctx = a_out_ctx->fmt_ctx;
        } else {
            continue;
        }
        /* copy packet */
        //转换PTS/DTS（Convert PTS/DTS）
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        pkt.stream_index = 0;
        //写入（Write）
        if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
            //LOGE("Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);
    }
    //写文件尾（Write file trailer）
    av_write_trailer(v_out_ctx->fmt_ctx);
    av_write_trailer(a_out_ctx->fmt_ctx);
    ret = 1;
    //销毁资源
    end:
    avio_close(v_out_ctx->fmt_ctx->pb);
    avformat_free_context(v_out_ctx->fmt_ctx);
    avio_close(a_out_ctx->fmt_ctx->pb);
    avformat_free_context(a_out_ctx->fmt_ctx);
    avformat_close_input(&ifmt_ctx);
    avformat_free_context(ifmt_ctx);
    return ret;
}

int demuxingVideo(const char *filePath, const char *h264) {
    if ((access(filePath, F_OK)) == -1 ) {
        return -1;
    }
    int ret = -1;
    AVPacket pkt;
    OutputStreamContext *v_out_ctx = NULL;
    AVStream *out_stream = NULL;
    AVFormatContext *ofmt_ctx = NULL;
    AVStream *in_stream = NULL;
    av_register_all();
    AVFormatContext *ifmt_ctx = NULL; //这个结构体描述了一个媒体文件或媒体流的构成和基本信息
    if (avformat_open_input(&ifmt_ctx, filePath, NULL, NULL) < 0) {
        LOGD("avformat_open_input  error!!!\n");
        ret = -1;
        goto end;
    }
    //查找输入文件信息
    if (avformat_find_stream_info(ifmt_ctx, NULL) < 0) {
        LOGD("avformat_find_stream_info error !!!\n");
        ret = -1;
        goto end;
    }
    //生成视频流上下文
    v_out_ctx = applyOutputStreamContext(ifmt_ctx, h264, AVMEDIA_TYPE_VIDEO);
    if (v_out_ctx == NULL) {
        LOGD("apply video OutputStreamContext fail");
        ret = -1;
        goto end;
    }
    //读取帧数据并写入输出文件
    while (av_read_frame(ifmt_ctx, &pkt) >= 0) {
        in_stream = ifmt_ctx->streams[pkt.stream_index];
        if (pkt.stream_index == v_out_ctx->streamIndex) {
            out_stream = v_out_ctx->fmt_ctx->streams[0];
            ofmt_ctx = v_out_ctx->fmt_ctx;
        } else {
            continue;
        }
        /* copy packet */
        //转换PTS/DTS（Convert PTS/DTS）
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        pkt.stream_index = 0;
        //写入（Write）
        if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
            //LOGE("Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);
    }
    //写文件尾（Write file trailer）
    av_write_trailer(v_out_ctx->fmt_ctx);
    ret = 1;
    //销毁资源
    end:
    if (ret<0){
        remove(h264);
    }
    avio_close(v_out_ctx->fmt_ctx->pb);
    avformat_free_context(v_out_ctx->fmt_ctx);
    avformat_close_input(&ifmt_ctx);
    avformat_free_context(ifmt_ctx);
    return ret;
}

int demuxingAudio(const char *filePath, const char *aac) {
    if ((access(filePath, F_OK)) == -1 ) {
        return -1;
    }
    int ret = -1;
    AVPacket pkt;
    OutputStreamContext *a_out_ctx = NULL;
    AVStream *out_stream = NULL;
    AVFormatContext *ofmt_ctx = NULL;
    AVStream *in_stream = NULL;
    av_register_all();
    AVFormatContext *ifmt_ctx = NULL; //这个结构体描述了一个媒体文件或媒体流的构成和基本信息
    if (avformat_open_input(&ifmt_ctx, filePath, NULL, NULL) < 0) {
        LOGD("avformat_open_input  error!!!\n");
        ret = -1;
        goto end;
    }
    //查找输入文件信息
    if (avformat_find_stream_info(ifmt_ctx, NULL) < 0) {
        LOGD("avformat_find_stream_info error !!!\n");
        ret = -1;
        goto end;
    }
    //生成音频流上下文
    a_out_ctx = applyOutputStreamContext(ifmt_ctx, aac, AVMEDIA_TYPE_AUDIO);
    if (a_out_ctx == NULL) {
        LOGD("apply audio OutputStreamContext fail");
        ret = -1;
        goto end;
    }
    //读取帧数据并写入输出文件
    while (av_read_frame(ifmt_ctx, &pkt) >= 0) {
        in_stream = ifmt_ctx->streams[pkt.stream_index];
        if (pkt.stream_index == a_out_ctx->streamIndex) {
            out_stream = a_out_ctx->fmt_ctx->streams[0];
            ofmt_ctx = a_out_ctx->fmt_ctx;
        } else {
            continue;
        }
        /* copy packet */
        //转换PTS/DTS（Convert PTS/DTS）
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
                                   (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        pkt.stream_index = 0;
        //写入（Write）
        if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
            //LOGE("Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);
    }
    //写文件尾（Write file trailer）
    av_write_trailer(a_out_ctx->fmt_ctx);
    ret = 1;
    //销毁资源
    end:
    if (ret<0){
        remove(aac);
    }
    avio_close(a_out_ctx->fmt_ctx->pb);
    avformat_free_context(a_out_ctx->fmt_ctx);
    avformat_close_input(&ifmt_ctx);
    avformat_free_context(ifmt_ctx);
    return ret;
}

