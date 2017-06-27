//
// Created by jakechen on 2017/6/3.
//

#include "VideoCodeProduce.h"

namespace video {
    static void getLastFrameTime(const char *file, double *time);

    VideoCodeProduce::VideoCodeProduce(VideoCodeParams *videoCodeParams) {
        av_register_all();
        this->params = videoCodeParams;
        LOGD("params->srcFile=%s   params->outFile=%s   params->rangeStart=%d   params->rangeEnd=%d   ",
             params->srcFile.c_str(), params->outFile.c_str(), params->rangeStart,
             params->rangeEnd);

        isSuccess = false;
        isStop = false;
        getLastFrameTime(params->srcFile.c_str(), &lastFrameTime);
        LOGD("VideoCodeProduce lastFrameTime %lf", lastFrameTime);
        if (lastFrameTime > 0) {
            if (this->params->rangeStart < 0) {
                this->params->rangeStart = 0;
            }
            if (lastFrameTime < 1000) {//因为1s的时间内不能剪辑，所以做一个容错处理
                this->params->rangeEnd = lastFrameTime;
            } else {
                if (this->params->rangeEnd <= this->params->rangeStart) {
                    this->params->rangeEnd = lastFrameTime;
                }
                if (this->params->rangeEnd > lastFrameTime) {
                    this->params->rangeEnd = lastFrameTime;
                }
            }
            srcPacketQueue = new queue<AVPacket>;
            decodeFrameQueue = new queue<DecodeFrame *>;
            outPacketQueue = new queue<AVPacket>;
            init();
            dts_start_from = (int64_t *) malloc(sizeof(int64_t) * ifmt_ctx->nb_streams);
            memset(dts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);
            pts_start_from = (int64_t *) malloc(sizeof(int64_t) * ifmt_ctx->nb_streams);
            memset(pts_start_from, 0, sizeof(int64_t) * ifmt_ctx->nb_streams);
            isInitSuccess = true;
        } else {
            isInitSuccess = false;
        }


    }

    static void getLastFrameTime(const char *file, double *time) {
        AVFormatContext *ifmt_ctx = NULL;
        int ret = -1;
        if ((ret = avformat_open_input(&ifmt_ctx, file, 0, 0)) < 0) {
            LOGE("Could not open input file '%s'", file);
            return;
        }

        if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
            LOGE("Failed to retrieve input stream information");
            return;
        }
        int video_index = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0); 
        if (video_index == -1) {
            LOGD("can't find video stream in %s\n", ifmt_ctx->filename); 
            return;
        }
        while (1) {
            AVPacket pkt;
              ret = av_read_frame(ifmt_ctx, &pkt); 
            if (ret < 0) {
                break; 
            } 
            if (pkt.stream_index == video_index) {
                *time = av_q2d(ifmt_ctx->streams[video_index]->time_base) * pkt.pts * 1000;
            } 
            av_packet_unref(&pkt);
        } 
        avio_closep(&ifmt_ctx->pb);
