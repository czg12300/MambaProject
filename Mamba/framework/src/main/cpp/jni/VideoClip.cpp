//
// Created by jakechen on 2016/12/30.
//


#include "VideoClip.h"

JNI(void, init)(JNIEnv *env, jclass type) {
    pool_init(10);
}

JNI(void, destroy)(JNIEnv *env, jclass type) {
    pool_destroy();
}

JNI(jstring, sayHello)(JNIEnv *env, jclass type, jstring name_) {
    string name = env->GetStringUTFChars(name_, NULL);
    string jni = name + ",this is come from jni";
    env->ReleaseStringUTFChars(name_, name.c_str());
    return env->NewStringUTF(jni.c_str());
}

JNI(void, helloJni)(JNIEnv *env, jclass type, jstring name_) {
    const string name = env->GetStringUTFChars(name_, 0);
    LOGD("this is jni log");

    env->ReleaseStringUTFChars(name_, name.c_str());
}

JNI(jint, demuxingKeyFrame)(JNIEnv *env, jclass type, jstring filePath_, jstring picPath_) {
//JNI(jstring, demuxingKeyFrame)(JNIEnv *env, jclass type, jstring filePath_, jstring picPath_) {
    const char *filePath = env->GetStringUTFChars(filePath_, 0);
    const char *picPath = env->GetStringUTFChars(picPath_, 0);
    int result = demuxing_key_frame(filePath, picPath);
    env->ReleaseStringUTFChars(filePath_, filePath);
    env->ReleaseStringUTFChars(picPath_, picPath);
    return result;
}

JNI(jint, demuxing)(JNIEnv *env, jclass type, jstring filePath_, jstring h264_, jstring aac_) {
    const char *filePath = env->GetStringUTFChars(filePath_, 0);
    const char *h264 = env->GetStringUTFChars(h264_, 0);
    const char *aac = env->GetStringUTFChars(aac_, 0);
    int result = demuxing(filePath, h264, aac);
    env->ReleaseStringUTFChars(filePath_, filePath);
    env->ReleaseStringUTFChars(h264_, h264);
    env->ReleaseStringUTFChars(aac_, aac);
    return result;
}

JNI(jint, rotateVideo)(JNIEnv *env, jclass type, jstring srcFile_,
                       jstring outFile_, jstring rotate_) {
    const char *in = env->GetStringUTFChars(srcFile_, 0);
    const char *out = env->GetStringUTFChars(outFile_, 0);
    const char *rotateC = env->GetStringUTFChars(rotate_, 0);
    int result = rotate(in, out, rotateC);
    env->ReleaseStringUTFChars(srcFile_, in);
    env->ReleaseStringUTFChars(outFile_, out);
    env->ReleaseStringUTFChars(rotate_, rotateC);
    return result;
}

JNI(jint, fastVideoSpeed)(JNIEnv *env, jclass type, jstring srcFile_,
                          jstring outFile_, jint rate_, jobject listener_) {
    const char *in = env->GetStringUTFChars(srcFile_, 0);
    const char *out = env->GetStringUTFChars(outFile_, 0);
    jclass classId = env->FindClass("com/kugou/xiuge/video/HandleProgressListener");
    jmethodID methodId = env->GetMethodID(classId, "onHandle", "(II)V");
    jmethodID methodSuccess = env->GetMethodID(classId, "onSuccess", "(Ljava/lang/String;)V");
    HandleProgressCallback *callback = new HandleProgressCallback(env, listener_, methodId);
    int result = fastVideo(in, out, rate_, callback);
    if (result >= 0) {
        env->CallVoidMethod(listener_, methodSuccess, env->NewStringUTF(out));
    }
    delete (callback);
    env->DeleteLocalRef(classId);
    env->DeleteLocalRef(listener_);
    env->ReleaseStringUTFChars(srcFile_, in);
    env->ReleaseStringUTFChars(outFile_, out);
    string resultStr = result > -1 ? "fastVideoSpeed success" : "fastVideoSpeed fail";
    return result;
}

