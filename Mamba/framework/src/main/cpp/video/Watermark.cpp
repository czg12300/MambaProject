//
// Created by jakechen on 2017/3/10.
//

#include "Watermark.h"
static AVFilterContext *buffersink_ctx;
static AVFilterContext *buffersrc_ctx;
static AVFilterGraph *filter_graph;


static int
init_filters(AVCodecContext *decodecCtx, AVStream *avStream, const char *filters_descr) {
    char args[512];
    int ret = 0;
    AVFilter *buffersrc = avfilter_get_by_name("buffer");
    AVFilter *buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();
    AVRational time_base = avStream->time_base;
    enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE};
    filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             decodecCtx->width, decodecCtx->height, decodecCtx->pix_fmt,
             time_base.num, time_base.den,
             decodecCtx->sample_aspect_ratio.num, decodecCtx->sample_aspect_ratio.den);

    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
                                       args, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
        goto end;
    }

    /* buffer video sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                                       NULL, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
        goto end;
    }

    ret = av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts,
                              AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
        goto end;
    }

    /*
     * Set the endpoints for the filter graph. The filter_graph will
     * be linked to the graph described by filters_descr.
     */

    /*
     * The buffer source output must be connected to the input pad of
     * the first filter described by filters_descr; since the first
     * filter input label is not specified, it is set to "in" by
     * default.
     */
    outputs->name = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    /*
     * The buffer sink input must be connected to the output pad of
     * the last filter described by filters_descr; since the last
     * filter output label is not specified, it is set to "out" by
     * default.
     */
    inputs->name = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = NULL;
    if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr,
                                        &inputs, &outputs, NULL)) < 0)
        goto end;

    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        goto end;
    end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
}

WatermarkVideoOptHandler::WatermarkVideoOptHandler(string watermarkCommand) {
    this->watermarkCommand = watermarkCommand;
    filt_frame = av_frame_alloc();
}

int WatermarkVideoOptHandler::changeFrameRate(int frameRate) {
    return frameRate;
}
int WatermarkVideoOptHandler::init(AVFormatContext *ifmt_ctx, AVCodecContext *decodecCtx,
                                   int video_index,int frameCount) {
    LOGD("WatermarkVideoOptHandler::init");
    return init_filters(decodecCtx, ifmt_ctx->streams[video_index], this->watermarkCommand.c_str());
}

AVFrame *WatermarkVideoOptHandler::optAvFrame(AVFrame *frame, int frameIndex) {
    LOGD("WatermarkVideoOptHandler::optAvFram");
    frame->pts = av_frame_get_best_effort_timestamp(frame);
    LOGD("WatermarkVideoOptHandler::optAvFram");
    /* push the decoded frame into the filtergraph */
    if (av_buffersrc_add_frame_flags(buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_KEEP_REF) <
        0) {
        LOGE(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
        return NULL;
    }
    /* pull filtered frames from the filtergraph */
    LOGD("while (1)");
    int ret = av_buffersink_get_frame(buffersink_ctx, this->filt_frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF || ret < 0) {
        LOGD("av_buffersink_get_frame fail");
        return NULL;
    }
    return this->filt_frame;
}

void WatermarkVideoOptHandler::releaseInEnd() {
    av_frame_free(&this->filt_frame);
    avfilter_graph_free(&filter_graph);
}

AllowEncodeAndWriteFrameState WatermarkVideoOptHandler::allowEncodeAndWriteFrame(AVStream *inStream, AVPacket pkt,int frameIndex) {
    return STATE_ALLOW;
}

void
WatermarkVideoOptHandler::changeTimebase(AVStream *inStream, AVStream *outStream, AVPacket pkt) {
    //do nothing
}

int addWatermark(const char *watermarkCommand, const char *srcFile, const char *out_file) {
    VideoOptHandler *handler = new WatermarkVideoOptHandler(watermarkCommand);
    VideoRecodeFactory *factory = new VideoRecodeFactory(srcFile, out_file);
    int ret = factory->demuxerAudio();
    LOGD("demuxerAudio");
    if (ret < 0) {
        return ret;
    }
    ret = factory->optVideo(handler);
    LOGD("optVideo");
    if (ret < 0) {
        return ret;
    }
    ret = factory->muxer(factory->outOptH264, factory->outAac, factory->outFile);
    LOGD("muxer");
    delete (handler);
    delete (factory);
    return ret;

}
