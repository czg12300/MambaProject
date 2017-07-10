//
// Created by jakechen on 2017/7/7.
//

#include "SoundTouchJni.h"

static SoundTouch *mSoundTouch = NULL;

JNI(void, setupAudioParameters)(JNIEnv *env, jclass type, jint sampleRate, jint channels) {
    if(mSoundTouch!=NULL){
        mSoundTouch->setSampleRate(sampleRate);
        mSoundTouch->setChannels(channels);
    }
}

JNI(void, getSampleByBytes)(JNIEnv *env, jclass type, jbyteArray dest_, jint len) {
    jbyte *data = env->GetByteArrayElements(dest_, 0);
    if(mSoundTouch!=NULL){
        mSoundTouch->receiveSamples((SAMPLETYPE *) data, len);
    }
    env->ReleaseByteArrayElements(dest_, data, 0);

}

JNI(void, putSampleByBytes)(JNIEnv *env, jclass type, jbyteArray src_, jint len) {
    jbyte *data = env->GetByteArrayElements(src_, 0);
    if(mSoundTouch!=NULL){
        mSoundTouch->putSamples((SAMPLETYPE *) data, len);
    }
    env->ReleaseByteArrayElements(src_, data, 0);
}

JNI(void, setTempo)(JNIEnv *env, jclass type, jdouble newTempo) {
    if(mSoundTouch!=NULL){
        mSoundTouch->setTempo(newTempo);
    }
}
JNI(void, setTempoChange)(JNIEnv *env, jclass type, jdouble newTempo) {
    if(mSoundTouch!=NULL){
        mSoundTouch->setTempoChange(newTempo);
    }
}

JNI(void, setPitch)(JNIEnv *env, jclass type, jdouble newPitch) {
    if(mSoundTouch!=NULL){
        mSoundTouch->setPitch(newPitch);
    }
}
JNI(void, setPitchOctaves)(JNIEnv *env, jclass type, jdouble newPitch) {
    if(mSoundTouch!=NULL){
        mSoundTouch->setPitchOctaves(newPitch);
    }
}
JNI(void, setPitchSemiTones)(JNIEnv *env, jclass type, jint newPitch) {
    if(mSoundTouch!=NULL){
        mSoundTouch->setPitchSemiTones(newPitch);
    }
}

JNI(void, setRateChange)(JNIEnv *env, jclass type, jdouble newRate) {
    if(mSoundTouch!=NULL){
        mSoundTouch->setRateChange(newRate);
    }
}
JNI(void, setRate)(JNIEnv *env, jclass type, jdouble newRate) {
    if(mSoundTouch!=NULL){
        mSoundTouch->setRate(newRate);
    }
}

JNI(void, init)(JNIEnv *env, jclass type) {
    if (mSoundTouch == NULL) {
        mSoundTouch = new SoundTouch();
    }
}

JNI(void, release)(JNIEnv *env, jclass type) {
    if (mSoundTouch != NULL) {
        delete (mSoundTouch);
        mSoundTouch = NULL;
    }
}