JNI(jint, slowVideoSpeed)(JNIEnv *env, jclass type, jstring srcFile_,
                          jstring outFile_, jint rate_, jobject listener_) {
    const char *in = env->GetStringUTFChars(srcFile_, 0);
    const char *out = env->GetStringUTFChars(outFile_, 0);
    jclass classId = env->FindClass("com/kugou/xiuge/video/HandleProgressListener");
    jmethodID methodId = env->GetMethodID(classId, "onHandle", "(II)V");
    jmethodID methodSuccess = env->GetMethodID(classId, "onSuccess", "(Ljava/lang/String;)V");
    HandleProgressCallback *callback = new HandleProgressCallback(env, listener_, methodId);
    int result = slowVideo(in, out, rate_, callback);
    if (result >= 0) {
        env->CallVoidMethod(listener_, methodSuccess, env->NewStringUTF(out));
    }
    delete (callback);
    env->DeleteLocalRef(classId);
    env->DeleteLocalRef(listener_);
    env->ReleaseStringUTFChars(srcFile_, in);
    env->ReleaseStringUTFChars(outFile_, out);
    return result;
}

JNI(jint, muxing)(JNIEnv *env, jclass type, jstring h264_, jstring audio_, jstring mp4_) {
    const char *h264 = env->GetStringUTFChars(h264_, 0);
    const char *audio = env->GetStringUTFChars(audio_, 0);
    const char *mp4 = env->GetStringUTFChars(mp4_, 0);
    int result = muxing(h264, audio, mp4);
    env->ReleaseStringUTFChars(h264_, h264);
    env->ReleaseStringUTFChars(audio_, audio);
    env->ReleaseStringUTFChars(mp4_, mp4);
    return result;
}

JNI(jint, muxingVideo)(JNIEnv *env, jclass type, jstring h264_, jstring audio_, jstring mp4_) {
    const char *h264 = env->GetStringUTFChars(h264_, 0);
    const char *audio = env->GetStringUTFChars(audio_, 0);
    const char *mp4 = env->GetStringUTFChars(mp4_, 0);
    int result = muxingVideoFile(h264, audio, mp4);
    env->ReleaseStringUTFChars(h264_, h264);
    env->ReleaseStringUTFChars(audio_, audio);
    env->ReleaseStringUTFChars(mp4_, mp4);
    return result;
}

JNI(jint, cutVideo)(JNIEnv *env, jclass type, jdouble start, jdouble end, jstring inFile,
                    jstring outFile) {
    double startTime = start;
    double endTime = end;
    const char *in = env->GetStringUTFChars(inFile, 0);
    const char *out = env->GetStringUTFChars(outFile, 0);
    int result = cut_video(startTime, endTime, in, out);
    env->ReleaseStringUTFChars(inFile, in);
    env->ReleaseStringUTFChars(outFile, out);
    return result;
}

JNI(jint, cutAudio)(JNIEnv *env, jclass type, jdouble start, jdouble end, jstring inFile,
                    jstring outFile) {
    double startTime = start;
    double endTime = end;
    const char *in = env->GetStringUTFChars(inFile, 0);
    const char *out = env->GetStringUTFChars(outFile, 0);
    int result = cut_audio(startTime, endTime, in, out);
    env->ReleaseStringUTFChars(inFile, in);
    env->ReleaseStringUTFChars(outFile, out);
    string resultStr = result > -1 ? "cut audio success" : "cut audio fail";
    return result;
}

JNI(jint, videoRecordStart)(JNIEnv *env, jclass type, jstring h264File, jint width,
                            jint height, jstring rotate_, jint frameRate, jlong bitRate) {
    const char *file = env->GetStringUTFChars(h264File, 0);
    const char *rotate = env->GetStringUTFChars(rotate_, 0);
    int ret = videoRecordStart1(file, width, height, rotate, frameRate, bitRate);
    env->ReleaseStringUTFChars(h264File, file);
    env->ReleaseStringUTFChars(rotate_, rotate);
    return ret;
}

JNI(jint, videoRecording)(JNIEnv *env, jclass type, jbyteArray data) {
    jbyte *jData = env->GetByteArrayElements(data, 0);
    int ret = videoRecording1((uint8_t *) jData);
    env->ReleaseByteArrayElements(data, jData, 0);
    return ret;
}

JNI(jint, videoRecordEnd)(JNIEnv *env, jclass type) {
    return videoRecordEnd1();
}

JNI(jint, addWatermark)(JNIEnv *env, jclass type, jstring watermarkCommand_,
                        jstring srcFile_, jstring outFile_) {
    const char *in = env->GetStringUTFChars(srcFile_, 0);
    const char *out = env->GetStringUTFChars(outFile_, 0);
    const char *png = env->GetStringUTFChars(watermarkCommand_, 0);
    int result = addWatermark(png, in, out);
    env->ReleaseStringUTFChars(srcFile_, in);
    env->ReleaseStringUTFChars(outFile_, out);
    env->ReleaseStringUTFChars(watermarkCommand_, png);
    return result;
}

