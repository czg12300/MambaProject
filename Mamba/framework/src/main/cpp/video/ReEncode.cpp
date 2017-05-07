//
// Created by walljiang on 2017/03/08.
//
#include "ReEncode.h"
#include "Log.h"
#include <vector>

extern "C" {
#include "libavformat/avformat.h"
#include <libavutil/imgutils.h>
#include "libavcodec/avcodec.h"
}
static AVFormatContext *ifmt_ctx;
static AVFormatContext *ofmt_ctx;
static int frame_count;
static int videoStreamIdx, audioStreamIdx;
static std::vector<int64_t> keyDts;

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
        LOGI("ofmt_ctx->oformat->flags：%d\n",ofmt_ctx->oformat->flags);
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

static int getKeyDts() {
    int ret = 0;
    AVPacket packet;
    av_init_packet(&packet);
    while (1) {
        if (av_read_frame(ifmt_ctx, &packet) < 0) {
            LOGE("av_read_frame < 0 \n");
            break;
        }
        if (packet.flags == AV_PKT_FLAG_KEY && packet.stream_index == videoStreamIdx) {
            keyDts.insert(keyDts.begin() + keyDts.size(), packet.dts);
            if ((ret = av_seek_frame(ifmt_ctx, videoStreamIdx, packet.dts + 1, AVSEEK_FLAG_FRAME)) <
                0) {
                LOGE("av_seek_frame failed\n");
                break;
            }
        }
    }

    if (keyDts.size() > 0) {
        LOGI("firstdts:%lld,size:%d", keyDts[0], keyDts.size());
        av_seek_frame(ifmt_ctx, videoStreamIdx, keyDts[0], AVSEEK_FLAG_FRAME);
    }
    av_packet_unref(&packet);
    return ret;
}

int reEncode(const char *srcFile, const char *outFile) {
    LOGI("reencode:srcFile:%s,outFile:%s", srcFile, outFile);
    int ret, width, height;
    AVPacket packet, enc_pkt;
    AVFrame *frame;
    int got_frame, got_picture;
    AVCodecContext *pEncodeCtx;

    av_register_all();
    if (open_input_file(srcFile, &videoStreamIdx, &audioStreamIdx) < 0) {
        LOGE("Open input file error\n");
        goto end;
    }
    LOGI("audioStreamIdx:%d，videoStreamIdx:%d\n", audioStreamIdx, videoStreamIdx);
    if (open_output_video(outFile) < 0) {
        goto end;
    }
    pEncodeCtx = ofmt_ctx->streams[videoStreamIdx]->codec;
    av_init_packet(&enc_pkt);
    av_init_packet(&packet);
    frame = av_frame_alloc();
    getKeyDts();
//    av_seek_frame(ifmt_ctx,videoStreamIdx,keyDts[keyDts.size()-2],AVSEEK_FLAG_FRAME);
    while (1) {
        //AVSEEK_FLAG_ANY和AVSEEK_FLAG_FRAME都是向后seek，AVSEEK_FLAG_BACKWARD是向前seek
        if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0) {
            LOGE("av_read_frame failed:%d\n", ret);
            ret = 0;
            break;
        }
//        if(keyDts[1] == packet.dts){
//            break;
//        }
//        if(packet.stream_index == audioStreamIdx){
        ret = decode_packet(&packet, &frame, &got_frame);
//        }else if(packet.stream_index == videoStreamIdx){
//            //seek到最后一个关键帧处，解码改gop组，并把
//        }
        if (ret < 0) {
            LOGE("decode_packet < 0 :%d \n", ret);
            break;
        }
        if (got_frame) {
            int ret = avcodec_encode_video2(pEncodeCtx, &enc_pkt, frame, &got_picture);
            if (ret < 0) {
                LOGE("Failed to encode!\n");
                goto end;
            }
            if (!(got_picture)) {
                continue;
            }
            enc_pkt.stream_index = videoStreamIdx;
            enc_pkt.dts = av_rescale_q_rnd(packet.dts,
                                           ifmt_ctx->streams[videoStreamIdx]->time_base,
                                           ofmt_ctx->streams[videoStreamIdx]->time_base,
                                           (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            enc_pkt.pts = av_rescale_q_rnd(packet.pts,
                                           ifmt_ctx->streams[videoStreamIdx]->time_base,
                                           ofmt_ctx->streams[videoStreamIdx]->time_base,
                                           (AVRounding) (AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            enc_pkt.duration = av_rescale_q(packet.duration,
                                            ifmt_ctx->streams[videoStreamIdx]->time_base,
                                            ofmt_ctx->streams[videoStreamIdx]->time_base);
            LOGI("WRITE PTS22:%lld,%lld",enc_pkt.pts,packet.pts);
            ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
            if (ret < 0) {
                break;
            }
        }
    }
    av_write_trailer(ofmt_ctx);
end:
    if (ifmt_ctx != NULL) {
        avcodec_close(ifmt_ctx->streams[videoStreamIdx]->codec);
        avcodec_close(ifmt_ctx->streams[audioStreamIdx]->codec);
        avformat_free_context(ifmt_ctx);
        ifmt_ctx == NULL;
    }
    if (ofmt_ctx != NULL) {
        avcodec_close(ofmt_ctx->streams[videoStreamIdx]->codec);
        avcodec_close(ofmt_ctx->streams[audioStreamIdx]->codec);
        avformat_free_context(ofmt_ctx);
        ofmt_ctx == NULL;
    }
    if (packet.size > 0) {
        packet.size = 0;
        av_packet_unref(&packet);
    }
    if (enc_pkt.size > 0) {
        enc_pkt.size = 0;
        av_packet_unref(&enc_pkt);
    }
    if (frame != NULL) {
        av_frame_free(&frame);
        frame = NULL;
    }
    return ret;
}