package com.jake.mamba.opengl;

import android.opengl.EGL14;
import android.opengl.EGLConfig;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.opengl.EGLExt;
import android.opengl.EGLSurface;
import android.view.Surface;

public class EglContextManager {
    private static final int EGL_RECORDABLE_ANDROID = 0x3142;
    private EGLDisplay mEGLDisplay = null;
    private EGLContext mEGLContext = null;
    private EGLConfig[] mConfigs = null;

    public EglContextManager() {
        eglContextSetup(2);
    }

    public EglContextManager(int version) {
        eglContextSetup(version);
    }

    private void eglContextSetup(int versionCode) {
        mEGLDisplay = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY);
        if (mEGLDisplay == EGL14.EGL_NO_DISPLAY) {
            throw new RuntimeException("unable to get EGL14 display");
        }
        int[] version = new int[2];
        if (!EGL14.eglInitialize(mEGLDisplay, version, 0, version, 1)) {
            mEGLDisplay = null;
            throw new RuntimeException("unable to initialize EGL14");
        }
        int openGLBit = EGL14.EGL_OPENGL_ES2_BIT;
        if (versionCode == 3) {
            openGLBit = EGLExt.EGL_OPENGL_ES3_BIT_KHR;
        }
        int[] attrList = {
                EGL14.EGL_RED_SIZE, 8,
                EGL14.EGL_GREEN_SIZE, 8,
                EGL14.EGL_BLUE_SIZE, 8,
                EGL14.EGL_RENDERABLE_TYPE, openGLBit,
                EGL_RECORDABLE_ANDROID, 1,
                EGL14.EGL_NONE
        };
        mConfigs = new EGLConfig[1];
        int[] numConfigs = new int[1];
        if (!EGL14.eglChooseConfig(mEGLDisplay, attrList, 0, mConfigs, 0, mConfigs.length, numConfigs, 0)) {
            throw new RuntimeException("unable to find RGB888+recordable ES2 EGL config");
        }
        int[] attr_list = {
                EGL14.EGL_CONTEXT_CLIENT_VERSION, versionCode,
                EGL14.EGL_NONE
        };
        mEGLContext = EGL14.eglCreateContext(mEGLDisplay, mConfigs[0], EGL14.eglGetCurrentContext(), attr_list, 0);
        EglUtils.checkEglError("eglCreateContext");
        if (mEGLContext == null) {
            throw new RuntimeException("null context");
        }
    }

    public EGLSurface makeEGLSurface(Surface surface) {
        int[] surfaceAttributes = {
                EGL14.EGL_NONE
        };
        EGLSurface EGLSurface = EGL14.eglCreateWindowSurface(mEGLDisplay, mConfigs[0], surface, surfaceAttributes, 0);
        EglUtils.checkEglError("eglCreateWindowSurface");
        if (EGLSurface == null) {
            throw new RuntimeException("surface was null");
        }
        return EGLSurface;
    }

    public void releaseEGLSurface(EGLSurface EGLSurface) {
        EGL14.eglDestroySurface(mEGLDisplay, EGLSurface);
    }

    public void makeCurrentSurface(EGLSurface EGLSurface) {
        if (!EGL14.eglMakeCurrent(mEGLDisplay, EGLSurface, EGLSurface, mEGLContext)) {
            throw new RuntimeException("eglMakeCurrent failed");
        }
    }

    public boolean swapSurfaceBuffers(EGLSurface EGLSurface) {
        return EGL14.eglSwapBuffers(mEGLDisplay, EGLSurface);
    }

    public void setSurfacePts(EGLSurface EGLSurface, long time) {
        EGLExt.eglPresentationTimeANDROID(mEGLDisplay, EGLSurface, time);
    }

    public void makeEGLContext() {
        if (!EGL14.eglMakeCurrent(mEGLDisplay, EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_SURFACE, mEGLContext)) {
            throw new RuntimeException("eglMakeContext failed");
        }
    }

    public void releaseEGLContext() {
        if (!EGL14.eglMakeCurrent(mEGLDisplay, EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_SURFACE, EGL14.EGL_NO_CONTEXT)) {
            throw new RuntimeException("eglMakeCurrent failed");
        }
    }

    public void release() {
        EGL14.eglDestroyContext(mEGLDisplay, mEGLContext);
        EGL14.eglReleaseThread();
        EGL14.eglTerminate(mEGLDisplay);
        mEGLDisplay = null;
        mEGLContext = null;
    }
}
