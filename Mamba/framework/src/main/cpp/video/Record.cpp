//
// Created by jakechen on 2017/3/22.
//
#include "VideoRecord.h"
//static AVFormatContext *ofmt_ctx;
//static AVStream* video_st;
////视音频流对应的结构体，用于视音频编解码。
//static AVCodecContext* pCodecCtx;
//static AVCodec* pCodec;
//static AVPacket enc_pkt; // 存储压缩数据（视频对应H.264等码流数据，音频对应AAC/MP3等码流数据）
//static AVFrame *pFrameYUV; // 存储非压缩的数据（视频对应RGB/YUV像素数据，音频对应PCM采样数据）
//
//int framecnt = 0;
//int yuv_width;
//int yuv_height;
//int y_length;
//int uv_length;
//int64_t start_time;
////const char* out_path = "rtmp://192.168.2.176/live/livestream";
//
////Output FFmpeg's av_log()
//void custom_log(void *ptr, int level, const char* fmt, va_list vl) {
//    FILE *fp = fopen("/storage/emulated/0/av_log.txt", "a+");
//    if (fp) {
//        vfprintf(fp, fmt, vl);
//        fflush(fp);
//        fclose(fp);
//    }
//}
//
//int videoRecordStart(const char *out_path, int width, int height){
//
//    yuv_width = width;
//    yuv_height = height;
//    y_length = width * height;
//    uv_length = width * height / 4;
//
//    //FFmpeg av_log() callback
//    av_log_set_callback(custom_log);
//
//    av_register_all();
//
//    avformat_network_init();
//
//    //output initialize
//    avformat_alloc_output_context2(&ofmt_ctx, NULL, "mp4", out_path);
//    //output encoder initialize
//    //函数的参数是一个解码器的ID，返回查找到的解码器（没有找到就返回NULL）。
//    pCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
//    if (!pCodec) {
//        LOGE("Can not find encoder!\n");
//        return -1;
//    }
//    pCodecCtx = avcodec_alloc_context3(pCodec);
//    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
//    pCodecCtx->width = width;
//    pCodecCtx->height = height;
//    pCodecCtx->time_base.num = 1;
//    pCodecCtx->time_base.den = 25;
//    pCodecCtx->bit_rate = 400000;
//    pCodecCtx->gop_size = 250;
//    /* Some formats want stream headers to be separate. */
//    if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
//        pCodecCtx->flags |= CODEC_FLAG_GLOBAL_HEADER;
//
//    //H264 codec param
//    //pCodecCtx->me_range = 16;
//    //pCodecCtx->max_qdiff = 4;
//    //pCodecCtx->qcompress = 0.6;
//    pCodecCtx->qmin = 10;
//    pCodecCtx->qmax = 51;
//    //Optional Param
//    pCodecCtx->max_b_frames = 1;
//    // Set H264 preset and tune
//    AVDictionary *param = 0;
////  av_dict_set(&param, "preset", "ultrafast", 0);
////  av_dict_set(&param, "tune", "zerolatency", 0);
//    av_opt_set(pCodecCtx->priv_data, "preset", "superfast", 0);
//    av_opt_set(pCodecCtx->priv_data, "tune", "zerolatency", 0);
//    //打开编码器
//    if (avcodec_open2(pCodecCtx, pCodec, &param) < 0) {
//        LOGE("Failed to open encoder!\n");
//        return -1;
//    }
//
//    //Add a new stream to output,should be called by the user before avformat_write_header() for muxing
//    video_st = avformat_new_stream(ofmt_ctx, pCodec);
//    if (video_st == NULL) {
//        return -1;
//    }
//    video_st->time_base.num = 1;
//    video_st->time_base.den = 25;
//    video_st->codec = pCodecCtx;
//    AVCodecParameters *par = avcodec_parameters_alloc();
//   int ret = avcodec_parameters_from_context(par, pCodecCtx);
//    if (ret < 0) {
//        LOGD("avcodec_parameters_from_context fail");
//        return ret;
//    }
//    ret = avcodec_parameters_copy(video_st->codecpar, par);
//    if (ret < 0) {
//        LOGD("avcodec_parameters_copy fail");
//        return ret;
//    }
//
//    //Open output URL,set before avformat_write_header() for muxing
//    if (avio_open(&ofmt_ctx->pb, out_path, AVIO_FLAG_READ_WRITE) < 0) {
//        LOGE("Failed to open output file!\n");
//        return -1;
//    }
//
//    //Write File Header
//    avformat_write_header(ofmt_ctx, NULL);
//
////    start_time = av_gettime();
//    return 0;
//}
//
//int videoRecording(uint8_t *data)  {
//    int ret;
//    int enc_got_frame = 0;
//    int i = 0;
//
//    // 为解码帧分配内存
//    pFrameYUV = av_frame_alloc();
//    uint8_t *out_buffer = (uint8_t *) av_malloc(
//            avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width,
//                               pCodecCtx->height));
//    avpicture_fill((AVPicture *) pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P,
//                   pCodecCtx->width, pCodecCtx->height);
//
//    //安卓摄像头数据为NV21格式，此处将其转换为YUV420P格式
////    memcpy(pFrameYUV->data[0], data, y_length);
////    for (i = 0; i < uv_length; i++) {
////        *(pFrameYUV->data[2] + i) = *(data + y_length + i * 2);
////        *(pFrameYUV->data[1] + i) = *(data + y_length + i * 2 + 1);
////    }
//
//    pFrameYUV->format = AV_PIX_FMT_YUV420P;
//    pFrameYUV->width = yuv_width;
//    pFrameYUV->height = yuv_height;
//  int  y_size = yuv_width * yuv_height;
//    pFrameYUV->data[0] = data;  //PCM Data
//    pFrameYUV->data[1] = (data + y_size * 5 / 4);      // U
//    pFrameYUV->data[2] = (data + y_size);  // V
//    enc_pkt.data = NULL;
//    enc_pkt.size = 0;
//    // 定义AVPacket对象后,请使用av_init_packet进行初始化
//    av_init_packet(&enc_pkt);
//
//    /** 编码一帧视频数据
//     * int avcodec_encode_video2(AVCodecContext *avctx, AVPacket *avpkt,
//     const AVFrame *frame, int *got_packet_ptr);
//
//     该函数每个参数的含义在注释里面已经写的很清楚了，在这里用中文简述一下：
//     avctx：编码器的AVCodecContext。
//     avpkt：编码输出的AVPacket。
//     frame：编码输入的AVFrame。
//     got_packet_ptr：成功编码一个AVPacket的时候设置为1。
//     函数返回0代表编码成功。
//
//     */
//    ret = avcodec_encode_video2(pCodecCtx, &enc_pkt, pFrameYUV, &enc_got_frame);
//    av_frame_free(&pFrameYUV);
//
//    if (enc_got_frame == 1) {
//        LOGI("Succeed to encode frame: %5d\tsize:%5d\n", framecnt,
//             enc_pkt.size);
//        framecnt++;
//        //标识该AVPacket所属的视频/音频流。
//        enc_pkt.stream_index = video_st->index; //标识该视频/音频流
//
//        //Write PTS
//        AVRational time_base = ofmt_ctx->streams[0]->time_base; //{ 1, 1000 };
//        AVRational r_framerate1 = { 60, 2 };    //{ 50, 2 };
//        AVRational time_base_q = { 1, AV_TIME_BASE };
//        //Duration between 2 frames (us)
//        int64_t calc_duration = (double) (AV_TIME_BASE)
//                                * (1 / av_q2d(r_framerate1));   //内部时间戳
//        //Parameters
//        //enc_pkt.pts = (double)(framecnt*calc_duration)*(double)(av_q2d(time_base_q)) / (double)(av_q2d(time_base));
//        enc_pkt.pts = av_rescale_q(framecnt * calc_duration, time_base_q,
//                                   time_base);
//        enc_pkt.dts = enc_pkt.pts;
//        enc_pkt.duration = av_rescale_q(calc_duration, time_base_q, time_base); //(double)(calc_duration)*(double)(av_q2d(time_base_q)) / (double)(av_q2d(time_base));
//        enc_pkt.pos = -1;
//
//        //Delay
//        int64_t pts_time = av_rescale_q(enc_pkt.dts, time_base, time_base_q);
////        int64_t now_time = av_gettime() - start_time;
////        if (pts_time > now_time)
////            av_usleep(pts_time - now_time);
//
//        ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
//        LOGD("av_interleaved_write_frame ret=%d",ret);
//        av_free_packet(&enc_pkt);
//    }
////  output(ofmt_ctx);
//    return 0;
//}
//
//int flush(){
//    int ret;
//    int got_frame;
//    AVPacket enc_pkt;
//    if (!(ofmt_ctx->streams[0]->codec->codec->capabilities & CODEC_CAP_DELAY))
//        return 0;
//    while (1) {
//        enc_pkt.data = NULL;
//        enc_pkt.size = 0;
//        av_init_packet(&enc_pkt);
//        ret = avcodec_encode_video2(ofmt_ctx->streams[0]->codec, &enc_pkt, NULL,
//                                    &got_frame);
//        if (ret < 0)
//            break;
//        if (!got_frame) {
//            ret = 0;
//            break;
//        }
//        LOGI("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n",
//             enc_pkt.size);
//
//        //Write PTS
//        AVRational time_base = ofmt_ctx->streams[0]->time_base; //{ 1, 1000 };
//        AVRational r_framerate1 = { 60, 2 };
//        AVRational time_base_q = { 1, AV_TIME_BASE };
//        //Duration between 2 frames (us)
//        int64_t calc_duration = (double) (AV_TIME_BASE)
//                                * (1 / av_q2d(r_framerate1));   //内部时间戳
//        //Parameters
//        enc_pkt.pts = av_rescale_q(framecnt * calc_duration, time_base_q,
//                                   time_base);
//        enc_pkt.dts = enc_pkt.pts;
//        enc_pkt.duration = av_rescale_q(calc_duration, time_base_q, time_base);
//
//        //转换PTS/DTS（Convert PTS/DTS）
//        enc_pkt.pos = -1;
//        framecnt++;
//        ofmt_ctx->duration = enc_pkt.duration * framecnt;
//
//        /* mux encoded frame */
//        ret = av_interleaved_write_frame(ofmt_ctx, &enc_pkt);
//        if (ret < 0)
//            break;
//    }
//    //Write file trailer
//    av_write_trailer(ofmt_ctx);
//    return 0;
//}
//
//int videoRecordEnd() {
//    flush();
//    if (video_st)
//        avcodec_close(video_st->codec);
//    avio_close(ofmt_ctx->pb);
//    avformat_free_context(ofmt_ctx);
//    return 0;
//}