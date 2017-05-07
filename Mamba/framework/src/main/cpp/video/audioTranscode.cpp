//
// Created by walljiang on 2017/04/13.
//
#include "audioTranscode.h"
SwrContext* pSwrCtx = NULL;
int64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
int out_sample_rate = 44100;
void initSwr(int64_t in_ch_layout, enum AVSampleFormat  in_sample_fmt, int  in_sample_rate)
{
    if (in_ch_layout != out_ch_layout
        || in_sample_rate != out_sample_rate
        || in_sample_fmt != out_sample_fmt)
    {
        if ( NULL == pSwrCtx )
        {
            pSwrCtx = swr_alloc();
        }
//#if LIBSWRESAMPLE_VERSION_MINOR >= 1	// 根据版本不同，选用适当函数
//        av_opt_set_int(pSwrCtx, "ich", av_get_channel_layout_nb_channels(in_ch_layout), 0);
//		av_opt_set_int(pSwrCtx, "och", av_get_channel_layout_nb_channels(out_ch_layout), 0);
//		av_opt_set_int(pSwrCtx, "in_sample_rate",  in_sample_rate, 0);
//		av_opt_set_int(pSwrCtx, "out_sample_rate",  out_sample_rate, 0);
//		av_opt_set_sample_fmt(pSwrCtx, "in_sample_fmt", in_sample_fmt, 0);
//		av_opt_set_sample_fmt(pSwrCtx, "out_sample_fmt", out_sample_fmt, 0);
//#else
        pSwrCtx = swr_alloc_set_opts(pSwrCtx,out_ch_layout,out_sample_fmt,out_sample_rate,in_ch_layout,in_sample_fmt,in_sample_rate,0, NULL);
//#endif
        swr_init(pSwrCtx);
    }
}

void deInitSwr(){
    if(pSwrCtx != NULL){
        swr_close(pSwrCtx);
        swr_free(&pSwrCtx);
        pSwrCtx = NULL;
    }
}

//setup_array函数摘自ffmpeg例程
void setup_array(uint8_t* out[SWR_CH_MAX], AVFrame* in_frame, int format, int samples)
{
    if (av_sample_fmt_is_planar((AVSampleFormat)format))
    {
        int i;
        int plane_size = av_get_bytes_per_sample((AVSampleFormat)(format & 0xFF)) * samples;
        format &= 0xFF;
        //从decoder出来的frame中的data数据不是连续分布的，所以不能这样写：in_frame->data[0]+i*plane_size;
        for (i = 0; i < in_frame->channels; i++)
        {
            out[i] = in_frame->data[i];
        }
    }
    else
    {
        LOGI("setup_array ELSE");
        out[0] = in_frame->data[0];
    }
}

int TransSample(AVFrame *in_frame, AVFrame *out_frame,enum AVSampleFormat src_sample_fmt)
{
    int ret;
    int max_dst_nb_samples = 4096;
    //int64_t dst_nb_samples;
    int64_t src_nb_samples = in_frame->nb_samples;
    out_frame->pts = in_frame->pts;
    uint8_t* paudiobuf;
    int decode_size, input_size, len;
    if (pSwrCtx != NULL)
    {
        out_frame->nb_samples = av_rescale_rnd(swr_get_delay(pSwrCtx, out_sample_rate) + src_nb_samples,out_sample_rate, out_sample_rate, AV_ROUND_UP);
        ret = av_samples_alloc(out_frame->data, &out_frame->linesize[0],
                               av_get_channel_layout_nb_channels(out_ch_layout), out_frame->nb_samples, out_sample_fmt, 0);
        if (ret < 0)
        {
            LOGE( "[%s.%d %s() Could not allocate samples Buffer\n", __FILE__, __LINE__, __FUNCTION__);
            return -1;
        }
        max_dst_nb_samples = out_frame->nb_samples;
        //输入也可能是分平面的，所以要做如下处理
        uint8_t* m_ain[SWR_CH_MAX];
        setup_array(m_ain, in_frame, src_sample_fmt, src_nb_samples);
        //注意这里，out_count和in_count是samples单位，不是byte
        //所以这样av_get_bytes_per_sample(in_fmt_ctx->streams[audio_index]->codec->sample_fmt) * src_nb_samples是错的
        len = swr_convert(pSwrCtx, out_frame->data, out_frame->nb_samples,
                          (const uint8_t**)m_ain, src_nb_samples);
        if (len < 0)
        {
            char errmsg[BUF_SIZE_1K];
            av_strerror(len, errmsg, sizeof(errmsg));
            LOGE( "[%s:%d] swr_convert!(%d)(%s)", __FILE__, __LINE__, len, errmsg);
            return -1;
        }
    }
    else
    {
        LOGI("pSwrCtx with out init!\n");
        return -1;
    }
    return 0;
}