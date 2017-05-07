
//
// Created by walljiang on 2017/04/16.
//
//#include <libavutil/pixdesc.h>
//#include <libavutil/imgutils.h>
//#include "libavformat/avformat.h"
//#include "Log.h"
//
//static const char* src_filename;
//static int refcount=0;
//static int width, height;
//static enum AVPixelFormat pix_fmt;
//static AVFormatContext *ifmt_ctx;
//static AVFormatContext *ofmt_ctx;
//static AVCodecContext *video_dec_ctx;
//static int video_stream_idx = -1, audio_stream_idx = -1;
//static AVFrame *frame = NULL;
//static AVPacket pkt;
//
//int openInputFile(const char* in_filename,AVFormatContext **fmt_ctx){
//    ifmt_ctx = NULL;
//    int ret = 0;
//    if ((ret = avformat_open_input(fmt_ctx, in_filename, 0, 0)) < 0) {
//        LOGE("Could not open input file '%s'", in_filename);
//        return ret;
//    }
//
//    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
//        LOGE("Failed to retrieve input stream information");
//        return ret;
//    }
//   return 0;
//}
//
//int openOutputFile(const char* out_filename,AVFormatContext **fmt_ctx,AVCodecContext **decodeCtx){
//    *fmt_ctx = NULL;
//    int ret = 0,i=0;
//    avformat_alloc_output_context2(fmt_ctx,NULL,NULL,out_filename);
//    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
//        AVStream *in_stream = ifmt_ctx->streams[i];
//        AVCodec *in_codec = avcodec_find_decoder(in_stream->codecpar->codec_id);
//        if (in_codec == NULL) {
//            LOGD("input codec is null");
//            ret = -1;
//            return ret;
//        }
//        AVStream *out_stream = avformat_new_stream(*fmt_ctx, in_codec);
//        if (!out_stream) {
//            LOGE("Failed allocating output stream\n");
//            ret = -1;
//            return ret;
//        }
//        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
//        avcodec_parameters_to_context(decodeCtx,)
//        if (ret < 0) {
//            LOGE("Failed to copy context from input to output stream codec context\n");
//            ret = -1;;
//            return ret;
//        }
//        out_stream->codecpar->codec_tag = 0;
//    }
//    ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
//    if (ret < 0) {
//        LOGE("Could not open output file '%s'", out_filename);
//        return ret;
//    }
//
//    ret = avformat_write_header(ofmt_ctx, NULL);
//    if (ret < 0) {
//        LOGE("Error occurred when opening output file\n");
//        ret = -1;
//        return ret;
//    }
//}
//
//int decodec_frame(AVCodecContext *pCodecCtx, AVPacket pkt,AVFrame **frame) {
//    if (!frame) {
//        return NULL;
//    }
//    int result = avcodec_send_packet(pCodecCtx, &pkt);
//    if (result < 0 && result != AVERROR(EAGAIN) && result != AVERROR_EOF) {
//        LOGI("reverse2.decodec_frame()向解码器发送数据失败");
//        av_frame_free(frame);
//        return NULL;
//    }
//    //从解码器返回解码输出数据
//    result = avcodec_receive_frame(pCodecCtx, *frame);
//    if (result < 0 && result != AVERROR_EOF) {
//        LOGI("reverse2.decodec_frame()解码数据失败");
//        av_frame_free(frame);
//        return NULL;
//    }
//    return result;
//}
//
//static int open_input_file1(const char *filename)
//{
//
//    _index_first = av_find_best_stream(ifmt_first_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
//    if (ret < 0) {avformat_new_stream()
//        LOGD("Cannot find a video stream in the input file\n");
//        return -1;
//    }
//    AVCodecParameters *pAVCodecParameters = (ifmt_first_ctx)->streams[_index_first]->codecpar;
//    pDecodeCtx1 = avcodec_alloc_context3(codec);
//    if ((ret = avcodec_parameters_to_context(pDecodeCtx1, pAVCodecParameters)) < 0) {
//        LOGI("open_decodec_context()无法根据pCodec分配AVCodecContext");
//        return ret;
//    }
//    //打开解码器
//    if ( (ret = avcodec_open2(pDecodeCtx1, codec, NULL)) < 0) {
//        LOGI("open_decodec_context()无法打开编码器");
//        return ret;
//    }
//    av_dump_format(ifmt_first_ctx, 0, filename, 0);
//    return 0;
//}