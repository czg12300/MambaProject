//
// Created by jakechen on 2017/7/20.
//

#include "MediaExtractorJni.h"

static MediaExtractor *extractor = NULL;

JNI(void, nativeSetup)(JNIEnv *env, jobject obj) {

//    if (extractor == NULL) {
    extractor = new MediaExtractor();
//    }
}


JNI(void, setDataSource)(JNIEnv *env, jobject obj, jstring file_) {
    const char *file = env->GetStringUTFChars(file_, 0);
    int ret = extractor->_setDataSource(file);
    if (ret < 0) {
        extractor->_release();
        jclass lClass = env->FindClass("java/io/IOException");
        if (lClass != NULL) {
            env->ThrowNew(lClass, "open file fail,please make sure file is media file");
        }
        env->DeleteLocalRef(lClass);
    }
    env->ReleaseStringUTFChars(file_, file);
}

JNI(void, seekTo)(JNIEnv *env, jobject obj, jint timeUs, jint mode) {
    extractor->_seekTo(timeUs, mode);
}

JNI(jint, getTrackCount)(JNIEnv *env, jobject obj) {
    return extractor->_getTrackCount();
}

JNI(jobject, nativeGetTrackFormat)(JNIEnv *env, jobject obj, jint index) {
    MediaFormat *format = extractor->_getTrackFormat(index);
    MediaFormatKey *key = new MediaFormatKey;
    jclass fClass = env->FindClass("com/framework/ndk/media/FfmpegMediaFormat");
    jmethodID fctorID = env->GetMethodID(fClass, "<init>", "()V");
    jmethodID fId = env->GetMethodID(fClass, "setInteger", "(Ljava/lang/String;I)V");
    jobject jformat = env->NewObject(fClass, fctorID);
    env->CallVoidMethod(jformat, fId, env->NewStringUTF(key->CHANNELS), format->channels);
    env->CallVoidMethod(jformat, fId, env->NewStringUTF(key->CODEC_TYPE), format->codec_type);
    env->CallVoidMethod(jformat, fId, env->NewStringUTF(key->CODEC_ID), format->codec_id);
    env->CallVoidMethod(jformat, fId, env->NewStringUTF(key->CHANNEL_LAYOUT),
                        format->channel_layout);
    env->CallVoidMethod(jformat, fId, env->NewStringUTF(key->WIDTH), format->width);
    env->CallVoidMethod(jformat, fId, env->NewStringUTF(key->HEIGHT), format->height);
    env->CallVoidMethod(jformat, fId, env->NewStringUTF(key->BIT_RATE), format->bit_rate);
    env->CallVoidMethod(jformat, fId, env->NewStringUTF(key->FRAME_RATE), format->frame_rate);
    env->CallVoidMethod(jformat, fId, env->NewStringUTF(key->SAMPLE_RATE), format->sample_rate);
    delete (format);
    delete (key);
    return jformat;
}

JNI(jint, getTimestamp)(JNIEnv *env, jobject obj) {
    return extractor->_getTimestamp();
}

JNI(jbyteArray, readSampleData)(JNIEnv *env, jobject obj) {
    int len = 0;
    const uint8_t *temp = extractor->_readSampleData(&len);
//    uint8_t *temp1 = new uint8_t;
//    memccpy(temp1, temp, 0, len);
    jbyte *data = (jbyte *) temp;
    jbyteArray jarrRV = env->NewByteArray(len);
    LOGD("readSampleData jni len %d", len);
    env->SetByteArrayRegion(jarrRV, 0, len, data);
    return jarrRV;
}

JNI(jint, readSampleData1)(JNIEnv *env, jobject obj, jbyteArray data) {
    int len = 0;
    const uint8_t *temp = extractor->_readSampleData(&len);

    jbyte *da = env->GetByteArrayElements(data, 0);
    *da = (jbyte) *temp;
    env->ReleaseByteArrayElements(data, da, 0);
    return len;
}

JNI(jint, getSampleDataSize)(JNIEnv *env, jobject obj) {
    return extractor->_getSampleDataSize();
}
//    AVPacket packet;
//    jbyte *data = env->GetByteArrayElements(data_, 0);

//    jclass jc = env->GetObjectClass(byteBuffer);
//    jfieldID fid = env->GetFieldID(jc, "data", "Ljava/nio/ByteBuffer;");
//    jobject bar = env->GetObjectField(obj, fid);
//    jbyte *data = (jbyte *) env->GetDirectBufferAddress(bar);
//    int ret = extractor->_readSampleData((uint8_t *) data);
//    jmethodID jm = env->GetMethodID(jc, "put", "([B)Ljava/nio/ByteBuffer");
//    uint8_t *temp = packet.data;
//    jbyte *data = (jbyte *) *temp;
//    env->CallObjectMethod(byteBuffer, jm, data);
//    av_packet_unref(&packet);
//    env->DeleteLocalRef(byteBuffer);
//    env->ReleaseByteArrayElements(data_, data, 0);
//    return ret;
//}

JNI(void, selectTrack)(JNIEnv *env, jobject obj, jint index) {
    extractor->_selectTrack(index);
}

JNI(jint, getTrackIndex)(JNIEnv *env, jobject obj) {
    return extractor->_getTrackIndex();
}

JNI(jint, getDuration)(JNIEnv *env, jobject obj) {
    return extractor->_getDuration();
}

JNI(void, release)(JNIEnv *env, jobject obj) {
    extractor->_release();
    delete (extractor);
    extractor = NULL;
}