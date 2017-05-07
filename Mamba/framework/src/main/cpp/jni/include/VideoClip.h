//
// Created by jakechen on 2016/12/30.
//


#include <jni.h>
#include <string>
#include <iostream>
#include "../../video/include/KeyFrame.h"
#include "../../video/include/Demuxer.h"
#include "../../video/include/Muxer.h"
#include "../../video/include/CutVideo.h"
#include "../../video/include/TimeMachine.h"
#include "../../video/include/Watermark.h"
#include "../../video/include/VideoRecord.h"
#include "../../video/include/VideoRecord1.h"
#include "../../video/include/VideoMerge.h"
#include "../../video/include/ReEncode.h"
#include "../../video/include/Remuxer.h"
#include "../../video/include/AudioRecord.h"
#include "../../video/include/RepeatMoment.h"
#include "../../video/include/HandleProgressCallback.h"
#include "../../video/include/VideoRotate.h"
#include "../../video/include/reverse.h"
#include "../../video/include/audioVolumn.h"
#include "../../video/include/audioRate.h"
#include "../../video/include/audioMix.h"
#include "../../video/include/audioDecode.h"
#include "../../video/include/ThreadPool.h"
#include "../../video/include/videoMix.h"
#include "../../video/include/videotranscode.h"
#include "../../video/include/videoFilter.h"

using namespace std;
extern "C"
{
#define JNI(rettype, name) JNIEXPORT rettype JNICALL Java_com_kugou_xiuge_video_VideoClipJni_##name
JNI(void, init)(JNIEnv *env, jclass type);
JNI(void, destroy)(JNIEnv *env, jclass type);
JNI(jint, videoMerge)(JNIEnv *env, jclass type, jstring srcFile1, jstring srcFile2,
                      jstring outFile);
JNI(jint, remuxing)(JNIEnv *env, jclass type, jstring srcFile_, jstring outFile_);
JNI(jint, demuxing)(JNIEnv *env, jclass type, jstring filePath_, jstring h264_, jstring aac_);
JNI(jint, rotateVideo)(JNIEnv *env, jclass type, jstring srcFile_,
                       jstring outFile_, jstring rotate_);
JNI(jint, fastVideoSpeed)(JNIEnv *env, jclass type, jstring srcFile_,
                          jstring outFile_, jint rate_, jobject listener_);
JNI(jint, slowVideoSpeed)(JNIEnv *env, jclass type, jstring srcFile_,
                          jstring outFile_, jint rate_, jobject listener_);
JNI(jint, repeatVideoMoment)(JNIEnv *env, jclass type, jstring srcFile_,
                             jstring outFile_, jdouble second_);
JNI(jint, relativeVideoMoment)(JNIEnv *env, jclass type, jstring srcFile_,
                               jstring outFile_, jdouble second_);
JNI(jint, muxing)(JNIEnv *env, jclass type, jstring h264_, jstring audio_, jstring mp4_);
JNI(jint, muxingVideo)(JNIEnv *env, jclass type, jstring h264_, jstring audio_, jstring mp4_);
JNI(jint, demuxingKeyFrame)(JNIEnv *env, jclass type, jstring filePath_, jstring picPath);
JNI(jstring, sayHello)(JNIEnv *env, jclass type, jstring name_);
JNI(void, helloJni)(JNIEnv *env, jclass type, jstring name_);
JNI(jint, cutVideo)(JNIEnv *env, jclass type, jdouble start, jdouble end, jstring inFile,
                    jstring outFile);
JNI(jint, cutAudio)(JNIEnv *env, jclass type, jdouble start, jdouble end, jstring inFile,
                    jstring outFile);

JNI(jint, videoRecordStart)(JNIEnv *env, jclass type, jstring h264File, jint width,
                            jint height, jstring rotate_, jint frameRate, jlong bitRate);
JNI(jint, videoRecording)(JNIEnv *env, jclass type, jbyteArray data);
JNI(jint, videoRecordEnd)(JNIEnv *env, jclass type);
JNI(jint, audioRecordStart)(JNIEnv *env, jclass type, jstring outFile, jint channels, jint bitrate,
                            jint sample_rate);
JNI(jint, audioRecording)(JNIEnv *env, jclass type, jbyteArray data, jint in_buffer_size);
JNI(jint, audioRecordEnd)(JNIEnv *env, jclass type);
JNI(jint, addWatermark)(JNIEnv *env, jclass type, jstring watermarkCommand_,
                        jstring srcFile_, jstring outFile_);
JNI(jint, reEncode)(JNIEnv *env, jclass type, jstring srcFile, jstring outFile);
JNI(jint, reverse)(JNIEnv *env, jclass type, jstring srcFile, jstring outFile);
JNI(jint, audioVolumn)(JNIEnv *env, jclass type, jstring srcFile1, jstring outFile, jfloat vol1);
JNI(jint, audioRate)(JNIEnv *env, jclass type, jstring srcFile, jstring outFile, jfloat rate,
                     jobject listener_);
JNI(jint, audioMix)(JNIEnv *env, jclass type, jstring srcFile1, jstring srcFile2, jstring outFile,
                    jfloat vol1, jfloat vol2);
JNI(jint, audioDecode)(JNIEnv *env, jclass type, jstring srcFile, jstring outFile, jint decodeType);
JNI(jint, videoMix)(JNIEnv *env, jclass type, jstring srcFile1, jstring srcFile2, jstring outFile,
                    jint mix1, jint mix2);
JNI(jint, videoTranscode)(JNIEnv *env, jclass type, jstring srcFile1, jstring outFile);
JNI(jint, videoFilter)(JNIEnv *env, jclass type, jstring srcFile1, jstring outFile,
                       jstring filterArgs);
}