package com.jake.mamba.opengl;

import android.opengl.EGL14;
import android.opengl.EGLConfig;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.opengl.EGLExt;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.Matrix;
import android.util.Log;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.opengles.GL10;

/**
 * @author jake
 * @since 2018/3/19 下午11:54
 */

public class EglUtils {
    public static final int NO_TEXTURE = -1;
    public static int getExternalOESTextureID() {
        int[] texture = new int[1];
        GLES20.glGenTextures(1, texture, 0);
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, texture[0]);
        GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
                GL10.GL_TEXTURE_MIN_FILTER, GL10.GL_LINEAR);
        GLES20.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
                GL10.GL_TEXTURE_MAG_FILTER, GL10.GL_LINEAR);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
                GL10.GL_TEXTURE_WRAP_S, GL10.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES,
                GL10.GL_TEXTURE_WRAP_T, GL10.GL_CLAMP_TO_EDGE);
        return texture[0];
    }
    /**
     * 用于判断是否支持egl
     *
     * @param versionCode
     * @return
     */
    public static boolean isSupportOpenGLES(int versionCode) {
        final int EGL_RECORDABLE_ANDROID = 0x3142;
        EGLDisplay eglDisplay = null;
        EGLContext eglContext = null;
        boolean result = false;
        try {
            eglDisplay = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY);
            if (eglDisplay == EGL14.EGL_NO_DISPLAY) {
                throw new RuntimeException("unable to get EGL14 display");
            }
            int[] version = new int[2];
            if (!EGL14.eglInitialize(eglDisplay, version, 0, version, 1)) {
                throw new RuntimeException("uunable to initialize EGL14");
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
            EGLConfig[] eglConfigs = new EGLConfig[1];
            int[] numConfigs = new int[1];
            if (!EGL14.eglChooseConfig(eglDisplay, attrList, 0, eglConfigs, 0, eglConfigs.length,
                    numConfigs, 0)) {
                throw new RuntimeException("unable to find RGB888+recordable ES2 EGL config");
            }
            eglContext = EGL14.eglCreateContext(eglDisplay, eglConfigs[0], EGL14.eglGetCurrentContext(),
                    new int[]{
                            EGL14.EGL_CONTEXT_CLIENT_VERSION, versionCode,
                            EGL14.EGL_NONE
                    }, 0);

            int error;
            while ((error = EGL14.eglGetError()) != EGL14.EGL_SUCCESS) {
                throw new RuntimeException("eglGetError: 0x" + Integer.toHexString(error));
            }
            if (eglContext != null) {
                result = true;
            }
        } catch (Throwable t) {
            t.printStackTrace();
        } finally {
            if (eglDisplay != null && eglContext != null) {
                EGL14.eglDestroyContext(eglDisplay, eglContext);
                EGL14.eglTerminate(eglDisplay);
                EGL14.eglReleaseThread();
            }
        }
        return result;
    }

    private static final String TAG = "GlCommonUtil";

    public static int createProgram(String vertexSource, String fragmentSource) {
        int vs = loadShader(GLES20.GL_VERTEX_SHADER, vertexSource);
        int fs = loadShader(GLES20.GL_FRAGMENT_SHADER, fragmentSource);
        int program = GLES20.glCreateProgram();
        GLES20.glAttachShader(program, vs);
        GLES20.glAttachShader(program, fs);
        GLES20.glLinkProgram(program);
        int[] linkStatus = new int[1];
        GLES20.glGetProgramiv(program, GLES20.GL_LINK_STATUS, linkStatus, 0);
        if (linkStatus[0] != GLES20.GL_TRUE) {
            Log.e(TAG, "Could not link program:");
            Log.e(TAG, GLES20.glGetProgramInfoLog(program));
            GLES20.glDeleteProgram(program);
            program = 0;
        }
        return program;
    }

    public static int loadShader(int shaderType, String source) {
        int shader = GLES20.glCreateShader(shaderType);
        GLES20.glShaderSource(shader, source);
        GLES20.glCompileShader(shader);
        int[] compiled = new int[1];
        GLES20.glGetShaderiv(shader, GLES20.GL_COMPILE_STATUS, compiled, 0);
        if (compiled[0] == 0) {
            Log.e(TAG, "Could not compile shader(TYPE=" + shaderType + "):");
            Log.e(TAG, GLES20.glGetShaderInfoLog(shader));
            GLES20.glDeleteShader(shader);
            shader = 0;
        }
        return shader;
    }

    public static void checkGlError(String op) {
        int error;
        while ((error = GLES20.glGetError()) != GLES20.GL_NO_ERROR) {
            Log.e(TAG, op + ": glGetError: 0x" + Integer.toHexString(error));
            throw new RuntimeException("glGetError encountered (see log)");
        }
    }

    public static void checkEglError(String op) {
        int error;
        while ((error = EGL14.eglGetError()) != EGL14.EGL_SUCCESS) {
            Log.e(TAG, op + ": eglGetError: 0x" + Integer.toHexString(error));
            throw new RuntimeException("eglGetError encountered (see log)");
        }
    }

    public static void deleteGLTexture(int iTextureID) {
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        int[] aTextures = new int[]{iTextureID};
        GLES20.glDeleteTextures(1, aTextures, 0);
    }

    public static FloatBuffer createSquareVtx() {
        final float vtx[] = {
                // XYZ, UV
                -1f, 1f, 0f, 0f, 1f,
                -1f, -1f, 0f, 0f, 0f,
                1f, 1f, 0f, 1f, 1f,
                1f, -1f, 0f, 1f, 0f,
        };
        ByteBuffer bb = ByteBuffer.allocateDirect(4 * vtx.length);
        bb.order(ByteOrder.nativeOrder());
        FloatBuffer fb = bb.asFloatBuffer();
        fb.put(vtx);
        fb.position(0);
        return fb;
    }

    public static FloatBuffer createVertexBuffer() {
        final float vtx[] = {
                // XYZ
                -1f, 1f, 0f,
                -1f, -1f, 0f,
                1f, 1f, 0f,
                1f, -1f, 0f,
        };
        ByteBuffer bb = ByteBuffer.allocateDirect(4 * vtx.length);
        bb.order(ByteOrder.nativeOrder());
        FloatBuffer fb = bb.asFloatBuffer();
        fb.put(vtx);
        fb.position(0);
        return fb;
    }

    public static FloatBuffer createTextureCoordinateBuffer() {
        final float vtx[] = {
                // UV
                0f, 1f,
                0f, 0f,
                1f, 1f,
                1f, 0f,
        };
        ByteBuffer bb = ByteBuffer.allocateDirect(4 * vtx.length);
        bb.order(ByteOrder.nativeOrder());
        FloatBuffer fb = bb.asFloatBuffer();
        fb.put(vtx);
        fb.position(0);
        return fb;
    }

    public static float[] createIdentityMtx() {
        float[] m = new float[16];
        Matrix.setIdentityM(m, 0);
        return m;
    }

    /**
     * 获取纹理的坐标
     *
     * @param inputWidth
     * @param inputHeight
     * @param outputWidth
     * @param outputHeight
     * @return
     */
    public static FloatBuffer getTextureCoordinate(int inputWidth, int inputHeight, int outputWidth, int outputHeight) {
        if (inputWidth <= 0 || inputHeight <= 0 || outputHeight <= 0 || outputWidth <= 0) {
            return null;
        }
        FloatBuffer textureBuffer;
        float hRatio = outputWidth / ((float) inputWidth);
        float vRatio = outputHeight / ((float) inputHeight);
        float ratio;
        float vtx[];
        if (hRatio > vRatio) {
            ratio = outputHeight / (inputHeight * hRatio);
            vtx = new float[]{//UV
                    0f, 0.5f + ratio / 2,
                    0f, 0.5f - ratio / 2,
                    1f, 0.5f + ratio / 2,
                    1f, 0.5f - ratio / 2,
            };
        } else {
            ratio = outputWidth / (inputWidth * vRatio);
            vtx = new float[]{//UV
                    0.5f - ratio / 2, 1f,
                    0.5f - ratio / 2, 0f,
                    0.5f + ratio / 2, 1f,
                    0.5f + ratio / 2, 0f,
            };
        }
        ByteBuffer bb = ByteBuffer.allocateDirect(4 * vtx.length);
        bb.order(ByteOrder.nativeOrder());
        textureBuffer = bb.asFloatBuffer();
        textureBuffer.put(vtx);
        textureBuffer.position(0);
        return textureBuffer;
    }

}
