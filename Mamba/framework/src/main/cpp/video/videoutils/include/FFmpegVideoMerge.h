//
// Created by jakechen on 2017/5/12.
//
#include "FfmpegVideoUtilsBase.h"
#include <queue>
extern "C" {
#include "libavformat/avformat.h"
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>

}
namespace video {
    int merge(queue<string> srcFiles, const char *outFile);
}