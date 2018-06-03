package com.jake.mamba.renderer.filter;

import android.graphics.Bitmap;
import android.opengl.GLES20;
import android.opengl.GLUtils;
import android.opengl.Matrix;

import com.jake.mamba.opengl.EglUtils;

import java.nio.FloatBuffer;

/**
 * 使用lookup图片作为滤镜
 *
 * @author jake
 * @since 2018/4/15 下午11:13
 */

public class LookupFilter extends GPUFilter {

    public static final String LOOKUP_VERTEX_SHADER = "" +
            "attribute vec4 position;\n" +
            "attribute vec4 inputTextureCoordinate;\n" +
            "uniform   mat4 uPosMtx;\n" +
            "varying   vec2 textureCoordinate;\n" +
            "void main() {\n" +
            "  gl_Position = uPosMtx * position;\n" +
            "  textureCoordinate   = inputTextureCoordinate.xy;\n" +
            "}\n";

    public static final String LOOKUP_FRAGMENT_SHADER = "" +
            "precision mediump float;\n" +
            "varying highp vec2 textureCoordinate;\n" +
            " \n" +
            " uniform sampler2D inputImageTexture;\n" +
            " uniform sampler2D inputImageTexture2; // lookup texture\n" +
            " \n" +
            " uniform int lookupFlag;\n" +
            " void main()\n" +
            " {\n" +
            "     lowp vec4 textureColor = texture2D(inputImageTexture, textureCoordinate);\n" +
            "     if(lookupFlag == 1) {\n" +
            "          \n" +
            "          mediump float blueColor = textureColor.b * 63.0;\n" +
            "          \n" +
            "          mediump vec2 quad1;\n" +
            "          quad1.y = floor(floor(blueColor) / 8.0);\n" +
            "          quad1.x = floor(blueColor) - (quad1.y * 8.0);\n" +
            "          \n" +
            "          mediump vec2 quad2;\n" +
            "          quad2.y = floor(ceil(blueColor) / 8.0);\n" +
            "          quad2.x = ceil(blueColor) - (quad2.y * 8.0);\n" +
            "          \n" +
            "          highp vec2 texPos1;\n" +
            "          texPos1.x = (quad1.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);\n" +
            "          texPos1.y = (quad1.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);\n" +
            "          \n" +
            "          highp vec2 texPos2;\n" +
            "          texPos2.x = (quad2.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);\n" +
            "          texPos2.y = (quad2.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);\n" +
            "          \n" +
            "          lowp vec4 newColor1 = texture2D(inputImageTexture2, texPos1);\n" +
            "          lowp vec4 newColor2 = texture2D(inputImageTexture2, texPos2);\n" +
            "          \n" +
            "          lowp vec4 newColor = mix(newColor1, newColor2, fract(blueColor));\n" +
            "          gl_FragColor = vec4(newColor.rgb, textureColor.w);\n" +
            "      } else {\n" +
            "          gl_FragColor = vec4(textureColor.rgb, 1.0);\n" +
            "      }\n" +
            " }";
    public static final int VIDEO_SCALE_MODE_FIT_XY = 1;
    public static final int VIDEO_SCALE_MODE_CENTER_INSIDE = 2;
    public static final int VIDEO_SCALE_MODE_CENTER_CROP = 3;
    public static final int VIDEO_SCALE_MODE_LF_ROOM = 4;

    private final FloatBuffer mNormalVtxBuf = EglUtils.createVertexBuffer();
    private final float[] mNormalPosMtx = EglUtils.createIdentityMtx();
    private final float[] mMirrorPosMtx = EglUtils.createIdentityMtx();

    private int mLookupTextureId = -1;


    private int mProgram = -1;
    private int maPositionHandle = -1;
    private int maTextureHandle = -1;
    private int muPosMtxHandle = -1;
    private int muSamplerHandle = -1;
    private int muSampler2Handle = -1;
    private int muLookupFlagHandler = -1;

    private FloatBuffer mTextureBuffer;
    private boolean mMirror;
    private int mScaleMode;
    private int mOutputWidth;
    private int mOutputHeight;

    public LookupFilter() {
        Matrix.scaleM(mMirrorPosMtx, 0, -1, 1, 1);
        mScaleMode = VIDEO_SCALE_MODE_CENTER_CROP;
    }

    public void setOutputSize(int width, int height) {
        mOutputWidth = width;
        mOutputHeight = height;
    }

