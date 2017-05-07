//
// Created by jakechen on 2017/1/17.
//

#include "FFmpegBase.h"
#include "FFmpegUtil.h"
#include "VideoRecodeFactory.h"

extern "C" {
#include "libavformat/avformat.h"
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>

}

#ifndef _XIUGE_CUT_VIDEO_HH
#define _XIUGE_CUT_VIDEO_HH

class CutVideoOptHandler : public VideoOptHandler {
private:
    double from_seconds;
    double end_seconds;
    int64_t *dts_start_from;
    int64_t *pts_start_from;
public:
    CutVideoOptHandler(double from_seconds, double end_seconds);

    int init(AVFormatContext *ifmt_ctx, AVCodecContext *decodecCtx, int video_index,int frameCount);

    AVFrame *optAvFrame(AVFrame *frame, int frameIndex);
    int changeFrameRate(int frameRate);
    AllowEncodeAndWriteFrameState allowEncodeAndWriteFrame(AVStream *inStream, AVPacket pkt,int frameIndex);
    void changeTimebase(AVStream *inStream, AVStream *outStream, AVPacket pkt);
    void releaseInEnd();

};

#endif

/**
 * 剪辑视频，根据起始时间和终止时间剪辑视频
 * @param from_seconds 起始时间
 * @param end_seconds 终止时间
 * @param in_filename 输入文件
 * @param out_filename 输出文件
 * @return  >=0 ,表示成功 ，否则表示失败
 */
int cut_video(double from_seconds, double end_seconds, const char *in_filename,
              const char *out_filename);/**
 * 剪辑音频，根据起始时间和终止时间剪辑视频
 * @param from_seconds 起始时间
 * @param end_seconds 终止时间
 * @param in_filename 输入文件
 * @param out_filename 输出文件
 * @return  >=0 ,表示成功 ，否则表示失败
 */
int cut_audio(double from_seconds, double end_seconds, const char *in_filename,
              const char *out_filename);