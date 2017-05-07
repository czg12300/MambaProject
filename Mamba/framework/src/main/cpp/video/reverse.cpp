//
// Created by walljiang on 2017/04/05.
//
#include <vector>
#include "reverse.h"
#include <semaphore.h>
#include <pthread.h>
#include "Log.h"
#include <stack>
#include <queue>
#include "bmputil.h"
#include <libswscale/swscale.h>

static AVFormatContext *ifmt_ctx;
static AVFormatContext *ofmt_ctx;
static int frame_count;
static int videoStreamIdx, audioStreamIdx;
//static std::vector<AVFrame> nextFrameCaches;
static int64_t frameCacheSize = 0, nextFrameCacheSize = 0;
static int64_t decodingDts;
static sem_t sem,sem2;
static std::vector<void *>::iterator it;
static int64_t decodeDts;
static std::stack<char *> frameStack;
static std::queue<AVFrame *> frameQueue;
static int width = 0,height=0;
#define MAX_SIZE 52428800

static struct CustonAVFrame{
    AVFrame* data;
    uint64_t pts;
    uint64_t dts;
    uint64_t duration;
};
static struct cacheVector {
    int size;
    int64_t data_size;
    std::vector<CustonAVFrame *> *data;
    int data_type;//0:frameCachesData,1:keyDtsData;
};

static std::vector<CustonAVFrame *> frameCachesData;
static cacheVector frameCaches = {0, 0, &frameCachesData, 0};
static std::vector<int64_t> keyDts;

static int addDataEnd(cacheVector *vector, CustonAVFrame *data);

static int addDataFront(cacheVector *vector, CustonAVFrame *data);

static int eraseDataFront(cacheVector *vector);

static int getDataFrameVector(CustonAVFrame **data, cacheVector *vector);


int  save_picture_init_2(FILE ** pFile,char * filename)
{
    *pFile = fopen(filename, "wb");
    return 1;
}

void  save_picture_uinit(FILE * pFile,AVPacket pkt)
{
    fwrite(pkt.data, sizeof(uint8_t),pkt.size, pFile);
    fclose(pFile);
//    av_free_packet(&pkt);
}

