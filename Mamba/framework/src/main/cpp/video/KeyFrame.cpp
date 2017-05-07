//
// Created by jakechen on 2017/1/12.
//


#include "KeyFrame.h"

/**
 * 根据文件获取format上下文
 * @param file_path 文件地址
 * @return AVFormatContext指针
 */
static AVFormatContext *applyAVFormatContext(const char *file_path);

/**
 * 保存帧为jpeg文件，文件命名为：num.jpeg 如：1.jpeg
 * @param pFrame 帧数据
 * @param frameNo 帧序号
 * @return  @return >=0 ,表示成功 ，否则表示失败
 */
static int saveFrameAsJPEG(AVFrame *pFrame, const char *path, int frameNo);


int demuxing_key_frame(const char *filePath, const char *picPath) {
    int ret = -1;
    int video_index = -1;
    AVCodecContext *pCodecCtx;
    int saveKeyFrameRet = -1;
    int key_frame_index = 0;
    AVPacket pkt;
    AVFrame *frame;
    AVFormatContext *fmt_ctx = applyAVFormatContext(filePath);
    if (fmt_ctx == NULL) {
        ret = -1;
        goto end;
    }
    video_index = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (video_index == -1) {
        LOGD("can't find video stream in %s\n", fmt_ctx->filename);
        ret = -1;
        goto end;
    }
    pCodecCtx = open_decodec_context(fmt_ctx->streams[video_index]);
    if (pCodecCtx == NULL) {
        LOGE("Could not open codec!");
        goto end;
    }
    frame = av_frame_alloc();
    if (!frame) {
        ret = -1;
        goto end;
    }
    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == video_index) {
            ret = avcodec_send_packet(pCodecCtx, &pkt);
            if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
                LOGE("avcodec_send_packet fail");
                continue;
            }
            //从解码器返回解码输出数据
            ret = avcodec_receive_frame(pCodecCtx, frame);
            if (ret < 0 && ret != AVERROR_EOF) {
                LOGE("avcodec_receive_frame fail");
                continue;
            }
            if (frame->key_frame) {
                LOGD("decodec_video_frame saveKeyFrame");
                saveKeyFrameRet = saveFrameAsJPEG(frame, picPath, key_frame_index++);
            }
        }
        av_packet_unref(&pkt);
    }
    if (saveKeyFrameRet >= 0) {
        ret = 1;
    }
    end:
    av_frame_free(&frame);
    avcodec_free_context(&pCodecCtx);
    avformat_close_input(&fmt_ctx);
    return ret;
}

AVFormatContext *applyAVFormatContext(const char *file_path) {
    av_register_all();
    AVFormatContext *fmt_ctx = NULL; //这个结构体描述了一个媒体文件或媒体流的构成和基本信息
    if (avformat_open_input(&fmt_ctx, file_path, NULL, NULL) < 0) {
        LOGD("avformat_open_input  error!!!\n");
        return NULL;
    }
//查找输入文件信息
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        LOGD("avformat_find_stream_info error !!!\n");
        free(fmt_ctx);
        return NULL;
    }
    return fmt_ctx;
}

int saveFrameAsJPEG(AVFrame *pFrame, const char *path, int frameNo) {
    if (path == NULL || pFrame == NULL) {
        return -1;
    }
    LOGD("start save jpeg");
    int ret = -1;
    AVPacket pkt;
    char out_file[256];
    AVStream *pAVStream;
    AVFormatContext *pFormatCtx;
    AVCodec *pCodec;
    AVCodecContext *pCodecCtx;
    char *jName = (char *) malloc(sizeof(char) * strlen(path) + sizeof(char) * strlen("/%d.jpg"));
    strcpy(jName, path);
    strcat(jName, "/%d.jpg");
    sprintf(out_file, jName, frameNo);
    // 分配AVFormatContext对象
    pFormatCtx = avformat_alloc_context();
    // 设置输出文件格式
    pFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);
    // 创建并初始化一个和该url相关的AVIOContext
    if (avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) {
        LOGD("Couldn't open output file. %s", out_file);
        ret = -1;
        goto end;
    }
    // 构建一个新stream
    pAVStream = avformat_new_stream(pFormatCtx, 0);
    if (pAVStream == NULL) {
        ret = -1;
        goto end;
    }
    pCodec = avcodec_find_encoder(pFormatCtx->oformat->video_codec);
    if (!pCodec) {
        LOGD("Codec not found.");
        ret = -1;
        goto end;
    }
    // 设置该stream的信息
    pCodecCtx = avcodec_alloc_context3(pCodec);;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUVJ420P;
    pCodecCtx->width = pFrame->width;
    pCodecCtx->height = pFrame->height;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGD("Could not open codec.");
        ret = -1;
        goto end;
    }
    //Write Header
    avformat_write_header(pFormatCtx, NULL);
    av_init_packet(&pkt);
    ret = avcodec_send_frame(pCodecCtx, pFrame);
    if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
        ret = -1;
        LOGD("avcodec_send_frame fail");
        goto end;
    }
    //从解码器返回解码输出数据
    ret = avcodec_receive_packet(pCodecCtx, &pkt);
    if (ret < 0) {
        LOGD("avcodec_receive_packet fail");
        ret = -1;
        goto end;
    }
    ret = av_write_frame(pFormatCtx, &pkt);
    av_write_trailer(pFormatCtx);
    end:
    free(jName);
    av_packet_unref(&pkt);
    avcodec_close(pCodecCtx);
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);
    if (ret >= 0) {
        ret = 1;
    }
    LOGD("save jpeg success");
    return ret;
}