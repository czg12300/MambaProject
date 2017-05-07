//
// Created by jakechen on 2017/3/9.
//

#ifndef XIUGE_VIDEOHANDLER_H
#define XIUGE_VIDEOHANDLER_H
extern "C" {
#include <libavformat/avformat.h>
};
/**
 * 是否允许对帧进行编码和写入的状态，分为3个状态
 * STATE_ALLOW（允许写入）、STATE_NOT_ALLOW（不允许写入）、STATE_FINISH_WRITE（完成写入）
 */
enum AllowEncodeAndWriteFrameState{
    STATE_ALLOW,STATE_NOT_ALLOW,STATE_FINISH_WRITE
};
/**
 * 本类为接口类，所有函数皆为纯函数，用于VideoRecordFactory的回调，子类需实现所有函数
 */
class VideoOptHandler {
public:
    /**
     * 初始化
     * @param ifmt_ctx
     * @param decodecCtx
     * @param video_index
     * @param frameCount
     * @return >=0表示成功，否则失败
     */
    virtual int init(AVFormatContext *ifmt_ctx, AVCodecContext *decodecCtx, int video_index,int frameCount)=0;
/**
 * 处理一帧的数据 ，如滤镜  缩放  加水印 或者收集解码后的帧数据
 * @param frame
 * @param frameIndex
 * @return  返回处理后的
 */
    virtual AVFrame *optAvFrame(AVFrame *frame, int frameIndex)=0;
/**
 * 在处理视频结尾的时候，释放资源
 */
    virtual void releaseInEnd()=0;
    /**
     * 修改帧率 ，如在减速视频的时候可以通过改变帧率实现
     * @param frameRate
     * @return 改变后的帧率
     */
    virtual int changeFrameRate(int frameRate)=0;
    /**
     * 判断是否要写入该帧
     * @param inStream
     * @param pkt 未解码的数据包
     * @param frameIndex
     * @return STATE_ALLOW（允许写入）、STATE_NOT_ALLOW（不允许写入）、STATE_FINISH_WRITE（完成写入）
     */
    virtual AllowEncodeAndWriteFrameState allowEncodeAndWriteFrame(AVStream *inStream, AVPacket pkt,int frameIndex)=0;
/**
 * 修改编码后的帧的时间戳
 * @param inStream
 * @param outStream
 * @param pkt
 */
    virtual void changeTimebase(AVStream *inStream, AVStream *outStream, AVPacket pkt)=0;
};

#endif //XIUGE_VIDEOHANDLER_H
