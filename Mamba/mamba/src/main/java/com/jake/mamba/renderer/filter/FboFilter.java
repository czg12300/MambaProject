package com.jake.mamba.renderer.filter;

import android.graphics.PointF;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;

import com.jake.mamba.opengl.EglUtils;

import java.nio.FloatBuffer;

public class FboFilter extends GPUFilter {

    private final FloatBuffer mVtxBuf = EglUtils.createSquareVtx();
    private final float[] mPosMtx = EglUtils.createIdentityMtx();

    protected int mTextureId = -1;
    private int mProgram = -1;
    private int maPositionHandle = -1;
    private int maTexCoordHandle = -1;
    private int muPosMtxHandle = -1;
    private int muTexMtxHandle = -1;
    private int mSingleStepOffsetHandler = -1;
    private int mBeautyFlagHandler = -1;

    private final int[] mTexId = new int[]{0};

    private int mFboId = -1;
    private int mRboId = -1;

    private int mOutputWidth = -1;
    private int mmOutputHeight = -1;
    private boolean mIsExternalOES;//是否从外部纹理读取数据

private float[] mTexMtx;
    public FboFilter() {
        mIsExternalOES = true;
    }

    public void setOutputSize(int width, int height) {
        mOutputWidth = width;
        mmOutputHeight = height;
    }


    @Override
    protected boolean onPrepare() {
        loadShaderAndParams();
        createEffectTexture();
        return true;
    }

    private void loadShaderAndParams() {
        EglUtils.checkGlError("initSH_S");
        mProgram = EglUtils.createProgram(Const.VERTEX, Const.FRAGMENT);
        maPositionHandle = GLES20.glGetAttribLocation(mProgram, "position");
        maTexCoordHandle = GLES20.glGetAttribLocation(mProgram, "inputTextureCoordinate");
        muPosMtxHandle = GLES20.glGetUniformLocation(mProgram, "uPosMtx");
        muTexMtxHandle = GLES20.glGetUniformLocation(mProgram, "uTexMtx");
        mSingleStepOffsetHandler = GLES20.glGetUniformLocation(mProgram, "singleStepOffset");
        mBeautyFlagHandler = GLES20.glGetUniformLocation(mProgram, "beautyFlag");

        if (mSingleStepOffsetHandler != -1) {
            setFloatVec2(mSingleStepOffsetHandler, new float[]{2.5f / mOutputWidth, 2.5f / mmOutputHeight});
        }
        if (mBeautyFlagHandler != -1) {
            setBeauty(1);
        }
        EglUtils.checkGlError("initSH_E");
    }

