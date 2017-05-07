//
// Created by jakechen on 2017/3/9.
//

#include "VideoRecodeFactory.h"

static
int demuxingAudio(string filePath, string aac);

VideoRecodeFactory::~VideoRecodeFactory() {
    remove(this->outAac.c_str());
    remove(this->outOptH264.c_str());
    remove(this->outOptAac.c_str());
    remove(this->tempDir.c_str());
}

VideoRecodeFactory::VideoRecodeFactory(string src_File, string out_File) {
    this->srcFile = src_File;
    this->outFile = out_File;
    this->tempDir = out_File + "temp";
    if (opendir(tempDir.c_str()) == NULL) {
        mkdir(tempDir.c_str(), 0);
    }
    this->outAac = this->tempDir + "/out.aac";
    this->outOptAac = this->tempDir + "/outOpt.aac";
    this->outOptH264 = this->tempDir + "/outOpt.h264";
}

int VideoRecodeFactory::demuxerAudio() {
    return demuxingAudio(this->srcFile, this->outAac);
}

int VideoRecodeFactory::muxer(string h264, string aac, string file) {
    return muxing(h264.c_str(), aac.c_str(), file.c_str());
}

static int getFrameCount(const char *file) {
    AVFormatContext *ifmt_ctx = NULL;
    AVPacket pkt;
    int ret = -1;
    int frameCount = 0;
    if ((ret = avformat_open_input(&ifmt_ctx, file, 0, 0)) < 0) {
        LOGE("Could not open input file '%s'", file);
        return 0;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        LOGE("Failed to retrieve input stream information");
        return 0;
    }
    int video_index = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0); 
    if (video_index == -1) {
        LOGD("can't find video stream in %s\n", ifmt_ctx->filename); 
        return 0;
    }
    while (1) {
          ret = av_read_frame(ifmt_ctx, &pkt); 
        if (ret < 0) {
            break; 
        } 
        if (pkt.stream_index == video_index) {
            frameCount++;
        } 
        av_packet_unref(&pkt);
    } 
    avio_closep(&ifmt_ctx->pb);
    return frameCount;
}

