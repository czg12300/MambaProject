package com.jake.mamba.renderer.filter;

import android.opengl.GLES11Ext;
import android.opengl.GLES20;

import com.jake.mamba.opengl.EglUtils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.opengles.GL10;

public class CameraFilter extends GPUFilter {

    private FloatBuffer mGLCubeBuffer;
    private FloatBuffer mGLTextureBuffer;

    protected int mTextureId = EglUtils.NO_TEXTURE;
    private int mProgram = -1;
    private int mOutputWidth = -1;
    private int mOutputHeight = -1;

    private float[] mTexMtx;
    private int mGLAttribPosition;
    private int mGLUniformTexture;
    private int mGLAttribTextureCoordinate;
    public static final float CUBE[] = {
            -1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, 1.0f,
    };
    public static final float TEXTURE_NO_ROTATION[] = {
            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
    };
    float[] rotatedTex = TEXTURE_NO_ROTATION;

    private static float flip(final float i) {
        if (i == 0.0f) {
            return 1.0f;
        }
        return 0.0f;
    }

    {
        rotatedTex = new float[]{
                rotatedTex[0], flip(rotatedTex[1]),
                rotatedTex[2], flip(rotatedTex[3]),
                rotatedTex[4], flip(rotatedTex[5]),
                rotatedTex[6], flip(rotatedTex[7]),
        };
    }

    public CameraFilter() {
        mGLCubeBuffer = ByteBuffer.allocateDirect(CUBE.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer();
        mGLCubeBuffer.put(CUBE).position(0);
        mGLTextureBuffer = ByteBuffer.allocateDirect(TEXTURE_NO_ROTATION.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer();
        mGLTextureBuffer.put(rotatedTex).position(0);
    }

    public void setOutputSize(int width, int height) {
        mOutputWidth = width;
        mOutputHeight = height;
    }


    @Override
    protected boolean onPrepare() {
        loadShaderAndParams();
        return true;
    }

    public int getTextureId() {
        if (mTextureId == EglUtils.NO_TEXTURE) {
            mTextureId = createExternalOESTextureID();
        }
        return mTextureId;
    }

    private int createExternalOESTextureID() {
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

    private int mTextureTransformMatrixLocation;

    private void loadShaderAndParams() {
        mProgram = EglUtils.createProgram(Const.VERTEX, Const.FRAGMENT);
        mGLAttribPosition = GLES20.glGetAttribLocation(mProgram, "position");
        mGLUniformTexture = GLES20.glGetUniformLocation(mProgram, "inputImageTexture");
        mGLAttribTextureCoordinate = GLES20.glGetAttribLocation(mProgram, "inputTextureCoordinate");
        mTextureTransformMatrixLocation = GLES20.glGetUniformLocation(mProgram, "textureTransform");
    }


    public void setTexMtx(float[] mTexMtx) {
        this.mTexMtx = mTexMtx;
    }


    @Override
    protected void onDraw() {
        mGLCubeBuffer.position(0);
        GLES20.glVertexAttribPointer(mGLAttribPosition, 2, GLES20.GL_FLOAT, false, 0, mGLCubeBuffer);
        GLES20.glEnableVertexAttribArray(mGLAttribPosition);
        mGLTextureBuffer.position(0);
        GLES20.glVertexAttribPointer(mGLAttribTextureCoordinate, 2, GLES20.GL_FLOAT, false, 0, mGLTextureBuffer);
        GLES20.glEnableVertexAttribArray(mGLAttribTextureCoordinate);
        GLES20.glUniformMatrix4fv(mTextureTransformMatrixLocation, 1, false, mTexMtx, 0);

        if (mTextureId != -1) {
            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, mTextureId);
            GLES20.glUniform1i(mGLUniformTexture, 0);
        }

        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
        GLES20.glDisableVertexAttribArray(mGLAttribPosition);
        GLES20.glDisableVertexAttribArray(mGLAttribTextureCoordinate);
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, 0);
//        GLES20.glUseProgram(0);

    }

    @Override
    protected void onPrepareDraw() {
        EglUtils.checkGlError("draw_S");
        GLES20.glViewport(0, 0, mWidth, mHeight);
        GLES20.glClearColor(0f, 0f, 0f, 1f);
        GLES20.glClear(GLES20.GL_DEPTH_BUFFER_BIT | GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glUseProgram(mProgram);
    }

    private void releaseProgram() {
        if (mProgram == -1) {
            return;
        }
        GLES20.glDeleteProgram(mProgram);
    }

    public void release() {
        releaseProgram();
    }

    @Override
    protected void onRelease() {

    }

    private static class Const {
        static String VERTEX = "attribute vec4 position;\n" +
                "attribute vec4 inputTextureCoordinate;\n" +
                "\n" +
                "uniform mat4 textureTransform;\n" +
                "varying vec2 textureCoordinate;\n" +
                "\n" +
                "void main()\n" +
                "{\n" +
                "	textureCoordinate = (textureTransform * inputTextureCoordinate).xy;\n" +
                "	gl_Position = position;\n" +
                "}";
        static String FRAGMENT = "#extension GL_OES_EGL_image_external : require\n" +
                "varying highp vec2 textureCoordinate;\n" +
                "\n" +
                "uniform samplerExternalOES inputImageTexture;\n" +
                "\n" +
                "void main()\n" +
                "{\n" +
                "	gl_FragColor = texture2D(inputImageTexture, textureCoordinate);\n" +
                "}";

    }
}
