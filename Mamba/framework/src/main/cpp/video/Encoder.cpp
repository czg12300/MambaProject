


#include <stdio.h>
#include "FFmpegBase.h"
#include <jni.h>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <libavutil/imgutils.h>

AVCodecContext *pCodecCtx = NULL;
AVPacket avpkt;
FILE *video_file;
unsigned char *outbuf = NULL;
unsigned char *yuv420buf = NULL;
AVFrame *yuv420pframe = NULL;
static int outsize = 0;
static int mwidth = 800;
static int mheight = 480;
int count = 0;
/* 
* encording init 
*/

JNIEXPORT jint JNICALL
Java_com_kugou_xg_clip_CameraEncodeFragment_videoinit(JNIEnv *env, jclass obj, jstring filename) {
    LOGI("%s\n", __func__);
    AVCodec *pCodec = NULL;
    avcodec_register_all();
    pCodec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);  //AV_CODEC_ID_H264//AV_CODEC_ID_MPEG1VIDEO
    if (pCodec == NULL) {
        LOGE("++++++++++++codec not found\n");
        return -1;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (pCodecCtx == NULL) {
        LOGE("++++++Could not allocate video codec context\n");
        return -1;
    }
    /* put sample parameters */
    pCodecCtx->bit_rate = 400000;
    /* resolution must be a multiple of two */
    pCodecCtx->width = mwidth;
    pCodecCtx->height = mheight;
    /* frames per second */
    pCodecCtx->time_base = (AVRational) {1, 25};
    pCodecCtx->gop_size = 10; /* emit one intra frame every ten frames */
    pCodecCtx->max_b_frames = 1;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;//AV_PIX_FMT_YUYV422;  
    /* open it */
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGE("+++++++Could not open codec\n");
        return -1;
    }
    outsize = mwidth * mheight * 2;
    outbuf = (unsigned char *) malloc(outsize * sizeof(char));
    yuv420buf = (unsigned char *) malloc(outsize * sizeof(char));
    const char * file=env->GetStringUTFChars(filename,NULL);
    if ((video_file = fopen(file, "wb")) == NULL) {
        LOGE("++++++++++++open %s failed\n", file);
        return -1;
    }
    env->ReleaseStringUTFChars(filename,file);
    return 1;
}

JNIEXPORT jint JNICALL
Java_com_kugou_xg_clip_CameraEncodeFragment_videostart(JNIEnv *env, jclass obj, jbyteArray yuvdata) {
    int frameFinished = 0, size = 0;
    jbyte *ydata = env->GetByteArrayElements(yuvdata, 0);
    yuv420pframe = NULL;
    //AVFrame * yuv422frame=NULL;  
    //struct SwsContext *swsctx = NULL;
    av_init_packet(&avpkt);
    avpkt.data = NULL;    // packet data will be allocated by the encoder
    avpkt.size = 0;
    yuv420pframe = av_frame_alloc();
    int y_size = pCodecCtx->width * pCodecCtx->height;
    /*
    if (!yuv420pframe) {
        printf("Could not allocate video frame\n");  
        return -1;  
    }  
    yuv420pframe->format = pCodecCtx->pix_fmt;  
    yuv420pframe->width  = pCodecCtx->width;  
    yuv420pframe->height = pCodecCtx->height;  
  
    int ret = av_image_alloc(yuv420pframe->data, yuv420pframe->linesize, pCodecCtx->width, pCodecCtx->height,  
                         pCodecCtx->pix_fmt, 16);  
    if (ret < 0) {  
        printf("Could not allocate raw picture buffer\n");  
        return -1;  
    }
*/

    uint8_t *picture_buf;
    int size1 = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
    picture_buf = (uint8_t *) av_malloc(y_size);
    if (!picture_buf) {
        av_free(yuv420pframe);
    }
    avpicture_fill((AVPicture *) yuv420pframe, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width,
                   pCodecCtx->height);

    //yuv422frame=avcodec_alloc_frame();
    yuv420pframe->pts = count;
    yuv420pframe->data[0] = (uint8_t *) ydata;  //PCM Data
    yuv420pframe->data[1] = (uint8_t *) (ydata + y_size);      // U
    yuv420pframe->data[2] = (uint8_t *) (ydata + y_size * 5 / 4);  // V

    //avpicture_fill((AVPicture *) yuv420pframe, (uint8_t *)ydata, AV_PIX_FMT_YUV420P,mwidth,mheight);//(uint8_t *)yuv420buf	
    LOGE("JNICALL Java_com_kugou_xg_clip_CameraEncode_videostart3");
    //avpicture_fill((AVPicture *) yuv422frame, (uint8_t *)ydata, AV_PIX_FMT_YUYV422,mwidth,mheight);
    LOGE("JNICALL Java_com_kugou_xg_clip_CameraEncode_videostart4");
    //swsctx = sws_getContext(mwidth,mheight, AV_PIX_FMT_YUYV422, mwidth, mheight,AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL); 
    LOGE("JNICALL Java_com_kugou_xg_clip_CameraEncode_videostart5");
    //sws_scale(swsctx,(const uint8_t* const*)yuv422frame->data,yuv422frame->linesize,0,mheight,yuv420pframe->data,yuv420pframe->linesize); 
    LOGE("JNICALL Java_com_kugou_xg_clip_CameraEncode_videostart6");
    LOGE("JNICALL Java_com_kugou_xg_clip_CameraEncode_videostart7");
    size = avcodec_encode_video2(pCodecCtx, &avpkt, yuv420pframe, &frameFinished);
    LOGE("JNICALL Java_com_kugou_xg_clip_CameraEncode_videostart8");
    count++;
    if (size < 0) {
        LOGE("+++++Error encoding frame\n");
        return -1;
    }
    if (frameFinished)
        fwrite(avpkt.data, 1, avpkt.size, video_file);
    LOGE("JNICALL Java_com_kugou_xg_clip_CameraEncode_videostart9");
    av_free_packet(&avpkt);
    //sws_freeContext(swsctx);  
    av_free(yuv420pframe);
    //av_free(yuv422frame);  
    env->ReleaseByteArrayElements(yuvdata, ydata, 0);
}

JNIEXPORT jint JNICALL Java_com_kugou_xg_clip_CameraEncodeFragment_videoclose(JNIEnv *env, jclass obj) {
    fclose(video_file);
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);
    av_freep(&yuv420pframe->data[0]);
    av_frame_free(&yuv420pframe);
    free(outbuf);
}
}