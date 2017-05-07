//
// Created by walljiang on 2017/04/07.
//

#include <audioMix.h>
#include "audioVolumn.h"

int audioVolumn(const char *srcFile1, const char *outFile,float vol1){
    LOGI("audioVolumn，srcFile:%s,outFile:%s,vol1:%f",srcFile1,outFile,vol1);
    int ret = audio_vol_change(srcFile1, outFile, 1.0*vol1);
    if (ret < 0) {
        LOGE("audio_vol_change error");
        return ret;
    }
    return 0;
}