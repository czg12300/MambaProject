//
// Created by walljiang on 2017/04/07.
//

#ifndef GITXIUGE_AUDIOMERGE_H
#define GITXIUGE_AUDIOMERGE_H

class OptAudioCallback : public ProgressCallback {
private:
    HandleProgressCallback *callback;
public:
    OptAudioCallback(HandleProgressCallback *callback);

    void onProgress(int progress, int total);

    void onFinish(int progress, int total);
};

#endif //GITXIUGE_AUDIOMERGE_H
extern "C" {
#include "libavformat/avformat.h"
#include <libavfilter/avfilter.h>
#include <libavutil/audio_fifo.h>
#include <libavutil/opt.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
};

#include "Log.h"

int audioRate(const char *srcFile1, const char *outFile, float rate, ProgressCallback *callback);
int audioRateFast(const char *srcFile1, const char *outFile, float rate, ProgressCallback *callback);

int
audioRate(const char *srcFile1, const char *outFile, float rate, HandleProgressCallback *callback);