JNI(jint, repeatVideoMoment)(JNIEnv *env, jclass type, jstring srcFile_,
                             jstring outFile_, jdouble second_) {
    const char *in = env->GetStringUTFChars(srcFile_, 0);
    const char *out = env->GetStringUTFChars(outFile_, 0);
    int result = repeatMoment(in, out, second_);
    env->ReleaseStringUTFChars(srcFile_, in);
    env->ReleaseStringUTFChars(outFile_, out);
    return result;
}

JNI(jint, relativeVideoMoment)(JNIEnv *env, jclass type, jstring srcFile_,
                               jstring outFile_, jdouble second_) {
    const char *in = env->GetStringUTFChars(srcFile_, 0);
    const char *out = env->GetStringUTFChars(outFile_, 0);
    int result = relativeMoment(in, out, second_);
    env->ReleaseStringUTFChars(srcFile_, in);
    env->ReleaseStringUTFChars(outFile_, out);
    return result;
}


JNI(jint, videoMerge)(JNIEnv *env, jclass type, jstring srcFile1, jstring srcFile2,
                      jstring outFile) {
    const char *in1 = env->GetStringUTFChars(srcFile1, 0);
    const char *in2 = env->GetStringUTFChars(srcFile2, 0);
    const char *out = env->GetStringUTFChars(outFile, 0);
    int result = videoMerge(in1, in2, out);
    env->ReleaseStringUTFChars(srcFile1, in1);
    env->ReleaseStringUTFChars(srcFile2, in2);
    env->ReleaseStringUTFChars(outFile, out);
    return result;
}

JNI(jint, reEncode)(JNIEnv *env, jclass type, jstring srcFile, jstring outFile) {
    const char *in = env->GetStringUTFChars(srcFile, 0);
    const char *out = env->GetStringUTFChars(outFile, 0);
    int result = reEncode(in, out);
    env->ReleaseStringUTFChars(srcFile, in);
    env->ReleaseStringUTFChars(outFile, out);
    return result;
}

JNI(jint, remuxing)(JNIEnv *env, jclass type, jstring srcFile_, jstring outFile_) {
    const char *in = env->GetStringUTFChars(srcFile_, 0);
    const char *out = env->GetStringUTFChars(outFile_, 0);
    int result = remuxer(in, out);
    env->ReleaseStringUTFChars(srcFile_, in);
    env->ReleaseStringUTFChars(outFile_, out);
    return result;
}


JNI(jint, audioRecordStart)(JNIEnv *env, jclass type, jstring outFile, jint channels, jint bitrate,
                            jint sample_rate) {
    const char *file = env->GetStringUTFChars(outFile, 0);
    int ret = audio_encode_init(file, channels, bitrate, sample_rate);
    env->ReleaseStringUTFChars(outFile, file);
    return ret;
}

JNI(jint, audioRecording)(JNIEnv *env, jclass type, jbyteArray data, jint in_buffer_size) {
    jbyte *jData = env->GetByteArrayElements(data, 0);
    int ret = audio_encoding((uint8_t *) jData, in_buffer_size);
    env->ReleaseByteArrayElements(data, jData, 0);
    return ret;
}

JNI(jint, audioRecordEnd)(JNIEnv *env, jclass type) {
    return audio_encode_end();
}

JNI(jint, reverse)(JNIEnv *env, jclass type, jstring srcFile, jstring outFile) {
    const char *_srcFile = env->GetStringUTFChars(srcFile, NULL);
    const char *_outFile = env->GetStringUTFChars(outFile, NULL);
    int ret = reverse(_srcFile, _outFile);
    env->ReleaseStringUTFChars(srcFile, _srcFile);
    env->ReleaseStringUTFChars(outFile, _outFile);
    return ret;
}

JNI(jint, audioVolumn)(JNIEnv *env, jclass type, jstring srcFile1, jstring outFile, jfloat vol1) {
    const char *_srcFile1 = env->GetStringUTFChars(srcFile1, NULL);
    const char *_outFile = env->GetStringUTFChars(outFile, NULL);
    float _vol1 = vol1;
    int ret = audioVolumn(_srcFile1, _outFile, _vol1);
    env->ReleaseStringUTFChars(srcFile1, _srcFile1);
    env->ReleaseStringUTFChars(outFile, _outFile);
    return ret;
}

