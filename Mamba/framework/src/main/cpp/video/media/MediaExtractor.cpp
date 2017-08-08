//
// Created by jakechen on 2017/7/20.
//

#include "MediaExtractor.h"

namespace video {
    static void getLastFrameTime(const char *file, double *time);

    MediaExtractor::MediaExtractor() {
        av_register_all();
        av_init_packet(&packet);
    }

    int MediaExtractor::_setDataSource(const char *file) {
        double dTime = 0;
        getLastFrameTime(file, &dTime);
        duration = dTime * 1000;
        fmt_ctx = NULL;
        if (avformat_open_input(&fmt_ctx, file, NULL, NULL) < 0) {
            LOGD("avformat_open_input  error!!!\n");
            return -1;
        }
        //查找输入文件信息
        if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
            LOGD("avformat_find_stream_info error !!!\n");
            return -1;
        }
        return 1;
    }

    int MediaExtractor::_getDuration() {
        return duration;
    }

    int MediaExtractor::_getTrackCount() {
        return fmt_ctx != NULL ? fmt_ctx->nb_streams : 0;
    }

    MediaFormat *MediaExtractor::_getTrackFormat(int index) {
        AVStream *stream = fmt_ctx->streams[index];
        MediaFormat *format = new MediaFormat();
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            format->codec_type = format->CODE_TYPE_VIDEO;
            format->width = stream->codecpar->width;
            format->height = stream->codecpar->height;
            format->bit_rate = stream->codecpar->bit_rate;
            int frame_rate = (int) (((float) stream->avg_frame_rate.num /
                                     (float) stream->avg_frame_rate.den) + 0.5);
            format->frame_rate = frame_rate;
        } else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            format->codec_type = format->CODE_TYPE_AUDIO;
            format->sample_rate = stream->codecpar->sample_rate;
            format->channels = stream->codecpar->channels;
            format->channel_layout = stream->codecpar->channel_layout;
        }
        format->codec_id = format->transCodeId(stream->codecpar->codec_id);
        return format;
    }

    uint8_t *MediaExtractor::_readSampleData(int *size) {
        av_init_packet(&packet);
        packet.data = NULL;
        packet.size = 0;
        if (trackIndex != AVERROR_STREAM_NOT_FOUND) {
            while (av_read_frame(fmt_ctx, &packet) >= 0) {
                if (packet.stream_index == trackIndex) {
                    *size = packet.size;
//                    uint8_t *temp = new uint8_t[packet.size];
//                    memcpy(temp, packet.data, packet.size);
                    return packet.data;
                } else {
                    av_init_packet(&packet);
                    packet.data = NULL;
                    packet.size = 0;
                }
            }
        } else {
            if (av_read_frame(fmt_ctx, &packet) >= 0) {
                *size = packet.size;
//                uint8_t *temp = new uint8_t[packet.size];
//                memcpy(temp, packet.data, packet.size);
                return packet.data;
            }
        }
        return NULL;
    }

    int MediaExtractor::_getTrackIndex() {
        return packet.stream_index;
    }

    int MediaExtractor::_getTimestamp() {
        return av_q2d(fmt_ctx->streams[_getTrackIndex()]->time_base) * packet.pts * 1000000;
    }

    int MediaExtractor::_getSampleDataSize() {
        return packet.size;
    }

    void MediaExtractor::_seekTo(int timeUs, int mode) {
        av_seek_frame(fmt_ctx, -1, (timeUs / 1000) * AV_TIME_BASE, mode);
    }

    void MediaExtractor::_selectTrack(int index) {
        trackIndex = index;
    }

    void MediaExtractor::_release() {
        if (fmt_ctx != NULL) {
            avformat_close_input(&fmt_ctx);
            avformat_free_context(fmt_ctx);
        }
        av_packet_unref(&packet);
    }


    static void getLastFrameTime(const char *file, double *time) {
        AVFormatContext *fmt_ctx = NULL;
        int ret = -1;
        if ((ret = avformat_open_input(&fmt_ctx, file, 0, 0)) < 0) {
            LOGE("Could not open input file '%s'", file);
            return;
        }

        if ((ret = avformat_find_stream_info(fmt_ctx, 0)) < 0) {
            LOGE("Failed to retrieve input stream information");
            return;
        }
        int video_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0); 
        if (video_index == -1) {
            LOGD("can't find video stream in %s\n", fmt_ctx->filename); 
            return;
        }
        while (1) {
            AVPacket pkt;
              ret = av_read_frame(fmt_ctx, &pkt); 
            if (ret < 0) {
                break; 
            } 
            if (pkt.stream_index == video_index) {
                *time = av_q2d(fmt_ctx->streams[video_index]->time_base) * pkt.pts * 1000;
            } 
            av_packet_unref(&pkt);
        } 
        avio_closep(&fmt_ctx->pb);
    }
}