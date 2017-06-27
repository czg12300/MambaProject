//
// Created by jakechen on 2017/6/1.
//
#ifndef SG_THREADEXECUTOR_H
#define SG_THREADEXECUTOR_H
#include "FFmpegBase.h"
namespace video{
    class ThreadExecutor {
    public:
        int pool_add_worker(void *(*process)(void *arg), void *arg);
    };

}


#endif
