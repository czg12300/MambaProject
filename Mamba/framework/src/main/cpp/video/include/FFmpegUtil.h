


//
//主要是一些公共的方法的提取
// Created by jakechen on 2017/2/7.
//



#include <Log.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

/*
 * 自定义分离后输出文件的上下文描述
 */
#ifndef _COMMON
#define _COMMON
struct OutputStreamContext {
    AVFormatContext *fmt_ctx = NULL;
    AVStream *stream = NULL;
    int streamIndex = 0;
};
#endif
/**
 * 复制视频角度
 * @param in
 * @param out
 */
void  copyAvStreamDict(AVStream *in,AVStream *out);

/**
 * 解码视频帧
 * @param pCodecCtx  解码上下文
 * @param pkt 解码数据
 * @return 解码成功就返回AVFrame，否则返回NULL
 */
AVFrame *decodec_video_frame(AVCodecContext *pCodecCtx, AVPacket pkt);

/**
 * 打开解码器，并返回解码器上下文
 * @param avStream
 * @return
 */
AVCodecContext *open_decodec_context(AVStream *avStream);

/**编码一帧数据
 *
 * @param codecContext
 * @param avFrame
 * @param avPacket
 * @return 0表示成功，否则失败
 */
int encodec_video_frame(AVCodecContext *codecContext, AVFrame *avFrame, AVPacket *avPacket);

/**
 * yuv420p 转换成 yuv420sp
 * @param yuv420p
 * @param yuv420sp
 * @param width
 * @param height
 */
void yuv420p_to_yuv420sp(unsigned char *yuv420p, unsigned char *yuv420sp, int width, int height);

/**
 *yuv420sp  转换成  yuv420p
 * @param yuv420sp
 * @param yuv420p
 * @param width
 * @param height
 */
void yuv420sp_to_yuv420p(unsigned char *yuv420sp, unsigned char *yuv420p, int width, int height);


/**
 * 根据AVMediaType和输出文件生成一个上下文 
 * @param fmt_ctx 视频文件的Format上下文
 * @param file 输出文件
 * @param type 媒体文件类型
 * @return
 */
OutputStreamContext *
applyOutputStreamContext(AVFormatContext *fmt_ctx, const char *file, AVMediaType type);

/**
 * 初始化编码器
 * @param codecId
 * @param width
 * @param height
 * @return
 */
AVCodecContext *applay_common_encoder(AVPixelFormat format, AVCodecID codecId, int width,
                                      int height);

/**
 * 从解码器中得到编码器信息并生成编码器
 * @param decoderCtx
 * @return
 */
AVCodecContext *applay_encoder_form_decoder(AVStream *avStream, int frameRate);

/**
 *将编码器中的缓存的帧输出
 * @param ofmt_ctx 输出文件格式的context
 * @param ecodecCtx 编码器context
 */
void flushEncoder(AVFormatContext *ofmt_ctx, AVCodecContext *ecodecCtx);

/**
 *将编码器中的缓存的帧输出
 * @param fp_out 输出文件结构体
 * @param ecodecCtx 编码器context
 */
void flushEncoder(FILE *fp_out, AVCodecContext *ecodecCtx);

/**
 * 设置视频编码器的统一参数
 * @param pCodecCtx
 * @param width
 * @param height
 */
void setVideoEncoderParams(AVCodecContext *pCodecCtx, int width, int height);

/**
 * 将yuv数据转换成avframe
 * @param pCodecCtx 编码器上下文
 * @param pts 显示数据，一般传帧数 1、2、3.。。
 * @param y
 * @param u
 * @param v
 * @return
 */
AVFrame *yuv_to_avframe(AVCodecContext *pCodecCtx, int pts, uint8_t *y, uint8_t *u, uint8_t *v);

/**
 * avframe之间的转换
 * @param orgFrame
 * @param srcFormat
 * @param dest
 * @return
 */
AVFrame *swsFrame(AVFrame *orgFrame, AVPixelFormat srcFormat, AVPixelFormat dest);