void generate_file_name(char * file_name,char * file_path,long long pts)
{
    if (file_path[strlen(file_path) - 1] == '/')
    {
        sprintf(file_name,"%s%lld.jpg",file_path,pts);
    }
    else
    {
        sprintf(file_name,"%s/%lld.jpg",file_path,pts);
    }
}
//static int open_input_file2(const char *filename) {
//    int ret;
//    ifmt_ctx2 = NULL;
//    if ((ret = avformat_open_input(&ifmt_ctx2, filename, NULL, NULL)) < 0) {
//        LOGE("Cannot open input file\n");
//        return ret;
//    }
//    if ((ret = avformat_find_stream_info(ifmt_ctx2, NULL)) < 0) {
//        LOGE("Cannot find stream information\n");
//        return ret;
//    }
//    return 0;
//}

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
                enc_ctx->width = dec_ctx->width;
                enc_ctx->height = dec_ctx->height;
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
            LOGE("Elementary stream #%d is of unknown type, cannot proceed\n",
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
        LOGI("ofmt_ctx->oformat->flags：%d\n", ofmt_ctx->oformat->flags);
        ret = avio_open(&ofmt_ctx->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            char errbuf[128];
            const char *errbuf_ptr = errbuf;
            errbuf_ptr = strerror(AVUNERROR(ret));
            LOGE("%s: %s\n", filename, errbuf_ptr);
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
    LOGI("getKeyDts \n");
    int ret = 0;
    AVPacket packet;
    av_init_packet(&packet);
    while (1) {
        if (av_read_frame(ifmt_ctx, &packet) < 0) {
            LOGE("av_read_frame < 0 \n");
            break;
        }
        if (packet.flags == AV_PKT_FLAG_KEY && packet.stream_index == videoStreamIdx) {
            keyDts.insert(keyDts.end(), packet.dts);
//            addDataEnd(&keyDts, &(packet.dts));
//            keyDts.data->insert(keyDts.data->begin() + keyDts.size, &dts);
//            keyDts.data_size = keyDts.data_size+ (sizeof(void *)*8);
//            keyDts.size++;
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

/**
 * 解码线程方法，方法格式固定
 * @param arg
 * @return
 */
static void *decodeToCaches(void *arg) {
    LOGI("decode thread start");
    sem_wait(&sem);
    int ret = 0, got_frame;
    AVFrame *frame;
    AVPacket packet;
    av_init_packet(&packet);
    int i = 0;
    av_seek_frame(ifmt_ctx, videoStreamIdx, keyDts[0], AVSEEK_FLAG_FRAME);

    //解码最后一个gop组
    while (1) {
        if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0) {
            LOGE("av_read_frame failed:%d\n", ret);
            ret = 0;
            break;
        }
        if(packet.dts == keyDts[2]){
            break;
        }
        if (packet.stream_index == videoStreamIdx) {
            frame = av_frame_alloc();
//            CustonAVFrame custonAVFrame = {frame,0,0,0};
            ret = decode_packet(&packet, &frame, &got_frame);
            if (frame && frame->width>0 && frame->height > 0) {

                LOGI("decodeToCaches0 data_size:%lld,framwidth:%d,frameheight:%d,framelinesize:%d,pts:%lld,dts:%lld",frameCaches.data_size,frame->width,frame->height,frame->linesize[0],packet.pts,packet.dts);
//                frameStack.push(frame);
                width = frame->width;
                height = frame->height;
                LOGI("decodeToCaches 1 width:%d,height:%d",width,height);
                char *tmp_stk = new char[frame->width * frame->height * 3 / 2];
                memset(tmp_stk, 0, frame->width * frame->height * 3 / 2);
                int n_step = 0;

                for(int i = 0; i < frame->height; i++) {
                    memcpy(tmp_stk + n_step, frame->data[0] + i * frame->linesize[0], frame->width);
                    n_step += frame->width;
                }
                for(int i = 0; i < frame->height/2; i++) {
                    memcpy(tmp_stk + n_step, frame->data[1] + i * frame->linesize[1], frame->width/2);
                    n_step += frame->width/2;
                }
                for(int i = 0; i < frame->height/2; i++) {
                    memcpy(tmp_stk + n_step, frame->data[2] + i * frame->linesize[2], frame->width/2);
                    n_step += frame->width/2;
                }
                frameStack.push(tmp_stk);
//                frameQueue.push(frame);
//                if (frameCaches.data_size > MAX_SIZE) {
//                    eraseDataFront(&frameCaches);
//                }
//                custonAVFrame.data = frame;
//                custonAVFrame.pts = packet.pts;
//                custonAVFrame.dts = packet.dts;
//                custonAVFrame.duration = packet.duration;
//                addDataEnd(&frameCaches, &custonAVFrame);
            }
        }
    }
    //解码最后一个gop组
    av_seek_frame(ifmt_ctx,videoStreamIdx,keyDts[0],AVSEEK_FLAG_FRAME);
    sem_post(&sem2);
    LOGI("decode thread exit");
    if (packet.size > 0) {
        packet.size = 0;
        av_packet_unref(&packet);
    }
    LOGI("decode thread exit2");
    pthread_exit(NULL);
}
//
//static int addDataEnd(cacheVector *vector, CustonAVFrame* data) {
//    int ret = -1;
//    if (vector != NULL && vector->data != NULL) {
//        LOGI("addDataEnd 1");
//        vector->data->insert(vector->data->end(), data);
////        vector->data->push_back(data);
//        vector->size = vector->size + 1;
//        if (vector->data_type == 0) {
//            int64_t width =(int64_t)(data->data->width);
//            int64_t height =(int64_t) (data->data->height);
//            int64_t datasize = vector->data_size + width * height * 3 / 2;
//            vector->data_size = datasize;
//        } else if (vector->data_type == 1) {
//            vector->data_size = vector->data_size + 64;
//        }
//        LOGI("addDataEnd 2");
//        return 0;
//    }
//    return ret;
//}
//
//static int addDataFront(cacheVector *vector, CustonAVFrame *data) {
//    int ret = -1;
//    if (vector != NULL && vector->data != NULL) {
//        vector->data->insert(vector->data->begin(), data);
//        vector->size = vector->size + 1;
//        if (vector->data_type == 0) {
//            int width = data->data->width;
//            int height = data->data->height;
//            vector->data_size = vector->data_size + width * height * 3 / 2;
//        } else if (vector->data_type == 1) {
//            vector->data_size = vector->data_size + 64;
//        }
//        return 0;
//    }
//    return ret;
//}
//
//static int eraseDataFront(cacheVector *vector) {
//    int ret = -1;
//    if (vector->size > 0) {
//        void *data = (*vector->data)[0];
//        (vector->data)->erase(vector->data->begin());
//        vector->size = vector->size - 1;
//        if (vector->data_type == 0) {
//            AVFrame *dataFrame = (AVFrame *) data;
//            int width = dataFrame->width;
//            int height = dataFrame->height;
//            vector->data_size = vector->data_size - width * height * 3 / 2;
//            LOGI("eraseDataFront");
//            av_frame_free(&dataFrame);
//        } else if (vector->data_type == 1) {
//            vector->data_size = vector->data_size - 64;
//        }
//        return 0;
//    }
//    return ret;
//}
//
//static int getDataFromVectorFront(CustonAVFrame *data,cacheVector *vector){
//    int ret = -1;
//    LOGI("111111111，%d",vector->size);
//    if (vector->size > 0) {
//
//        data = (*vector->data)[0];
//        LOGI("2222222222 :%lld",data->pts);
//        (vector->data)->erase(vector->data->begin());
//        vector->size = vector->size - 1;
//        if (vector->data_type == 0) {
//            int width = (data)->data->width;
//            int height = (data)->data->height;
//            vector->data_size = vector->data_size - width * height * 3 / 2;
//        } else if (vector->data_type == 1) {
//            vector->data_size = vector->data_size - 64;
//        }
//        LOGI("vector size:%d\n", vector->size);
//        return 0;
//    }
//    return ret;
//}
//
//static int getDataFrameVector(CustonAVFrame **data, cacheVector *vector) {
//    int ret = -1;
//    if (vector->size > 0) {
//        *data = (*vector->data)[vector->size - 1];
//        (vector->data)->erase(vector->data->end() - 1);
//        vector->size = vector->size - 1;
//        if (vector->data_type == 0) {
//            int width = ((AVFrame *) (*data))->width;
//            int height = ((AVFrame *) (*data))->height;
//            vector->data_size = vector->data_size - width * height * 3 / 2;
//        } else if (vector->data_type == 1) {
//            vector->data_size = vector->data_size - 64;
//        }
//        LOGI("vector size:%d\n", vector->size);
//        return 0;
//    }
//    return ret;
//}

int reverse(const char *srcFile, const char *outFile) {
    bool semInited = false;
    int ret=0,  i = 0;
    AVPacket packet, enc_pkt;
    AVFrame *frame;
    int got_frame, got_picture;
    AVCodecContext *pEncodeCtx;
    void *data;
    void *thread_result;
    FILE * video_dst_file = NULL;
    char file_name[50] = {0};
    char *filepath = "/sdcard/AClip/reverse2";
    int picture_size;
    uint8_t *picture_buf;

//    CustonAVFrame custonAVFrame;
//    char _outFile[100];
//    char *p;
//    char *tempFile;
//    char tmp_yuv_file_name[100];
//    char tmp_aac_file_name[100];
//    char tmp_mp4_file_name[100];
//    if(srcFile == NULL || outFile == NULL){
//        return -1;
//    }
//    strcpy(_outFile,outFile);
//    p = strtok(_outFile,".");
//    tempFile = p;
//    while(p){
//        p = strtok(NULL,".");
//    }
//    free(_outFile);
//    strcpy(tmp_yuv_file_name,tempFile);
//    strcpy(tmp_aac_file_name,tempFile);
//    strcpy(tmp_mp4_file_name,tempFile);
//    sprintf(tmp_yuv_file_name,"%s%s",tmp_yuv_file_name,"_temp.yuv");
//    sprintf(tmp_aac_file_name,"%s%s",tmp_aac_file_name,"_temp.aac");
//    sprintf(tmp_mp4_file_name,"%s%s",tmp_mp4_file_name,"_temp.mp4");
//    LOGI("reverse,tmp_yuv_file_name:%s,tmp_aac_file_name:%s,tmp_mp4_file_name:%s\n",tmp_yuv_file_name,tmp_aac_file_name,tmp_mp4_file_name);

    av_register_all();
//    if ((ret = open_input_file2(srcFile)) < 0) {
//        LOGE("open input file2 error\n");
//        goto end;
//    }
    if ((ret = open_input_file(srcFile, &videoStreamIdx, &audioStreamIdx)) < 0) {
        LOGE("Open input file error\n");
        goto end;
    }
    LOGI("audioStreamIdx:%d，videoStreamIdx:%d\n", audioStreamIdx, videoStreamIdx);
    if ((ret = open_output_video(outFile)) < 0) {
        goto end;
    }
    pEncodeCtx = ofmt_ctx->streams[videoStreamIdx]->codec;
    av_init_packet(&enc_pkt);
    frame = av_frame_alloc();
    picture_size = avpicture_get_size(pEncodeCtx->pix_fmt, width, height);
    picture_buf = (uint8_t *)av_malloc(picture_size);
    avpicture_fill((AVPicture *)frame, picture_buf, pEncodeCtx->pix_fmt, width, height);
//    av_new_packet(&enc_pkt, picture_size);
    //threadinit
    pthread_t a_thread;
    ret = sem_init(&sem, 0, 0);
    ret = sem_init(&sem2,0,0);
    if (ret != 0) {
        LOGE("Sem init failed");
        return -1;
    }
    semInited = true;
    ret = pthread_create(&a_thread, NULL, decodeToCaches, NULL);
    if (ret != 0) {
        LOGE("Thread create failed");
        return -1;
    }
    //threadinit

    getKeyDts();
    if (keyDts.size() > 0) {
        LOGI("sem_post");
        sem_post(&sem);
    }
    sem_wait(&sem2);
    while (1) {
        //AVSEEK_FLAG_ANY和AVSEEK_FLAG_FRAME都是向后seek，AVSEEK_FLAG_BACKWARD是向前seek
        if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0) {
            LOGE("av_read_frame failed:%d\n", ret);
            ret = 0;
            break;
        }
        if(packet.dts == keyDts[2]){
            LOGI("SSSSS");
            break;
        }
        if (packet.stream_index == audioStreamIdx) {
            LOGI("time to decode audio");
            ret = decode_packet(&packet, &frame, &got_frame);
        } else if (packet.stream_index == videoStreamIdx) {
            //seek到最后一个关键帧处，解码改gop组，并把
            LOGI("packet.stream_index == videoStreamIdx");
//            if ((getDataFromVectorFront(&custonAVFrame, &frameCaches)) < 0) {
//                LOGE("getDataFrameVector cacheFrame failed\n");
//                break;
//            }
            if(!frameStack.empty()){
                picture_buf = (uint8_t *)frameStack.top();
                LOGI("while 1 width:%d,height:%d",width,height);
                int ysize = width * height;
                frame->data[0] = (uint8_t *)picture_buf;                   // Y
                frame->data[1] = (uint8_t *)picture_buf + ysize;          // U
                frame->data[2] = (uint8_t *)picture_buf + ysize * 5 / 4;  // V
                //int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(ifmt_ctx->streams[stream_index]->r_frame_rate);
                //pFrame->pts = (double)(i * calc_duration) / (double)(av_q2d(ifmt_ctx->streams[stream_index]->time_base) * AV_TIME_BASE);

                frame->pts = 2*i;
                i++;
                frame->width = width;
                frame->height = height;
                frame->format = AV_PIX_FMT_YUV420P;
//                if (frame)
//                {
//                    LOGI("width:%d,height:%d",frame->width,frame->height);
//                    SaveBmpPicture(frame, frame->height,frame->width , i);
//                }
                frameStack.pop();
            }else{
                break;
            }
//            frame->pts = packet.pts;
            LOGI("framestart pts:%lld,%lld",frame->pts,packet.pts);
            int ret = avcodec_encode_video2(pEncodeCtx, &enc_pkt, frame, &got_picture);
            if (ret < 0) {
                LOGE("Failed to encode!\n");
                break;
            }
            LOGI("encode success 0");
//            av_frame_free(&(frame));

            if (!(got_picture)) {
                LOGI("encode failed");
                continue;
            }

            LOGI("encode success 1");
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
            LOGI("WRITE PTS:%lld",enc_pkt.pts);
            ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);

//            if (ret < 0) {
//                break;
//            }
        }
    }
    av_write_trailer(ofmt_ctx);
    end:
    if (semInited) {
        sem_close(&sem);
        sem_destroy(&sem);
        sem_close(&sem2);
        sem_destroy(&sem2);
    }
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