JNI(jint, audioRate)(JNIEnv *env, jclass type, jstring srcFile, jstring outFile, jfloat rate,
                     jobject listener_) {
    const char *_srcFile = env->GetStringUTFChars(srcFile, NULL);
    const char *_outFile = env->GetStringUTFChars(outFile, NULL);
    float _rate = rate;
    jclass classId = env->FindClass("com/kugou/xiuge/video/HandleProgressListener");
    jmethodID methodId = env->GetMethodID(classId, "onHandle", "(II)V");
    jmethodID methodSuccess = env->GetMethodID(classId, "onSuccess", "(Ljava/lang/String;)V");
    HandleProgressCallback *callback = new HandleProgressCallback(env, listener_, methodId);
    int ret = audioRate(_srcFile, _outFile, _rate, callback);
    env->ReleaseStringUTFChars(srcFile, _srcFile);
    env->ReleaseStringUTFChars(outFile, _outFile);
    return ret;
}

JNI(jint, audioMix)(JNIEnv *env, jclass type, jstring srcFile1, jstring srcFile2,
                    jstring outFile, jfloat vol1, jfloat vol2) {
    const char *_srcFile1 = env->GetStringUTFChars(srcFile1, NULL);
    const char *_srcFile2 = env->GetStringUTFChars(srcFile2, NULL);
    const char *_outFile = env->GetStringUTFChars(outFile, NULL);
    float _vol1 = vol1;
    float _vol2 = vol2;
    int ret = audioMix(_srcFile1, _srcFile2, _outFile, _vol1, _vol2);
    env->ReleaseStringUTFChars(srcFile1, _srcFile1);
    env->ReleaseStringUTFChars(srcFile2, _srcFile2);
    env->ReleaseStringUTFChars(outFile, _outFile);
    return ret;
}

JNI(jint, audioDecode)(JNIEnv *env, jclass type, jstring srcFile, jstring outFile,
                       jint decodeType) {
    const char *_srcFile = env->GetStringUTFChars(srcFile, NULL);
    const char *_outFile = env->GetStringUTFChars(outFile, NULL);
    int _decodeType = decodeType;
    int ret = audioDecode(_srcFile, _outFile, _decodeType);
    env->ReleaseStringUTFChars(srcFile, _srcFile);
    env->ReleaseStringUTFChars(outFile, _outFile);
    return ret;
}

JNI(jint, videoMix)(JNIEnv *env, jclass type, jstring srcFile1, jstring srcFile2, jstring outFile,
                    jint mix1, jint mix2) {
    const char *_srcFile1 = env->GetStringUTFChars(srcFile1, NULL);
    const char *_srcFile2 = env->GetStringUTFChars(srcFile2, NULL);
    const char *_outFile = env->GetStringUTFChars(outFile, NULL);
    int _mix1 = mix1;
    int _mix2 = mix2;
    int ret = videoMix(_srcFile1, _srcFile2, _outFile, _mix1, _mix2);
    env->ReleaseStringUTFChars(srcFile1, _srcFile1);
    env->ReleaseStringUTFChars(srcFile2, _srcFile2);
    env->ReleaseStringUTFChars(outFile, _outFile);
    return ret;
}

JNI(jint, videoTranscode)(JNIEnv *env, jclass type, jstring srcFile1, jstring outFile) {
    const char *_srcFile = env->GetStringUTFChars(srcFile1, NULL);
    const char *_outFile = env->GetStringUTFChars(outFile, NULL);
    int ret = videoTranscode(_srcFile, _outFile);
    env->ReleaseStringUTFChars(srcFile1, _srcFile);
    env->ReleaseStringUTFChars(outFile, _outFile);
    return ret;
}

JNI(jint, videoFilter)(JNIEnv *env, jclass type, jstring srcFile1, jstring outFile,
                       jstring filterArgs) {
    const char *_srcFile = env->GetStringUTFChars(srcFile1, NULL);
    const char *_outFile = env->GetStringUTFChars(outFile, NULL);
    const char *_filterargs = env->GetStringUTFChars(filterArgs, NULL);
    int ret = videoFilter(_srcFile, _outFile, _filterargs);
    env->ReleaseStringUTFChars(srcFile1, _srcFile);
    env->ReleaseStringUTFChars(outFile, _outFile);
    env->ReleaseStringUTFChars(filterArgs, _filterargs);
    return ret;
}