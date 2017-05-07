//
// Created by jakechen on 2017/2/10.
//
#include "FFmpegBase.h"
#include "FFmpegUtil.h"
#include <jni.h>
extern "C" {
#include <libavutil/opt.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <libavutil/imgutils.h>
}
/**
 * 开始录制视频，主要是初始化
 * @param h264File
 * @param width
 * @param height
 * @return
 */
int videoRecordStart(const char *h264File, int width, int height);
/**
 * 视频录制中，主要是将y u v数据编码成👌h264
 * @param data yuv数据
 * @return
 */
int videoRecording(uint8_t *data);
/**
 * 停止录制视频
 * @return
 */
int videoRecordEnd();
