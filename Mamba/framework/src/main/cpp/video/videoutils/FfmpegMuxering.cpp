//
// Created by bearshi on 2017/6/13.
//

#include "FfmpegMuxering.h"
#include "log.h"

namespace video {
#define USE_H264BSF 0
#define USE_AACBSF 1

    FfmpegMuxering::FfmpegMuxering()
    {
        ifmt_ctx_h264 = NULL;
        ifmt_ctx_aac = NULL;
        ofmt_ctx = NULL;
    }

    FfmpegMuxering::~FfmpegMuxering()
    {
        avformat_close_input(&ifmt_ctx_h264);
        avformat_close_input(&ifmt_ctx_aac);

        if (ofmt_ctx && !(ofmt_ctx->flags & AVFMT_NOFILE))
            avio_close(ofmt_ctx->pb);
        avformat_free_context(ofmt_ctx);
    }

    int FfmpegMuxering::OpenInputH264File(const char *intputfilename)
    {
        int ret;
        unsigned int i;
        ifmt_ctx_h264 = NULL;
        if ((ret = avformat_open_input(&ifmt_ctx_h264, intputfilename, NULL, NULL)) < 0) {
            LOGI("Cannot open input file in OpenInputH264File\n");
            return ret;
        }
        if ((ret = avformat_find_stream_info(ifmt_ctx_h264, NULL)) < 0) {
            LOGI("Cannot find stream information\n");
            return ret;
        }
        for (i = 0; i < ifmt_ctx_h264->nb_streams; i++) {
            AVStream *stream;
            AVCodecContext *codec_ctx;
            stream = ifmt_ctx_h264->streams[i];
            codec_ctx = stream->codec;

            if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
                _index_in_h264 = i;

                ret = avcodec_open2(codec_ctx,
                                    avcodec_find_decoder(codec_ctx->codec_id), NULL);
                if (ret < 0) {
                    LOGI("Failed to open decoder for stream #%u\n", i);
                    return ret;
                }
            }
        }
        av_dump_format(ifmt_ctx_h264, 0, intputfilename, 0);

