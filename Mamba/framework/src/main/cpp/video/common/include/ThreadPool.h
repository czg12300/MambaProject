//
// Created by jakechen on 2017/1/11.
//
#ifndef TVPLAYER_THREADPOOL_WORKER
#define TVPLAYER_THREADPOOL_WORKER
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>
#include "Log.h"



/*
*线程池里所有运行和等待的任务都是一个CThread_worker
*由于所有任务都在链表里，所以是一个链表结构
*/
typedef struct worker {
    /*回调函数，任务运行时会调用此函数，注意也可声明成其它形式*/
    void *(*process)(void *arg);

    void *arg;/*回调函数的参数*/
    struct worker *next;

} CThread_worker;


/*线程池结构*/
typedef struct {
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_ready;

    /*链表结构，线程池中所有等待任务*/
    CThread_worker *queue_head;

    /*是否销毁线程池*/
    int shutdown;
    pthread_t *threadid;
    /*线程池中允许的活动线程数目*/
    int max_thread_num;
    /*当前等待队列的任务数目*/
    int cur_queue_size;

} CThread_pool;


void pool_init(int max_thread_num);

/*向线程池中加入任务*/
int pool_add_worker(void *(*process)(void *arg), void *arg);

/*销毁线程池，等待队列中的任务不会再被执行，但是正在运行的线程会一直
把任务运行完后再退出*/
int pool_destroy();

void *thread_routine(void *arg);

#endif