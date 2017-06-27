//
// Created by yongfali on 2016/4/7.
//

#include <stdio.h>
#include <assert.h>
#include <jni.h>
#include "Log.h"
#include "include/KuGouEffect.h"

using namespace e;

#ifdef __cplusplus
extern "C" {
#endif

struct fields_t {
    jclass clazz;
    jfieldID context;
};

static struct fields_t fields;
static const char *const kClassPathName = "com/kugou/fanxing/KuGouEffect";

static KuGouEffect *getEffectInstance(JNIEnv *env, jobject thiz) {
    return reinterpret_cast<KuGouEffect *>(env->GetLongField(thiz, fields.context));
}

static void setupEffectInstance(JNIEnv *env, jobject thiz, KuGouEffect *player) {
    env->SetLongField(thiz, fields.context, reinterpret_cast<jlong>(player));
}

JNIEXPORT void kugou_effect_static_init(JNIEnv *env) {
    jclass clazz = env->FindClass(kClassPathName);
    if (clazz == NULL) {
        LOGE("JNIKuGouEffect kugou_effect_static_init FindClass(%s) failed!", kClassPathName);
        return;
    }

    fields.clazz = static_cast<jclass>(env->NewGlobalRef(clazz));
    fields.context = env->GetFieldID(fields.clazz, "mNativeContext", "J");
    if (fields.context == NULL) {
        //LOGE("JNIKuGouEffect kugou_effect_static_init failed!");
        return;
    }
}

JNIEXPORT void kugou_effect_create(JNIEnv *env, jobject obj) {
    KuGouEffect *effect = KuGouEffect::Singleton(KuGouEffectLevelNormal, "KuGou-Media-Group");
    if (effect != NULL) {
        setupEffectInstance(env, obj, effect);
        LOGD("JNIKuGouEffect create!!");
    }
}

JNIEXPORT void kugou_effect_destroy(JNIEnv *env, jobject obj) {
    KuGouEffect *effect = getEffectInstance(env, obj);
    if (effect != NULL) {
        setupEffectInstance(env, obj, 0);
        KuGouEffect::ReleaseInstance();
        LOGD("JNIKuGouEffect destroy!!!");
    }
}

JNIEXPORT void
kugou_effect_setViewport(JNIEnv *env, jobject obj, jint x, jint y, jint width, jint height) {
    KuGouEffect *effect = getEffectInstance(env, obj);
    if (effect != NULL) {
        effect->SetViewport(x, y, width, height);
    }
}

JNIEXPORT void
kugou_effect_setImageRotation(JNIEnv *env, jobject obj, jint angle, jint flipX, jint flipY) {
    KuGouEffect *effect = getEffectInstance(env, obj);
    if (effect != NULL) {
        effect->SetImageRotation(angle, flipX, flipY);
    }
}

JNIEXPORT void kugou_effect_setDeviceRotation(JNIEnv *env, jobject obj, jint angle) {
    KuGouEffect *effect = getEffectInstance(env, obj);
    if (effect != NULL) {
        effect->SetDeviceRotation(angle);
    }
}

JNIEXPORT void kugou_effect_setAnimateCacheSize(JNIEnv *env, jobject obj, jint size) {
    KuGouEffect *effect = getEffectInstance(env, obj);
    if (effect != NULL) {
        effect->SetAnimateCacheSize(size);
    }
}

JNIEXPORT void kugou_effect_setAnimatePath(JNIEnv *env, jobject obj, jstring path) {
    KuGouEffect *effect = getEffectInstance(env, obj);
    if (effect != NULL) {
        const char *text = env->GetStringUTFChars(path, NULL);
        if (text) {
            effect->SetAnimatePath(text);

            env->ReleaseStringUTFChars(path, text);
        }
    }
}

JNIEXPORT void kugou_effect_setFaceAREnable(JNIEnv *env, jobject obj, jint enable) {
    KuGouEffect *effect = getEffectInstance(env, obj);
    if (effect != NULL) {
        effect->SetFaceAREnable(enable);
    }
}

JNIEXPORT void kugou_effect_setBeautyEnable(JNIEnv *env, jobject obj, jint enable) {
    KuGouEffect *effect = getEffectInstance(env, obj);
    if (effect != NULL) {
        effect->SetBeautyEnable(enable);
    }
}

JNIEXPORT void kugou_effect_setBeautyParams(JNIEnv *env, jobject obj, jint type, jfloat value) {
    KuGouEffect *effect = getEffectInstance(env, obj);
    if (effect != NULL) {
        effect->SetBeautyParams((BeautyParams) type, value);
    }
}

JNIEXPORT void kugou_effect_setBeautyLevel(JNIEnv *env, jobject obj, jint level) {
    KuGouEffect *effect = getEffectInstance(env, obj);
    if (effect != NULL) {
        effect->SetBeautyLevel(level);
    }
}

JNIEXPORT void
kugou_effect_render(JNIEnv *env, jobject obj, jbyteArray array, jint width, jint height,
                    jint format) {
    KuGouEffect *effect = getEffectInstance(env, obj);
    if (effect != NULL) {
        if (array != NULL) {
            jbyte *pBuffer = env->GetByteArrayElements(array, 0);
            if (pBuffer) {
                effect->Render(pBuffer, width, height, format);
                env->ReleaseByteArrayElements(array, pBuffer, JNI_ABORT);
            } else {
                LOGD("JNIKuGouEffect render buffer is null");
            }
        } else {
            //effect->Render(NULL, width, height, 12, format);
            LOGD("JNIKuGouEffect render array is null");
        }
    } else {
        LOGD("JNIKuGouEffect effect object is null");
    }
}
JNIEXPORT void
kugou_effect_render1(JNIEnv *env, jobject obj, jbyteArray dst, jbyteArray src, jint width,
                    jint height, jint format) {
    KuGouEffect *effect = getEffectInstance(env, obj);
    if (effect != NULL) {
        if (dst != NULL&&src!=NULL) {
            jbyte *pDst = env->GetByteArrayElements(dst, 0);
            jbyte *pSrc = env->GetByteArrayElements(src, 0);
            if (pDst&&pSrc) {
                effect->Render(pDst, pSrc, width, height, format);
                env->ReleaseByteArrayElements(dst, pDst, JNI_ABORT);
                env->ReleaseByteArrayElements(src, pSrc, JNI_ABORT);
            } else {
                LOGD("JNIKuGouEffect render buffer is null");
            }
        } else {
            //effect->Render(NULL, width, height, 12, format);
            LOGD("JNIKuGouEffect render array is null");
        }
    } else {
        LOGD("JNIKuGouEffect effect object is null");
    }
}

static JNINativeMethod gMethods[] =
        {
                {"static_init",         "()V",                   (void *) kugou_effect_static_init},
                {"create",              "()V",                   (void *) kugou_effect_create},
                {"setViewport",         "(IIII)V",               (void *) kugou_effect_setViewport},
                {"setImageRotation",    "(III)V",                (void *) kugou_effect_setImageRotation},
                {"setDeviceRotation",   "(I)V",                  (void *) kugou_effect_setDeviceRotation},
                {"setAnimateCacheSize", "(I)V",                  (void *) kugou_effect_setAnimateCacheSize},
                {"setAnimatePath",      "(Ljava/lang/String;)V", (void *) kugou_effect_setAnimatePath},
                {"setFaceAREnable",     "(I)V",                  (void *) kugou_effect_setFaceAREnable},
                {"setBeautyEnable",     "(I)V",                  (void *) kugou_effect_setBeautyEnable},
                {"setBeautyParams",     "(IF)V",                 (void *) kugou_effect_setBeautyParams},
                {"setBeautyLevel",      "(I)V",                  (void *) kugou_effect_setBeautyLevel},
                {"render",              "([BIII)V",              (void *) kugou_effect_render},
                {"render",              "([B[BIII)V",             (void *) kugou_effect_render1},
                {"destroy",             "()V",                   (void *) kugou_effect_destroy},
        };

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    jclass clazz;
    jint result = -1;

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        LOGE("JNI OnLoad get env failed!");
        return result;
    }
    assert(env != NULL);

    clazz = env->FindClass(kClassPathName);
    if (clazz == NULL) {
        LOGE("JNI OnLoad find class failed!");
        return result;
    }

    if (env->RegisterNatives(clazz, gMethods, sizeof(gMethods) / sizeof(gMethods[0])) < 0) {
        LOGE("JNI OnLoad register native failed! %u", sizeof(gMethods) / sizeof(gMethods[0]));
        return result;
    }

    /* success -- return valid version number */
    result = JNI_VERSION_1_4;
    LOGD("JNI OnLoad done\r\n");
    return result;
}

#ifdef __cplusplus
}
#endif