        return 0;
    }

    int FfmpegMuxering::OpenInputAacFile(const char *intputfilename)
    {
        int ret;
        unsigned int i;
        ifmt_ctx_aac = NULL;
        if ((ret = avformat_open_input(&ifmt_ctx_aac, intputfilename, NULL, NULL)) < 0) {
            LOGI("Cannot open input file in OpenInputAacFile\n");
            return ret;
        }
        if ((ret = avformat_find_stream_info(ifmt_ctx_aac, NULL)) < 0) {
            LOGI("Cannot find stream information\n");
            return ret;
        }
        for (i = 0; i < ifmt_ctx_aac->nb_streams; i++) {
            AVStream *stream;
            AVCodecContext *codec_ctx;
            stream = ifmt_ctx_aac->streams[i];
            codec_ctx = stream->codec;

            if (codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
                _index_in_aac = i;

                ret = avcodec_open2(codec_ctx,
                                    avcodec_find_decoder(codec_ctx->codec_id), NULL);
                if (ret < 0) {
                    LOGI("Failed to open decoder for stream #%u\n", i);
                    return ret;
                }
            }
        }
        av_dump_format(ifmt_ctx_aac, 0, intputfilename, 0);

        return 0;
    }

    int FfmpegMuxering::OpenOutputAacFile(const char *filename)
    {
        int i;
        LOGI("FfmpegMuxering::OpenOutputAacFile");
        ofmt_ctx = NULL;
        avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, filename);

        if (!ofmt_ctx) {
            av_log(NULL, AV_LOG_ERROR, "Could not create output context\n");
            return AVERROR_UNKNOWN;
        }

        for (i = 0; i < ifmt_ctx_h264->nb_streams; i++) {
            if(ifmt_ctx_h264->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
                AVStream *in_stream = ifmt_ctx_h264->streams[i];
                AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
                if (!out_stream) {
                    av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
                    return -1;
                }

                if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Failed to copy context from input to output stream codec context\n");
                    return -1;
                }

                out_stream->codec->codec_tag = 0;
                if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                    out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
                _index_out_h264 = out_stream->index;
                _index_in_h264 = i;
                break;
            }
        }

        for (i = 0; i < ifmt_ctx_aac->nb_streams; i++) {
            if(ifmt_ctx_aac->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
                AVStream *in_stream = ifmt_ctx_aac->streams[i];
                AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);

                if (!out_stream) {
                    av_log(NULL, AV_LOG_ERROR, "Failed allocating output stream\n");
                    return -1;
                }
                if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Failed to copy context from input to output stream codec context\n");
                    return -1;
                }
                out_stream->codec->codec_tag = 0;
                if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
                    out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
                _index_in_aac = i;
                _index_out_aac = out_stream->index;
                break;
            }
        }

        av_dump_format(ofmt_ctx, 0, filename, 1);
        if (!(ofmt_ctx->flags & AVFMT_NOFILE)) {
            if (avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE) < 0) {
                av_log(NULL, AV_LOG_ERROR, "Could not open output file '%s'", filename);
                return -1;
            }
        }

        if (avformat_write_header(ofmt_ctx, NULL) < 0) {
            av_log(NULL, AV_LOG_ERROR, "Error occurred when opening output file");
            return -1;
        }

        return 0;
    }

    int FfmpegMuxering::MuxerH264Aac(const char *h264filename, const char *aacfilename, const char *outputfilename)
    {
        int ret = -1;
        LOGI("MuxerH264Aac h264filename = %s", h264filename);
        LOGI("MuxerH264Aac aacfilename = %s", aacfilename);
        LOGI("MuxerH264Aac outputfilename = %s", outputfilename);

#if USE_H264BSF
        AVBitStreamFilterContext* h264bsfc =  av_bitstream_filter_init("h264_mp4toannexb");
#endif

        int64_t cur_pts_v = 0, cur_pts_a = 0;
        AVFormatContext *ifmt_ctx;
        int stream_index;
        AVPacket pkt;
        AVStream *in_stream, *out_stream;
        int frame_index = 0;
        AVBitStreamFilterContext* aacbsfc;

        av_register_all();
        ret = OpenInputH264File(h264filename);
        if (ret < 0) goto clearUp;
        ret = OpenInputAacFile(aacfilename);
        if (ret < 0) goto clearUp;
        ret = OpenOutputAacFile(outputfilename);
        if (ret < 0) goto clearUp;

        if (ifmt_ctx_aac->streams[_index_in_aac]->codec->codec_id == AV_CODEC_ID_AAC) {
            #if USE_AACBSF
                LOGI("MuxerH264Aac codec_id = AV_CODEC_ID_AAC");
                aacbsfc =  av_bitstream_filter_init("aac_adtstoasc");
            #endif
        }

        while (true) {
            if(av_compare_ts(cur_pts_v, ifmt_ctx_h264->streams[_index_in_h264]->time_base,
                             cur_pts_a, ifmt_ctx_aac->streams[_index_in_aac]->time_base) <= 0){
                ifmt_ctx = ifmt_ctx_h264;
                stream_index = _index_out_h264;

                if(av_read_frame(ifmt_ctx, &pkt) >= 0){
                    do{
                        in_stream  = ifmt_ctx->streams[pkt.stream_index];
                        out_stream = ofmt_ctx->streams[stream_index];
                        if(pkt.stream_index == _index_in_h264){
                            if(pkt.pts == AV_NOPTS_VALUE){
                                AVRational time_base1 = in_stream->time_base;
                                int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
                                pkt.pts = (double)(frame_index*calc_duration)/(double)(av_q2d(time_base1) * AV_TIME_BASE);
                                pkt.dts = pkt.pts;
                                pkt.duration = (double)calc_duration/(double)(av_q2d(time_base1) * AV_TIME_BASE);
                                frame_index++;
                            }
                            cur_pts_v = pkt.pts;
                            break;
                        }
                    }while(av_read_frame(ifmt_ctx, &pkt) >= 0);
                } else {
                    break;
                }
            } else {
                ifmt_ctx = ifmt_ctx_aac;
                stream_index = _index_out_aac;
                if(av_read_frame(ifmt_ctx, &pkt) >= 0){
                    do{
                        in_stream = ifmt_ctx->streams[pkt.stream_index];
                        out_stream = ofmt_ctx->streams[stream_index];
                        if(pkt.stream_index == _index_in_aac){
                            if(pkt.pts == AV_NOPTS_VALUE){
                                AVRational time_base1 = in_stream->time_base;
                                int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(in_stream->r_frame_rate);
                                pkt.pts = (double)(frame_index*calc_duration) / (double)(av_q2d(time_base1) * AV_TIME_BASE);
                                pkt.dts = pkt.pts;
                                pkt.duration = (double)calc_duration / (double)(av_q2d(time_base1) * AV_TIME_BASE);
                                frame_index++;
                            }
                            cur_pts_a = pkt.pts;
                            break;
                        }
                    }while(av_read_frame(ifmt_ctx, &pkt) >= 0);
                }else{
                    break;
                }
            }

#if USE_H264BSF
            av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif

            if (ifmt_ctx_aac->streams[_index_in_aac]->codec->codec_id == AV_CODEC_ID_AAC) {
                #if USE_AACBSF
                     av_bitstream_filter_filter(aacbsfc, out_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
                #endif
            }

            pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);

            //pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            //pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
            //pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);

            pkt.pos = -1;
            pkt.stream_index = stream_index;

            //printf("Write 1 Packet. size:%5d\tpts:%lld stream_index= %d\n", pkt.size, pkt.pts, stream_index);

            if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
                av_log(NULL, AV_LOG_ERROR, "Error muxing packet");
                ret = -1;
                break;
            }
            av_free_packet(&pkt);
        }

        av_write_trailer(ofmt_ctx);
#if USE_H264BSF
        av_bitstream_filter_close(h264bsfc);
#endif
        if (ifmt_ctx_aac->streams[_index_in_aac]->codec->codec_id == AV_CODEC_ID_AAC) {
            #if USE_AACBSF
                av_bitstream_filter_close(aacbsfc);
            #endif
        }

        clearUp:
        LOGI("MuxerH264Aac ret = %d", ret);
        return ret;
    }
}