    @Override
    protected boolean onPrepare() {
        EglUtils.checkGlError("initGL_S");
        mProgram = EglUtils.createProgram(LOOKUP_VERTEX_SHADER, LOOKUP_FRAGMENT_SHADER);
        maPositionHandle = GLES20.glGetAttribLocation(mProgram, "position");
        maTextureHandle = GLES20.glGetAttribLocation(mProgram, "inputTextureCoordinate");
        muSamplerHandle = GLES20.glGetUniformLocation(mProgram, "inputImageTexture");
        muSampler2Handle = GLES20.glGetUniformLocation(mProgram, "inputImageTexture2");
        muPosMtxHandle = GLES20.glGetUniformLocation(mProgram, "uPosMtx");
        muLookupFlagHandler = GLES20.glGetUniformLocation(mProgram, "lookupFlag");

        GLES20.glDisable(GLES20.GL_DEPTH_TEST);
        GLES20.glDisable(GLES20.GL_CULL_FACE);
        GLES20.glDisable(GLES20.GL_BLEND);

        EglUtils.checkGlError("initGL_E");
        return true;
    }

    @Override
    protected void onPrepareDraw() {

    }


    @Override
    protected void onDraw() {
        EglUtils.checkGlError("draw_S");
        GLES20.glViewport(0, 0, mWidth, mHeight);
        GLES20.glClearColor(0.5f, 0.5f, 0.5f, 1f);
        GLES20.glClear(GLES20.GL_DEPTH_BUFFER_BIT | GLES20.GL_COLOR_BUFFER_BIT);
        GLES20.glUseProgram(mProgram);
        //绘制摄像头预览界面
        if (mTextureBuffer == null) {
            mTextureBuffer = EglUtils.getTextureCoordinate(mWidth, mHeight, mOutputWidth, mOutputHeight);
        }
        mNormalVtxBuf.position(0);
        GLES20.glVertexAttribPointer(maPositionHandle,
                3, GLES20.GL_FLOAT, false, 4 * 3, mNormalVtxBuf);
        GLES20.glEnableVertexAttribArray(maPositionHandle);
        mTextureBuffer.position(0);
        GLES20.glVertexAttribPointer(maTextureHandle,
                2, GLES20.GL_FLOAT, false, 4 * 2, mTextureBuffer);
        GLES20.glEnableVertexAttribArray(maTextureHandle);

        if (mMirror) {
            GLES20.glUniformMatrix4fv(muPosMtxHandle, 1, false, mMirrorPosMtx, 0);
        } else {
            GLES20.glUniformMatrix4fv(muPosMtxHandle, 1, false, mNormalPosMtx, 0);
        }

        if (muSamplerHandle >= 0) {
            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mInputTextureId);
            GLES20.glUniform1i(muSamplerHandle, 0);
        }
        if (muSampler2Handle >= 0) {
            if (mLookupTextureId != -1) {
                GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
                GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mLookupTextureId);
                GLES20.glUniform1i(muSampler2Handle, 1);
                GLES20.glUniform1i(muLookupFlagHandler, 1);
            } else {
                GLES20.glUniform1i(muLookupFlagHandler, 0);
            }
        }
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);

        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        GLES20.glUseProgram(0);
        EglUtils.checkGlError("draw_E");
    }

    @Override
    protected void onRelease() {
        if (mLookupTextureId != -1) {
            EglUtils.deleteGLTexture(mLookupTextureId);
        }
    }


    public void setLookup(Bitmap bitmap) {
        if (mLookupTextureId != -1) {
            EglUtils.deleteGLTexture(mLookupTextureId);
        }
        if (bitmap != null) {
            int[] textures = new int[1];
            GLES20.glGenTextures(1, textures, 0);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, textures[0]);
            GLUtils.texImage2D(GLES20.GL_TEXTURE_2D, 0, bitmap, 0);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MIN_FILTER,
                    GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_MAG_FILTER,
                    GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_S,
                    GLES20.GL_CLAMP_TO_EDGE);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D, GLES20.GL_TEXTURE_WRAP_T,
                    GLES20.GL_CLAMP_TO_EDGE);
            mLookupTextureId = textures[0];
            bitmap.recycle();
        } else {
            mLookupTextureId = -1;
        }
    }


    public void setMirror(boolean mirror) {
        mMirror = mirror;
    }

}
