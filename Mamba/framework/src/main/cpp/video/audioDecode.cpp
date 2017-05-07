//
// Created by walljiang on 2017/04/12.
//
#include "audioDecode.h"
#include "audioTranscode.h"

FILE *fp0 = NULL;
static AVFormatContext *ifmt_ctx;
static AVFormatContext *ofmt_ctx;
static AVCodecContext *decodeCtx= NULL;
static AVCodec *codec;
static int audioStreamIndex = -1;
static AVFrame* pFrame = NULL,*frame_out = NULL;
AVPacket packet;
static int swrInitFlag = 0;

int ffDecode_init(AVFormatContext *ifmt_ctx){
    int ret=0;
    AVCodecParameters *pAVCodecParameters;
    audioStreamIndex = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, &codec, 0);
    LOGI("audioStreamIndex:%d",audioStreamIndex);
    if (audioStreamIndex < 0) {
        LOGD("Cannot find a video stream in the input file\n");
        return ret;
    }
    pAVCodecParameters = avcodec_parameters_alloc();
    if(!pAVCodecParameters){
        LOGE("无法avcodec_parameters_alloc");
        return ret;
    }
    if((ret = avcodec_parameters_copy(pAVCodecParameters,ifmt_ctx->streams[audioStreamIndex]->codecpar))<0){
        LOGE("avcodec_parameters_copy error \n");
        avcodec_parameters_free(&pAVCodecParameters);
        return ret;
    }
    decodeCtx = avcodec_alloc_context3(codec);
    if(!decodeCtx){
        avcodec_parameters_free(&pAVCodecParameters);
        LOGE("avcodec_alloc_context3 ERROR");
        return ret;
    }
    if((ret = avcodec_parameters_to_context(decodeCtx,pAVCodecParameters))<0){
        LOGE("avcodec_parameters_to_context < 0");
        avcodec_parameters_free(&pAVCodecParameters);
        return ret;
    }
    avcodec_parameters_free(&pAVCodecParameters);
    if ((ret = avcodec_open2(decodeCtx, codec, NULL)) < 0) {
        LOGI("无法打开解码器");
        return ret;
    }
    pFrame = av_frame_alloc();
    frame_out = av_frame_alloc();
    av_init_packet(&packet);
    if(swrInitFlag == 0){
        initSwr(decodeCtx->channel_layout,decodeCtx->sample_fmt,decodeCtx->sample_rate);
        swrInitFlag = swrInitFlag + 1;
    }
    LOGI("decodeCtx,channels:%d,sample_rate:%d,sample_fmt:%s\n",decodeCtx->channels,decodeCtx->sample_rate,av_get_sample_fmt_name(decodeCtx->sample_fmt));
    return ret;
}

int ffDecode_deInit(){
    if(swrInitFlag == 1){
        deInitSwr();
        swrInitFlag = 0;
    }
    if(ifmt_ctx != NULL){
        avformat_free_context(ifmt_ctx);
        ifmt_ctx = NULL;
    }
    if(decodeCtx != NULL){
        avcodec_free_context(&decodeCtx);
        decodeCtx = NULL;
    }
    av_packet_unref(&packet);
    if(pFrame != NULL){
        av_frame_free(&pFrame);
        pFrame = NULL;
    }
    if(frame_out != NULL){
        av_frame_free(&frame_out);
        frame_out = NULL;
    }
    if(fp0 != NULL){
        fclose(fp0);
        fp0 = NULL;
    }
    return 0;
}

int ffDecode2(AVPacket* packet,AVFrame *frame){
    int ret = 0;
    if (packet->stream_index == audioStreamIndex) {
        ret = avcodec_send_packet(decodeCtx, packet);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            LOGI("向解码器发送数据失败%d\n",ret);
            return ret;
        }
        //从解码器返回解码输出数据
        ret = avcodec_receive_frame(decodeCtx,frame);
        if (ret < 0) {
            if(ret == AVERROR(EAGAIN)){
                LOGE("output is not available right now - user must try to send new input");
            }else if(ret == AVERROR_EOF){
                LOGE("the decoder has been fully flushed, and there will be no more output frames");
            }else if(ret == AVERROR(EINVAL)){
                LOGE("codec not opened, or it is an encoder");
            }else {
                LOGE("legitimate decoding errors");
            }
            return ret;
        }
    }else{
        LOGE("非音频数据包");
        return AVERROR(EAGAIN);
    }
    return ret;
}

/*
 * decodetype == 1时调用ffmpeg解码音频
 */
int ffDecode(const char *srcFile1,const char *outFile){
    int ret,i = 0;
    av_register_all();
    if ((ret = avformat_open_input(&ifmt_ctx, srcFile1, NULL, NULL)) < 0) {
        LOGE( "Cannot open input file\n");
        goto end;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
        LOGE( "Cannot find stream information\n");
        goto end;
    }
    ffDecode_init(ifmt_ctx);
    fp0 = fopen(outFile, "wb+");
    while (true) {
        i++;
        packet.data = NULL;
        packet.size = 0;
        if ((ret = av_read_frame(ifmt_ctx, &packet)) < 0) {
            LOGE("av_read_frame < 0");
            goto end;
        }
        ret = ffDecode2(&packet,pFrame);
        if(ret== AVERROR(EAGAIN)) {
            continue;
        } else if(ret == AVERROR_EOF){
            break;
        } else if(ret < 0){
            goto end;
        }
        if(pFrame){
            if(av_sample_fmt_is_planar((AVSampleFormat)decodeCtx->sample_fmt)){
                pFrame->pts = av_frame_get_best_effort_timestamp(pFrame);

                if (0 != TransSample(pFrame, frame_out,decodeCtx->sample_fmt))
                {
                    LOGE("can not swr the audio data!\n");
                    goto end;
                }
                fwrite(frame_out->data[0] , 1, frame_out->linesize[0], fp0);
            }else{
                fwrite(pFrame->data[0] , 1, pFrame->linesize[0], fp0);
            }
        }
    }
    return 0;
end:
    ffDecode_deInit();
    return ret;
}

/*
 * decodetype = 0时调用fdk-aac解码音频
 */
int fdkDecode(const char *srcFile1,const char *outFile){
    return 0;
}

int audioDecode(const char *srcFile1,const char *outFile,int decodeType){
    LOGI("audioDecode:%s ,outfile:%s,decodetype:%d",srcFile1,outFile,decodeType);
    if(decodeType == 0){
        fdkDecode(srcFile1,outFile);
    }else{
        ffDecode(srcFile1,outFile);
    }
    return 0;
}