    private void createEffectTexture() {
        if (mOutputWidth <= 0 || mmOutputHeight <= 0) {
            return;
        }
        EglUtils.checkGlError("initFBO_S");
        createFrameBuffer();
        GLES20.glGenTextures(1, mTexId, 0);

        GLES20.glBindRenderbuffer(GLES20.GL_RENDERBUFFER, mRboId);
        GLES20.glRenderbufferStorage(GLES20.GL_RENDERBUFFER,
                GLES20.GL_DEPTH_COMPONENT16, mOutputWidth, mmOutputHeight);

        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, mFboId);
        GLES20.glFramebufferRenderbuffer(GLES20.GL_FRAMEBUFFER,
                GLES20.GL_DEPTH_ATTACHMENT, GLES20.GL_RENDERBUFFER, mRboId);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTexId[0]);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_MIN_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_MAG_FILTER, GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_WRAP_S, GLES20.GL_CLAMP_TO_EDGE);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,
                GLES20.GL_TEXTURE_WRAP_T, GLES20.GL_CLAMP_TO_EDGE);

        GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_RGBA,
                mOutputWidth, mmOutputHeight, 0, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, null);

        GLES20.glFramebufferTexture2D(GLES20.GL_FRAMEBUFFER,
                GLES20.GL_COLOR_ATTACHMENT0, GLES20.GL_TEXTURE_2D, mTexId[0], 0);

        if (GLES20.glCheckFramebufferStatus(GLES20.GL_FRAMEBUFFER) !=
                GLES20.GL_FRAMEBUFFER_COMPLETE) {
            throw new RuntimeException("glCheckFramebufferStatus()");
        }
        EglUtils.checkGlError("initFBO_E");
    }



    public int getTextureId() {
        return mTexId[0];
    }

    public void setTexMtx(float[] mTexMtx) {
        this.mTexMtx = mTexMtx;
    }

    public void setBeauty(final int level) {
        runOnDraw(new Runnable() {
            @Override
            public void run() {
                setInteger(mBeautyFlagHandler, level);
            }
        });

    }

    @Override
    protected void onDraw() {
        mVtxBuf.position(0);
        GLES20.glVertexAttribPointer(maPositionHandle,
                3, GLES20.GL_FLOAT, false, 4 * (3 + 2), mVtxBuf);
        GLES20.glEnableVertexAttribArray(maPositionHandle);

        mVtxBuf.position(3);
        GLES20.glVertexAttribPointer(maTexCoordHandle,
                2, GLES20.GL_FLOAT, false, 4 * (3 + 2), mVtxBuf);
        GLES20.glEnableVertexAttribArray(maTexCoordHandle);

        if (muPosMtxHandle >= 0) {
            GLES20.glUniformMatrix4fv(muPosMtxHandle, 1, false, mPosMtx, 0);
        }
        if (muTexMtxHandle >= 0) {
            GLES20.glUniformMatrix4fv(muTexMtxHandle, 1, false, mTexMtx, 0);
        }
        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        if (mIsExternalOES) {
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, mTextureId);
        } else {
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, mTextureId);
        }
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);

        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, 0);
        if (mIsExternalOES) {
            GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, 0);
        } else {
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, 0);
        }
        GLES20.glUseProgram(0);

        EglUtils.checkGlError("draw_E");
    }

    @Override
    protected void onPrepareDraw() {
        EglUtils.checkGlError("draw_S");
        GLES20.glBindFramebuffer(GLES20.GL_FRAMEBUFFER, mFboId);

        GLES20.glViewport(0, 0, mWidth, mHeight);
        GLES20.glClearColor(0f, 0f, 0f, 1f);
        GLES20.glClear(GLES20.GL_DEPTH_BUFFER_BIT | GLES20.GL_COLOR_BUFFER_BIT);

        GLES20.glUseProgram(mProgram);
    }


    protected void setInteger(final int location, final int intValue) {
        runOnDraw(new Runnable() {
            @Override
            public void run() {
                GLES20.glUniform1i(location, intValue);
            }
        });
    }

    protected void setFloat(final int location, final float floatValue) {
        runOnDraw(new Runnable() {
            @Override
            public void run() {
                GLES20.glUniform1f(location, floatValue);
            }
        });
    }

    protected void setFloatVec2(final int location, final float[] arrayValue) {
        runOnDraw(new Runnable() {
            @Override
            public void run() {
                GLES20.glUniform2fv(location, 1, FloatBuffer.wrap(arrayValue));
            }
        });
    }

    protected void setFloatVec3(final int location, final float[] arrayValue) {
        runOnDraw(new Runnable() {
            @Override
            public void run() {
                GLES20.glUniform3fv(location, 1, FloatBuffer.wrap(arrayValue));
            }
        });
    }

    protected void setFloatVec4(final int location, final float[] arrayValue) {
        runOnDraw(new Runnable() {
            @Override
            public void run() {
                GLES20.glUniform4fv(location, 1, FloatBuffer.wrap(arrayValue));
            }
        });
    }

    protected void setFloatArray(final int location, final float[] arrayValue) {
        runOnDraw(new Runnable() {
            @Override
            public void run() {
                GLES20.glUniform1fv(location, arrayValue.length, FloatBuffer.wrap(arrayValue));
            }
        });
    }

    protected void setPoint(final int location, final PointF point) {
        runOnDraw(new Runnable() {

            @Override
            public void run() {
                float[] vec2 = new float[2];
                vec2[0] = point.x;
                vec2[1] = point.y;
                GLES20.glUniform2fv(location, 1, vec2, 0);
            }
        });
    }

    protected void setUniformMatrix3f(final int location, final float[] matrix) {
        runOnDraw(new Runnable() {

            @Override
            public void run() {
                GLES20.glUniformMatrix3fv(location, 1, false, matrix, 0);
            }
        });
    }

    protected void setUniformMatrix4f(final int location, final float[] matrix) {
        runOnDraw(new Runnable() {

            @Override
            public void run() {
                GLES20.glUniformMatrix4fv(location, 1, false, matrix, 0);
            }
        });
    }

    private void createFrameBuffer() {
        int[] fboId = new int[]{0};
        int[] rboId = new int[]{0};
        GLES20.glGenFramebuffers(1, fboId, 0);
        GLES20.glGenRenderbuffers(1, rboId, 0);
        mFboId = fboId[0];
        mRboId = rboId[0];
    }

    private void releaseFrameBuffer() {
        if (mFboId != -1) {
            int[] fboId = new int[]{mFboId};
            GLES20.glDeleteFramebuffers(1, fboId, 0);
            mFboId = -1;
        }
        if (mRboId != -1) {
            int[] rboId = new int[]{mRboId};
            GLES20.glDeleteRenderbuffers(1, rboId, 0);
            mRboId = -1;
        }
    }

    private void releaseProgram() {
        if (mProgram == -1) {
            return;
        }
        GLES20.glDeleteProgram(mProgram);
    }

    public void release() {
        releaseFrameBuffer();
        releaseProgram();
    }

    @Override
    protected void onRelease() {

    }

    private static class Const {
        static String VERTEX = "attribute vec4 position;\n" +
                "attribute vec4 inputTextureCoordinate;\n" +
                "\n" +
                "const int GAUSSIAN_SAMPLES = 9;\n" +
                "\n" +
                "uniform float texelWidthOffset;\n" +
                "uniform float texelHeightOffset;\n" +
                "\n" +
                "uniform   mat4 uPosMtx;\n" +
                "uniform   mat4 uTexMtx;\n" +
                "\n" +
                "varying vec2 textureCoordinate;\n" +
                "\n" +
                "void main()\n" +
                "{\n" +
                "    gl_Position = uPosMtx * position;\n" +
                "    textureCoordinate = (uTexMtx * inputTextureCoordinate).xy;\n" +
                "}";
        static String FRAGMENT = "precision mediump float;\n" +
                "varying vec2 textureCoordinate;\n" +
                "uniform sampler2D sTexture;\n" +
                "highp vec4 params = vec4(0.61, 0.82, 0.25, 0.07);\n" +
                "const highp vec3 W = vec3(0.299,0.587,0.114);\n" +
                "const mat3 saturateMatrix = mat3(1.1102,-0.0598,-0.061,\n" +
                "                                 -0.0774,1.0826,-0.1186,\n" +
                "                                 -0.0228,-0.0228,1.1772);\n" +
                "\n" +
                "uniform int beautyFlag;\n" +
                "uniform vec2 singleStepOffset;\n" +
                "\n" +
                "float hardlight(float color) {\n" +
                "  if(color <= 0.5) {\n" +
                "     color = color * color * 2.0;\n" +
                "  } else {\n" +
                "     color = 1.0 - ((1.0 - color)*(1.0 - color) * 2.0);\n" +
                "   }\n" +
                "  return color;\n" +
                "}\n" +
                "\n" +
                "void main() {\n" +
                "    vec4 tc = texture2D(sTexture, textureCoordinate);\n" +
                "    if(beautyFlag == 1) {\n" +
                "        float color = tc.r * 0.3 + tc.g * 0.59 + tc.b * 0.11;\n" +
                "        vec2 blurCoordinates[24];\n" +
                "        blurCoordinates[0] = textureCoordinate.xy + singleStepOffset * vec2(0.0, -10.0);\n" +
                "        //blurCoordinates[1] = textureCoordinate.xy + singleStepOffset * vec2(0.0, 10.0);\n" +
                "        blurCoordinates[2] = textureCoordinate.xy + singleStepOffset * vec2(-10.0, 0.0);\n" +
                "        //blurCoordinates[3] = textureCoordinate.xy + singleStepOffset * vec2(10.0, 0.0);\n" +
                "\n" +
                "        blurCoordinates[4] = textureCoordinate.xy + singleStepOffset * vec2(5.0, -8.0);\n" +
                "        //blurCoordinates[5] = textureCoordinate.xy + singleStepOffset * vec2(5.0, 8.0);\n" +
                "        blurCoordinates[6] = textureCoordinate.xy + singleStepOffset * vec2(-5.0, 8.0);\n" +
                "        //blurCoordinates[7] = textureCoordinate.xy + singleStepOffset * vec2(-5.0, -8.0);\n" +
                "\n" +
                "        blurCoordinates[8] = textureCoordinate.xy + singleStepOffset * vec2(8.0, -5.0);\n" +
                "        //blurCoordinates[9] = textureCoordinate.xy + singleStepOffset * vec2(8.0, 5.0);\n" +
                "        blurCoordinates[10] = textureCoordinate.xy + singleStepOffset * vec2(-8.0, 5.0);\n" +
                "        //blurCoordinates[11] = textureCoordinate.xy + singleStepOffset * vec2(-8.0, -5.0);\n" +
                "\n" +
                "        blurCoordinates[12] = textureCoordinate.xy + singleStepOffset * vec2(0.0, -6.0);\n" +
                "        //blurCoordinates[13] = textureCoordinate.xy + singleStepOffset * vec2(0.0, 6.0);\n" +
                "        blurCoordinates[14] = textureCoordinate.xy + singleStepOffset * vec2(6.0, 0.0);\n" +
                "        //blurCoordinates[15] = textureCoordinate.xy + singleStepOffset * vec2(-6.0, 0.0);\n" +
                "\n" +
                "        blurCoordinates[16] = textureCoordinate.xy + singleStepOffset * vec2(-4.0, -4.0);\n" +
                "        //blurCoordinates[17] = textureCoordinate.xy + singleStepOffset * vec2(-4.0, 4.0);\n" +
                "        blurCoordinates[18] = textureCoordinate.xy + singleStepOffset * vec2(4.0, -4.0);\n" +
                "        //blurCoordinates[19] = textureCoordinate.xy + singleStepOffset * vec2(4.0, 4.0);\n" +
                "\n" +
                "        blurCoordinates[20] = textureCoordinate.xy + singleStepOffset * vec2(-2.0, -2.0);\n" +
                "        //blurCoordinates[21] = textureCoordinate.xy + singleStepOffset * vec2(-2.0, 2.0);\n" +
                "        blurCoordinates[22] = textureCoordinate.xy + singleStepOffset * vec2(2.0, -2.0);\n" +
                "        //blurCoordinates[23] = textureCoordinate.xy + singleStepOffset * vec2(2.0, 2.0);\n" +
                "\n" +
                "\n" +
                "        float sampleColor = texture2D(sTexture, textureCoordinate).g * 11.0;\n" +
                "        sampleColor += texture2D(sTexture, blurCoordinates[0]).g;\n" +
                "        //sampleColor += texture2D(sTexture, blurCoordinates[1]).g;\n" +
                "        sampleColor += texture2D(sTexture, blurCoordinates[2]).g;\n" +
                "        //sampleColor += texture2D(sTexture, blurCoordinates[3]).g;\n" +
                "        sampleColor += texture2D(sTexture, blurCoordinates[4]).g;\n" +
                "        //sampleColor += texture2D(sTexture, blurCoordinates[5]).g;\n" +
                "        sampleColor += texture2D(sTexture, blurCoordinates[6]).g;\n" +
                "        //sampleColor += texture2D(sTexture, blurCoordinates[7]).g;\n" +
                "        sampleColor += texture2D(sTexture, blurCoordinates[8]).g;\n" +
                "        //sampleColor += texture2D(sTexture, blurCoordinates[9]).g;\n" +
                "        sampleColor += texture2D(sTexture, blurCoordinates[10]).g;\n" +
                "        //sampleColor += texture2D(sTexture, blurCoordinates[11]).g;\n" +
                "\n" +
                "        sampleColor += texture2D(sTexture, blurCoordinates[12]).g * 2.0;\n" +
                "        //sampleColor += texture2D(sTexture, blurCoordinates[13]).g * 2.0;\n" +
                "        sampleColor += texture2D(sTexture, blurCoordinates[14]).g * 2.0;\n" +
                "        //sampleColor += texture2D(sTexture, blurCoordinates[15]).g * 2.0;\n" +
                "        sampleColor += texture2D(sTexture, blurCoordinates[16]).g * 2.0;\n" +
                "        //sampleColor += texture2D(sTexture, blurCoordinates[17]).g * 2.0;\n" +
                "        sampleColor += texture2D(sTexture, blurCoordinates[18]).g * 2.0;\n" +
                "        //sampleColor += texture2D(sTexture, blurCoordinates[19]).g * 2.0;\n" +
                "\n" +
                "        sampleColor += texture2D(sTexture, blurCoordinates[20]).g * 3.0;\n" +
                "        //sampleColor += texture2D(sTexture, blurCoordinates[21]).g * 3.0;\n" +
                "        sampleColor += texture2D(sTexture, blurCoordinates[22]).g * 3.0;\n" +
                "        //sampleColor += texture2D(sTexture, blurCoordinates[23]).g * 3.0;\n" +
                "\n" +
                "        sampleColor = sampleColor / 31.0;\n" +
                "        vec3 centralColor = texture2D(sTexture, textureCoordinate).rgb;\n" +
                "        float highpass = centralColor.g - sampleColor + 0.5;\n" +
                "        for(int i = 0; i < 5;i++)\n" +
                "        {\n" +
                "           highpass = hardlight(highpass);\n" +
                "        }\n" +
                "        float lumance = dot(centralColor, W);\n" +
                "        float alpha = pow(lumance, params.r);\n" +
                "        vec3 smoothColor = centralColor + (centralColor-vec3(highpass))*alpha*0.07;\n" +
                "        gl_FragColor = vec4(smoothColor,1.0);\n" +
                "        gl_FragColor = vec4(((gl_FragColor.rgb - vec3(0.5)) * 1.06 + vec3(0.5)), 1.0);\n" +
                "    } else if(beautyFlag == 2) {\n" +
                "                float color = tc.r * 0.3 + tc.g * 0.59 + tc.b * 0.11;\n" +
                "                vec2 blurCoordinates[24];\n" +
                "                blurCoordinates[0] = textureCoordinate.xy + singleStepOffset * vec2(0.0, -10.0);\n" +
                "                //blurCoordinates[1] = textureCoordinate.xy + singleStepOffset * vec2(0.0, 10.0);\n" +
                "                blurCoordinates[2] = textureCoordinate.xy + singleStepOffset * vec2(-10.0, 0.0);\n" +
                "                //blurCoordinates[3] = textureCoordinate.xy + singleStepOffset * vec2(10.0, 0.0);\n" +
                "\n" +
                "                blurCoordinates[4] = textureCoordinate.xy + singleStepOffset * vec2(5.0, -8.0);\n" +
                "                //blurCoordinates[5] = textureCoordinate.xy + singleStepOffset * vec2(5.0, 8.0);\n" +
                "                blurCoordinates[6] = textureCoordinate.xy + singleStepOffset * vec2(-5.0, 8.0);\n" +
                "                //blurCoordinates[7] = textureCoordinate.xy + singleStepOffset * vec2(-5.0, -8.0);\n" +
                "\n" +
                "                blurCoordinates[8] = textureCoordinate.xy + singleStepOffset * vec2(8.0, -5.0);\n" +
                "                //blurCoordinates[9] = textureCoordinate.xy + singleStepOffset * vec2(8.0, 5.0);\n" +
                "                blurCoordinates[10] = textureCoordinate.xy + singleStepOffset * vec2(-8.0, 5.0);\n" +
                "                //blurCoordinates[11] = textureCoordinate.xy + singleStepOffset * vec2(-8.0, -5.0);\n" +
                "\n" +
                "                blurCoordinates[12] = textureCoordinate.xy + singleStepOffset * vec2(0.0, -6.0);\n" +
                "                //blurCoordinates[13] = textureCoordinate.xy + singleStepOffset * vec2(0.0, 6.0);\n" +
                "                blurCoordinates[14] = textureCoordinate.xy + singleStepOffset * vec2(6.0, 0.0);\n" +
                "                //blurCoordinates[15] = textureCoordinate.xy + singleStepOffset * vec2(-6.0, 0.0);\n" +
                "\n" +
                "                blurCoordinates[16] = textureCoordinate.xy + singleStepOffset * vec2(-4.0, -4.0);\n" +
                "                //blurCoordinates[17] = textureCoordinate.xy + singleStepOffset * vec2(-4.0, 4.0);\n" +
                "                blurCoordinates[18] = textureCoordinate.xy + singleStepOffset * vec2(4.0, -4.0);\n" +
                "                //blurCoordinates[19] = textureCoordinate.xy + singleStepOffset * vec2(4.0, 4.0);\n" +
                "\n" +
                "                blurCoordinates[20] = textureCoordinate.xy + singleStepOffset * vec2(-2.0, -2.0);\n" +
                "                //blurCoordinates[21] = textureCoordinate.xy + singleStepOffset * vec2(-2.0, 2.0);\n" +
                "                blurCoordinates[22] = textureCoordinate.xy + singleStepOffset * vec2(2.0, -2.0);\n" +
                "                //blurCoordinates[23] = textureCoordinate.xy + singleStepOffset * vec2(2.0, 2.0);\n" +
                "\n" +
                "\n" +
                "                float sampleColor = texture2D(sTexture, textureCoordinate).g * 6.0;\n" +
                "                sampleColor += texture2D(sTexture, blurCoordinates[0]).g;\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[1]).g;\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[2]).g;\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[3]).g;\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[4]).g;\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[5]).g;\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[6]).g;\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[7]).g;\n" +
                "                sampleColor += texture2D(sTexture, blurCoordinates[8]).g;\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[9]).g;\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[10]).g;\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[11]).g;\n" +
                "\n" +
                "                sampleColor += texture2D(sTexture, blurCoordinates[12]).g * 2.0;\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[13]).g * 2.0;\n" +
                "                sampleColor += texture2D(sTexture, blurCoordinates[14]).g * 2.0;\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[15]).g * 2.0;\n" +
                "                sampleColor += texture2D(sTexture, blurCoordinates[16]).g * 2.0;\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[17]).g * 2.0;\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[18]).g * 2.0;\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[19]).g * 2.0;\n" +
                "\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[20]).g * 3.0;\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[21]).g * 3.0;\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[22]).g * 3.0;\n" +
                "                //sampleColor += texture2D(sTexture, blurCoordinates[23]).g * 3.0;\n" +
                "\n" +
                "                sampleColor = sampleColor / 14.0;\n" +
                "                vec3 centralColor = texture2D(sTexture, textureCoordinate).rgb;\n" +
                "                float highpass = centralColor.g - sampleColor + 0.5;\n" +
                "                for(int i = 0; i < 5;i++)\n" +
                "                {\n" +
                "                   highpass = hardlight(highpass);\n" +
                "                }\n" +
                "                float lumance = dot(centralColor, W);\n" +
                "                float alpha = pow(lumance, params.r);\n" +
                "                vec3 smoothColor = centralColor + (centralColor-vec3(highpass))*alpha*0.07;\n" +
                "                gl_FragColor = vec4(smoothColor,1.0);\n" +
                "                gl_FragColor = vec4(((gl_FragColor.rgb - vec3(0.5)) * 1.06 + vec3(0.5)), 1.0);\n" +
                "    } else {\n" +
                "        gl_FragColor = vec4(tc.rgb, 1.0);\n" +
                "    }\n" +
                "}";

    }
}