//        avformat_close_input(&ifmt_ctx);
//        avformat_free_context(ifmt_ctx);
    }

    void VideoCodeProduce::start() {
        if (!isInitSuccess) {
            release();
        }
        mutex_src_package = new pthread_mutex_t;
        mutex_decode_frame = new pthread_mutex_t;
        mutex_out_package = new pthread_mutex_t;
        pthread_mutex_init(mutex_src_package, NULL);
        pthread_mutex_init(mutex_decode_frame, NULL);
        pthread_mutex_init(mutex_out_package, NULL);
        threadIds = new pthread_t[4];
        ThreadExecuteParams *executeParams = new ThreadExecuteParams();
        executeParams->methodId = METHOD_ID_READ;
        executeParams->produce = this;
        pthread_create(&threadIds[0], NULL, threatExecute, executeParams);

        executeParams = new ThreadExecuteParams();
        executeParams->methodId = METHOD_ID_DECODE;
        executeParams->produce = this;
        pthread_create(&threadIds[1], NULL, threatExecute, executeParams);

        executeParams = new ThreadExecuteParams();
        executeParams->methodId = METHOD_ID_ENCODE;
        executeParams->produce = this;
        pthread_create(&threadIds[2], NULL, threatExecute, executeParams);

        executeParams = new ThreadExecuteParams();
        executeParams->methodId = METHOD_ID_WRITE;
        executeParams->produce = this;
        pthread_create(&threadIds[3], NULL, threatExecute, executeParams);
    }

    void VideoCodeProduce::release() {
        LOGD("release  isSuccess=%d", isSuccess);
        isStop = true;
        if (threadIds != NULL) {
            for (int i = 0; i < 4; i++) {
                pthread_join(threadIds[i], NULL);
            }
            delete (threadIds);
        }

        if (mutex_src_package != NULL) {
            pthread_mutex_destroy(mutex_src_package);
            delete (mutex_src_package);
        }
        if (mutex_decode_frame != NULL) {
            pthread_mutex_destroy(mutex_decode_frame);
            delete (mutex_decode_frame);
        }
        if (mutex_out_package != NULL) {
            pthread_mutex_destroy(mutex_out_package);
            delete (mutex_out_package);
        }
        if (params->effect != NULL) {
            params->effect->release();
        }
        if (decode_ctx != NULL) {
            avcodec_close(decode_ctx);
        }
        if (encode_ctx != NULL) {
            avcodec_close(encode_ctx);
        }
        if (ifmt_ctx != NULL) {
            avformat_close_input(&ifmt_ctx);
            avformat_free_context(ifmt_ctx);
        }
        if (ofmt_ctx != NULL) {
            avio_close(ofmt_ctx->pb);
            avformat_free_context(ofmt_ctx);
        }
        if (srcPacketQueue != NULL) {
            delete (srcPacketQueue);
        }
        if (decodeFrameQueue != NULL) {
            delete (decodeFrameQueue);
        }
        if (outPacketQueue != NULL) {
            delete (outPacketQueue);
        }
        if (dts_start_from != NULL) {
            free(dts_start_from);
        }
        if (pts_start_from != NULL) {
            free(pts_start_from);
        }
        LOGD("release  1");
        if (params->effect != NULL) {
            if (isSuccess) {
                params->effect->onSuccess();
            } else {
                params->effect->onFail();
            }
        }
        LOGD("release  1");
    }

    VideoCodeProduce::~VideoCodeProduce() {
    }

    bool VideoCodeProduce::hasAudioStream() {
        return audio_index != -1 && audio_index != AVERROR_STREAM_NOT_FOUND;
    }

    void VideoCodeProduce::init() {
        ifmt_ctx = NULL;
        ofmt_ctx = NULL;
        int ret = -1;
        if ((ret = avformat_open_input(&ifmt_ctx, params->srcFile.c_str(), 0, 0)) < 0) {
            LOGE("Could not open input file '%s'", params->srcFile.c_str());
            release();
            return;
        }

        if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
            LOGE("Failed to retrieve input stream information");
            release();
            return;
        }
        video_index = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0); 
        audio_index = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0); 
        if (video_index == -1) {
            LOGD("can't find video stream in %s\n", ifmt_ctx->filename); 
            ret = -1; 
            release();
            return;
        }
        in_stream = ifmt_ctx->streams[video_index];
        frame_rate = (int) (((float) in_stream->avg_frame_rate.num /
                             (float) in_stream->avg_frame_rate.den) + 0.5);
        //打开解码器
        AVCodec *pCodec = avcodec_find_decoder(in_stream->codecpar->codec_id);
        if (pCodec == NULL) {
            LOGE("open_decodec_context()无法根据avstream找到decoder");
            release();
            return;
        }
        decode_ctx = avcodec_alloc_context3(pCodec);
        if ((avcodec_parameters_to_context(decode_ctx, in_stream->codecpar)) < 0) {
            LOGE("open_decodec_context()无法根据pCodec分配AVCodecContext");
            release();
            return;
        }

        if (avcodec_open2(decode_ctx, pCodec, NULL) < 0) {
            LOGE("open_decodec_context()无法打开编码器");
            release();
            return;
        }
        //配置输出文件
        avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, params->outFile.c_str());
        if (!ofmt_ctx) {
            LOGE("Could not create output context\n");
            free(ofmt_ctx);
            release();
            return;
        }
        out_stream = avformat_new_stream(ofmt_ctx, pCodec);
        if (out_stream == NULL) {
            LOGE("avformat_new_stream.");
            release();
            return;
        }
        ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if (avio_open(&ofmt_ctx->pb, params->outFile.c_str(), AVIO_FLAG_READ_WRITE) < 0) {
            LOGE("Couldn't open output file.");
            release();
            return;
        }
        if (avformat_write_header(ofmt_ctx, NULL) < 0) {
            LOGD("Error occurred when opening video output file.");
            release();
            return;
        }
        //打开编码器
        pCodec = avcodec_find_encoder(in_stream->codecpar->codec_id);
        if (pCodec == NULL) {
            LOGD("Can not find encoder! \n");
            release();
            return;
        }
        encode_ctx = avcodec_alloc_context3(pCodec);
        if (encode_ctx == NULL) {
            LOGD("can not  find AVCodecContext! \n");
            release();
            return;
        }

        encode_ctx->sample_aspect_ratio = in_stream->codec->sample_aspect_ratio;
        encode_ctx->codec_type = in_stream->codecpar->codec_type;
        encode_ctx->pix_fmt = in_stream->codec->pix_fmt;
        encode_ctx->bit_rate_tolerance = in_stream->codec->bit_rate_tolerance;
        encode_ctx->width = in_stream->codecpar->width;
        encode_ctx->height = in_stream->codecpar->height;
        encode_ctx->bit_rate = in_stream->codec->bit_rate;
        encode_ctx->gop_size = in_stream->codec->gop_size;
        encode_ctx->time_base = in_stream->codec->time_base;
        encode_ctx->time_base.num = 1;
        encode_ctx->time_base.den = frame_rate;
        encode_ctx->qmin = in_stream->codec->qmin;
        encode_ctx->qmax = in_stream->codec->qmax;
        encode_ctx->qcompress = 1;
        encode_ctx->max_b_frames = in_stream->codec->max_b_frames;
        av_opt_set(encode_ctx->priv_data, "preset", "ultrafast", 0);
        av_opt_set(encode_ctx->priv_data, "tune", "zerolatency", 0);
        ret = avcodec_open2(encode_ctx, pCodec, NULL);
        if (ret < 0) {
            LOGD("Failed to open encoder! ret=%d", ret);
            release();
            return;
        }
        if (params->effect != NULL) {
            params->effect->init(ifmt_ctx, ofmt_ctx, decode_ctx, encode_ctx, in_stream, out_stream,
                                 params->rangeStart, params->rangeEnd);
        }
        LOGD("VideoCodeProduce  init");
    }

    void VideoCodeProduce::readPackage() {
        if (ifmt_ctx != NULL) {
            LOGD("VideoCodeProduce  readPackage start");
            if (params->rangeStart > 0) {
                av_seek_frame(ifmt_ctx, -1, params->rangeStart / 1000 * AV_TIME_BASE,
                              AVSEEK_FLAG_BACKWARD);
            }
            AVPacket pkt;
            while (!isStop) {
                if (srcPacketQueue->size() < 3) {
                    LOGD("VideoCodeProduce   srcPacketQueue->size()=%d",
                         srcPacketQueue->size());
                    int ret = av_read_frame(ifmt_ctx, &pkt);
                    if (ret < 0) {
                        readFinish = true;
                        break;
                    }
                    if (pkt.stream_index == video_index) {
                        double time = av_q2d(in_stream->time_base) * pkt.pts * 1000;
                        if (time >= params->rangeEnd) {
                            if (params->effect != NULL) {
                                params->effect->onReadPackage(pkt);
                                long seekTimeTemp = params->effect->getNeedSeekTime(pkt);
                                if (seekTimeTemp >= 0) {
                                    seekTime = seekTimeTemp;
                                    av_seek_frame(ifmt_ctx, -1, (seekTime / 1000) * AV_TIME_BASE,
                                                  AVSEEK_FLAG_BACKWARD);
                                    continue;
                                }
                            }
                            readFinish = true;
                            break;
                        }
                        if (params->effect != NULL) {
                            params->effect->onReadPackage(pkt);
                        }
                        pthread_mutex_lock(mutex_src_package);
                        srcPacketQueue->push(pkt);
                        pthread_mutex_unlock(mutex_src_package);
                        if (params->effect != NULL) {
                            params->effect->onReadPackage(pkt);
                            long seekTimeTemp = params->effect->getNeedSeekTime(pkt);
                            if (seekTimeTemp >= 0) {
                                seekTime = seekTimeTemp;
                                av_seek_frame(ifmt_ctx, -1, (seekTime / 1000) * AV_TIME_BASE,
                                              AVSEEK_FLAG_BACKWARD);
                            }
                        }
                    }

                } else {
                    usleep(10);
                }

            }
        }
    }

    void VideoCodeProduce::decodePackage() {
        if (params->effect != NULL && !params->effect->needDecode()) {
            return;
        }
        LOGD("VideoCodeProduce  decodePackage start");
        while (!isStop) {
            if (srcPacketQueue->size() > 0 && decodeFrameQueue->size() < 5) {
                LOGD("VideoCodeProduce     decodeFrameQueue->size()=%d",
                     decodeFrameQueue->size());
                AVFrame *frame = av_frame_alloc();
                pthread_mutex_lock(mutex_src_package);
                AVPacket pkt = srcPacketQueue->front();
                srcPacketQueue->pop();
                pthread_mutex_unlock(mutex_src_package);
                int ret = avcodec_send_packet(decode_ctx, &pkt);
                if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                    LOGE("avcodec_send_packet fail");
                    av_packet_unref(&pkt);
                    release();
                    break;
                }
                //从解码器返回解码输出数据
                ret = avcodec_receive_frame(decode_ctx, frame);
                if (ret < 0 && ret != AVERROR_EOF) {
                    LOGE("avcodec_receive_frame fail");
                    av_packet_unref(&pkt);
                    av_frame_free(&frame);
                    release();
                    break;
                }
                av_packet_unref(&pkt);
                double time = av_q2d(in_stream->time_base) * frame->pts * 1000;
                LOGD("VideoCodeProduce rangeStart=%ld  time=%lf", params->rangeStart, time);
                if (time < params->rangeStart) {
                    continue;
                }
                if (seekTime >= 0 && time < seekTime) {
                    continue;
                }
                DecodeFrame *decodeFrame = new DecodeFrame();
                decodeFrame->frame = frame;
                decodeFrame->writeTimes = 1;
                if (params->effect != NULL) {
                    params->effect->optFrame(decodeFrame);
                }
                if (decodeFrame->writeTimes > 0) {
                    pthread_mutex_lock(mutex_decode_frame);
                    decodeFrameQueue->push(decodeFrame);
                    pthread_mutex_unlock(mutex_decode_frame);
                }
            } else {
                if (readFinish && srcPacketQueue->size() == 0) {
                    break;
                } else {
                    usleep(10);
                }
            }
        }
    }

    void VideoCodeProduce::encodeFrame() {
        if (params->effect != NULL && !params->effect->needDecode()) {
            return;
        }
        LOGD("VideoCodeProduce  encodeFrame start");
        while (!isStop) {
            if (decodeFrameQueue->size() > 0 && outPacketQueue->size() < 5) {
                LOGD("VideoCodeProduce   outPacketQueue->size()=%d  ",
                     outPacketQueue->size());

                pthread_mutex_lock(mutex_decode_frame);
                DecodeFrame *decodeFrame = decodeFrameQueue->front();
                decodeFrameQueue->pop();
                pthread_mutex_unlock(mutex_decode_frame);
                AVFrame *frame = decodeFrame->frame;
                int result = 0;
                for (int i = 0; i < decodeFrame->writeTimes; i++) {
                    AVPacket pkt;
                    av_init_packet(&pkt);
                    pkt.data = NULL; // packet data will be allocated by the encoder
                    pkt.size = 0;
                    int ret = avcodec_send_frame(encode_ctx, frame);
                    if (ret != 0) {
                        LOGD("avcodec_send_frame fail,ret=%d", ret);
                        av_packet_unref(&pkt);
                        result = -1;
                        break;
                    }
                    ret = avcodec_receive_packet(encode_ctx, &pkt);
                    if (ret < 0 && ret != AVERROR_EOF) {
                        LOGE("avcodec_receive_frame fail");
                        av_packet_unref(&pkt);
                        result = -1;
                        break;
                    }
                    pthread_mutex_lock(mutex_out_package);
                    outPacketQueue->push(pkt);
                    pthread_mutex_unlock(mutex_out_package);
                }
                if (result < 0) {
                    release();
                }
                delete decodeFrame;
                av_frame_unref(frame);
                av_frame_free(&frame);
                frame = NULL;

            } else {
                if (readFinish && srcPacketQueue->size() == 0 && decodeFrameQueue->size() == 0) {
                    break;
                } else {
                    usleep(10);
                }
            }
        }

    }

    void VideoCodeProduce::writeFile() {
        LOGD("VideoCodeProduce  writeFile start");
        while (!isStop) {
            AVPacket pkt;
            int len = 0;
            if (params->effect != NULL && !params->effect->needDecode()) {
                len = srcPacketQueue->size();
            } else {
                len = outPacketQueue->size();
            }
            if (len > 0) {
                LOGD("VideoCodeProduce  writeFile len=%d  ", len);
                if (params->effect != NULL && !params->effect->needDecode()) {
                    LOGD("VideoCodeProduce  writeFile SrcAVPacket  ");
                    pthread_mutex_lock(mutex_src_package);
                    pkt = srcPacketQueue->front();
                    srcPacketQueue->pop();
                    pthread_mutex_unlock(mutex_src_package);

                } else {
                    LOGD("VideoCodeProduce  writeFile outPacketQueue  ");
                    pthread_mutex_lock(mutex_out_package);
                    pkt = outPacketQueue->front();
                    outPacketQueue->pop();
                    pthread_mutex_unlock(mutex_out_package);

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
                if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
                    av_packet_unref(&pkt);
                    LOGE("Error muxing packet\n");
                    release();
                    break;
                }
                av_packet_unref(&pkt);
            } else {
                if (readFinish && srcPacketQueue->size() == 0 && decodeFrameQueue->size() == 0 &&
                    len == 0) {
                    av_write_trailer(ofmt_ctx);
                    isSuccess = true;
                    release();
                    break;
                } else {
                    usleep(10);
                }
            }
        }
    }

    static void *threatExecute(void *arg) {
        if (arg != NULL) {
            ThreadExecuteParams *params = (ThreadExecuteParams *) arg;
            switch (params->methodId) {
                case METHOD_ID_READ:
                    params->produce->readPackage();
                    break;
                case METHOD_ID_DECODE:
                    params->produce->decodePackage();
                    break;
                case METHOD_ID_ENCODE:
                    params->produce->encodeFrame();
                    break;
                case METHOD_ID_WRITE:
                    params->produce->writeFile();
                    break;
            }
            delete (params);
        }
        return NULL;
    }
}