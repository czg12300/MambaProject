//
// Created by jakechen on 2017/3/9.
//

#include "FFmpegBase.h"
#include <sys/stat.h>
#include <dirent.h>
#include "Muxer.h"
#include "VideoOptHandler.h"
#include "FFmpegUtil.h"
#include "HandleProgressCallback.h"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
};
#ifndef XIUGE_VIDEORECODEUTIL_H
#define XIUGE_VIDEORECODEUTIL_H

/**
 * 本类为处理视频解码后再编码的核心代码，用于处理需要将视频解码后再编码的功能使用
 */
class VideoRecodeFactory {
    AVCodecContext *ecodecCtx = NULL;
    VideoOptHandler *handler;
    AVPacket pkt;
    AVFormatContext *ifmt_ctx = NULL;
    int video_index = -1;
    OutputStreamContext *v_out_ctx = NULL;
public:
    string tempDir;
    string outAac;
    string outOptAac;
    string outOptH264;
    string srcFile;
    string outFile;

/**
 * 构造函数
 * @param src_File 源文件
 * @param out_File 输出文件
 * @return
 */
    VideoRecodeFactory(string src_File, string out_File);

/**
 * 分离音频
 * @return >=0,表示成功，否则失败
 */
    int demuxerAudio();

/**
 * 音视频合并，将音频流和视频流合并为视频
 * @param h264 视频流，不仅仅为264码流，还可以是其他视频流
 * @param aac 音频流，不仅仅为aac码流，还可以是其他音频流
 * @param file  输出文件
 * @return  >=0,表示成功，否则失败
 */
    int muxer(string h264, string aac, string file);

/**
 * 对avFrame进行编码并写入
 * @param frame
 * @return  >=0,表示成功，否则失败
 */
    int encode_write_frame(AVFrame *frame);

/**
 * 处理视频的解码后编码写入流程
 * @param handler 视频处理回调
 * @return  >=0,表示成功，否则失败
 */
    int optVideo(VideoOptHandler *handler);

/**
 * 析构函数，用于释放资源
 */
    ~VideoRecodeFactory();

};


#endif //XIUGE_VIDEORECODEUTIL_A