int VideoRecodeFactory::optVideo(VideoOptHandler *video_handler) {
    int ret = -1;
    AVCodecContext *decodecCtx = NULL;
    AVFrame *frame;
    AVStream *in_stream;
    int frameRate = 0;
    int frameCount = getFrameCount(this->srcFile.c_str());
    int frameIndex = 0;
    int tag;
    this->handler = video_handler;
    av_register_all();
    avfilter_register_all();
    if ((access(this->srcFile.c_str(), F_OK)) == -1) {
        return -1;
    }
    if ((ret = avformat_open_input(&ifmt_ctx, this->srcFile.c_str(), 0, 0)) < 0) {
        LOGE("Could not open input file '%s'", srcFile.c_str());
        goto end;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        LOGE("Failed to retrieve input stream information");
        goto end;
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
    if (ret = handler->init(ifmt_ctx, decodecCtx, video_index, frameCount) < 0) {
        LOGE("init_filters fail");
        goto end;
    }
    v_out_ctx = applyOutputStreamContext(ifmt_ctx, this->outOptH264.c_str(), AVMEDIA_TYPE_VIDEO);
    frameRate = (int) (((float) ifmt_ctx->streams[video_index]->avg_frame_rate.num /
                        (float) ifmt_ctx->streams[video_index]->avg_frame_rate.den) + 0.5);
    LOGD("frameRate=%d", frameRate);
    frameRate = handler->changeFrameRate(frameRate);
    ecodecCtx = applay_encoder_form_decoder(ifmt_ctx->streams[video_index], frameRate);
    //给解码帧申请内存
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

        in_stream = ifmt_ctx->streams[pkt.stream_index];
        if (pkt.stream_index == video_index) {
//的作用是解码一帧视频数据。输入一个压缩编码的结构体AVPacket，输出一个解码后的结构体AVFrame。该函数的声明位于libavcodec\avcodec.h
            ret = avcodec_send_packet(decodecCtx, &pkt);
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                LOGE("avcodec_send_packet fail");
                av_packet_unref(&pkt);
                continue;
            }
            //从解码器返回解码输出数据
            ret = avcodec_receive_frame(decodecCtx, frame);
            if (ret < 0 && ret != AVERROR_EOF) {
                LOGE("avcodec_receive_frame fail");
                av_packet_unref(&pkt);
                continue;
            }
            int lastIndex = frameIndex;
            AllowEncodeAndWriteFrameState writeFrameState = handler->allowEncodeAndWriteFrame(
                    in_stream, pkt, frameIndex++);
            if (writeFrameState == STATE_NOT_ALLOW) {
                av_packet_unref(&pkt);
                continue;
            } else if (writeFrameState == STATE_FINISH_WRITE) {
                av_packet_unref(&pkt);
                break;
            } else if (writeFrameState == STATE_ALLOW) {
                AVFrame *handledFrame = handler->optAvFrame(frame, lastIndex);
                if (!handledFrame) {
                    continue;
                }
                encode_write_frame(handledFrame);
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
        if (av_interleaved_write_frame(v_out_ctx->fmt_ctx, &pkt) < 0) {
            av_packet_unref(&pkt);
            LOGE("Error muxing packet\n");
            break;
        }
        av_packet_unref(&avPacket);
    }
    //写文件尾（Write file trailer）
    av_write_trailer(v_out_ctx->fmt_ctx);
    ret = 1;
    end:
    if (ret < 0) {
        remove(this->outOptH264.c_str());
    }
    handler->releaseInEnd();
    avio_close(v_out_ctx->fmt_ctx->pb);
    avformat_free_context(v_out_ctx->fmt_ctx);
    avcodec_close(decodecCtx);
    avcodec_close(ecodecCtx);
    av_frame_free(&frame);
    avio_closep(&ifmt_ctx->pb);
    avformat_close_input(&ifmt_ctx);
    avformat_free_context(ifmt_ctx);
    return ret;
}

int VideoRecodeFactory::encode_write_frame(AVFrame *frame) {
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL; // packet data will be allocated by the encoder
    pkt.size = 0;
    if (encodec_video_frame(ecodecCtx, frame, &pkt) < 0) {
        av_packet_unref(&pkt);
        LOGE("Error muxing packet\n");
        return -1;
    }
    handler->changeTimebase(ifmt_ctx->streams[video_index], v_out_ctx->stream, pkt);
    if (av_interleaved_write_frame(v_out_ctx->fmt_ctx, &pkt) < 0) {
        av_packet_unref(&pkt);
        LOGE("Error muxing packet\n");
        return -1;
    }
    return 1;
}

static
int demuxingAudio(string filePath, string aac) {
    int ret = -1;
    AVPacket pkt;
    OutputStreamContext *a_out_ctx = NULL;
    AVStream *out_stream = NULL;
    AVFormatContext *ofmt_ctx = NULL;
    AVStream *in_stream = NULL;
    av_register_all();
    if ((access(filePath.c_str(), F_OK)) == -1) {
        return -1;
    }
    AVFormatContext *ifmt_ctx = NULL; //这个结构体描述了一个媒体文件或媒体流的构成和基本信息
    if (avformat_open_input(&ifmt_ctx, filePath.c_str(), NULL, NULL) < 0) {
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
    a_out_ctx = applyOutputStreamContext(ifmt_ctx, aac.c_str(), AVMEDIA_TYPE_AUDIO);
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
    if (ret < 0) {
        remove(aac.c_str());
    }
    avio_close(a_out_ctx->fmt_ctx->pb);
    avformat_free_context(a_out_ctx->fmt_ctx);
    avformat_close_input(&ifmt_ctx);
    avformat_free_context(ifmt_ctx);
    return ret;
}
//int VideoOptHandler::init(AVFormatContext *ifmt_ctx, AVCodecContext *decodecCtx, int video_index) {}
//
//void VideoOptHandler::releaseInEnd() {}
//
//AVFrame *VideoOptHandler::optAvFrame(AVFrame *frame) {}