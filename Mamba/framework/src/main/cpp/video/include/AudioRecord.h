//
// Created by jakechen on 2017/3/13.
//
#include "FFmpegBase.h"
#include <jni.h>

#ifndef XIUGE_AUDIORECORD_H
#define XIUGE_AUDIORECORD_H
/**
 * 初始化编码器
 * @param out_file
 * @return
 */
int audio_encode_init(string out_file,int channels,int bitrate,int sample_rate);
/**
 * 编码音频
 * @param data
 * @return
 */
int audio_encoding(uint8_t *data,int in_buffer_size);
/**
 * 释放资源
 * @return
 */
int audio_encode_end();


#endif //XIUGE_AUDIORECORD_H
