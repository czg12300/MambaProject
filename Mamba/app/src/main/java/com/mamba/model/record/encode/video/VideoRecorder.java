package com.mamba.model.record.encode.video;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.opengl.EGLContext;
import android.view.Surface;

import com.mamba.AppApplication;
import com.mamba.model.VLog;
import com.mamba.model.record.encode.OnRecorderListener;
import com.mamba.model.record.renderer.ISurfaceRenderer;
import com.mamba.model.record.renderer.gles.EglCore;
import com.mamba.model.record.renderer.gles.WindowSurface;
import com.mamba.model.record.renderer.gpuimage.FilterFactory;
import com.mamba.model.record.renderer.gpuimage.FilterType;
import com.mamba.model.record.renderer.gpuimage.Rotation;
import com.mamba.model.record.renderer.gpuimage.TextureRotationUtil;
import com.mamba.model.record.renderer.gpuimage.filter.CameraInputFilter;
import com.mamba.model.record.renderer.gpuimage.filter.GPUImageFilter;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.Arrays;
import java.util.Queue;
import java.util.concurrent.LinkedBlockingQueue;


/**
 * 视频录制
 *
 * @author jake
 * @since 2017/8/8 下午6:10
 */

public class VideoRecorder implements Runnable, ISurfaceRenderer {
    private static final int TIME_OUT = 1000;
    private WindowSurface mInputWindowSurface;
    private EglCore mEglCore;
    private CameraInputFilter mInput;
    private GPUImageFilter mFilter;
    private FloatBuffer gLCubeBuffer;
    private FloatBuffer gLTextureBuffer;
    private OnRecorderListener onRecorderListener;
    private MediaCodec mediaCodec;
    private VideoParams mVideoParams;
    private PrepareParams mPrepareParams;
    private Queue<RawFrame> mRawFrameList;
    private OutputStream mOutput;
    private volatile boolean mIsRenderer = false;
    private volatile boolean mIsPrepare = false;
    private int mPreviewWidth;
    private int mPreviewHeight;
    private byte[] configByte;
    private int frameIndex = 0;
    private long lastFrameTimestamp = 0;

    public void setOnRecorderListener(OnRecorderListener onRecorderListener) {
        this.onRecorderListener = onRecorderListener;
    }

    public void start(VideoParams params) {
        mVideoParams = params;
        if (!mIsRenderer) {
            new Thread(this).start();
        }
    }

    public void stop() {
        mIsRenderer = false;
    }

    @Override
    public void run() {
        mIsRenderer = true;
        mIsPrepare = false;
        if (onRecorderListener != null) {
            onRecorderListener.onStart(mVideoParams.id, OnRecorderListener.TYPE_VIDEO);
        }
        try {
            prepare();
            mIsPrepare = true;
        } catch (IOException e) {
            e.printStackTrace();
        }
        if (mIsPrepare) {
            encode();
        }
        release();
        if (onRecorderListener != null) {
            boolean isSuccess = mIsPrepare;
            File file = new File(mVideoParams.outFile);
            if (file.exists() && file.isFile()) {
                isSuccess = mIsPrepare && true;
            }
            if (isSuccess) {
                onRecorderListener.onSuccess(mVideoParams.id, OnRecorderListener.TYPE_VIDEO);
            } else {
                onRecorderListener.onFail(mVideoParams.id, OnRecorderListener.TYPE_VIDEO);
            }
        }
    }

    private void prepare() throws IOException {
        gLCubeBuffer = ByteBuffer.allocateDirect(TextureRotationUtil.CUBE.length * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
        gLCubeBuffer.put(TextureRotationUtil.CUBE).position(0);
        gLTextureBuffer = ByteBuffer.allocateDirect(TextureRotationUtil.TEXTURE_NO_ROTATION.length * 4).order(ByteOrder.nativeOrder()).asFloatBuffer();
        gLTextureBuffer.put(TextureRotationUtil.TEXTURE_NO_ROTATION).position(0);
        mRawFrameList = new LinkedBlockingQueue<>();
        String mime = transCodecType(mVideoParams.codecType);
        MediaFormat format = MediaFormat.createVideoFormat(mime, mVideoParams.width, mVideoParams.height);
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        format.setInteger(MediaFormat.KEY_BIT_RATE, mVideoParams.bitRate);
        format.setInteger(MediaFormat.KEY_FRAME_RATE, mVideoParams.frameRate);
        if (mVideoParams.keyIFrameInterval > 0) {
            format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, mVideoParams.keyIFrameInterval);
        } else {
            format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);
        }
        mediaCodec = mediaCodec.createEncoderByType(mime);
        mediaCodec.configure(format, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
        Surface surface = mediaCodec.createInputSurface();
        mediaCodec.start();
        while (true) {
            if (mPrepareParams != null) {
                prepareRenderer(surface, mPrepareParams.sharedContext, mPrepareParams.width, mPrepareParams.height, mPrepareParams.filter);
                mPrepareParams = null;
                break;
            } else {
                try {
                    Thread.sleep(10);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
        mOutput = new DataOutputStream(new FileOutputStream(mVideoParams.outFile));
        lastFrameTimestamp = 0;
        frameIndex = 0;
    }

    private void prepareRenderer(Surface surface, EGLContext sharedContext, int width, int height, GPUImageFilter filter) {
        VLog.d("prepare");
        mPreviewWidth = width;
        mPreviewHeight = height;
        mEglCore = new EglCore(sharedContext, EglCore.FLAG_RECORDABLE);
        mInputWindowSurface = new WindowSurface(mEglCore, surface, true);
        mInputWindowSurface.makeCurrent();
        VLog.d("WindowSurface success");
        mInput = new CameraInputFilter();
        mInput.init();
        mFilter = filter;
        if (mFilter != null) {
            mFilter.init();
            mFilter.onInputSizeChanged(mPreviewWidth, mPreviewHeight);
            mFilter.onDisplaySizeChanged(mVideoParams.width, mVideoParams.height);
        }
        gLTextureBuffer.clear();
        gLTextureBuffer.put(TextureRotationUtil.getRotation(Rotation.fromInt(90),
                false, true)).position(0);
        VLog.d("prepare success");
    }

    private String transCodecType(VideoParams.CodecType codecType) {
        String result = MediaFormat.MIMETYPE_VIDEO_AVC;
        switch (codecType) {
            case H264:
                result = MediaFormat.MIMETYPE_VIDEO_AVC;
                break;
            case H265:
                result = MediaFormat.MIMETYPE_VIDEO_HEVC;
                break;
        }
        return result;
    }

    private void encode() {
        while (true) {
            RawFrame frame = pollRawFrame();
            if (frame != null) {
                handFrameUpdate(frame);
            } else {
                try {
                    Thread.sleep(10);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            if (!mIsRenderer) {
                break;
            }
        }
        RawFrame frame = pollRawFrame();
        while (frame != null) {
            handFrameUpdate(frame);
            frame = pollRawFrame();
        }
        mediaCodec.signalEndOfInputStream();
        while (true) {
            VLog.ld().append(" mediaCodec.signalEndOfInputStream()  while (true)").showLog();
            boolean isBreak = encodeAndWrite();
            if (isBreak) {
                break;
            }
        }
    }

    private void handFrameUpdate(RawFrame rawFrame) {
        if (rawFrame == null) {
            return;
        }
        int count = calculateEncodeTimes(rawFrame.timestamp);
        for (int i = 0; i < count; i++) {
            renderer(rawFrame);
        }
    }

    private void renderer(RawFrame rawFrame) {
        VLog.d("handFrameUpdate start");
        mInputWindowSurface.setPresentationTime(computePresentationTime(frameIndex++, mVideoParams.frameRate));
        mInput.setTextureTransformMatrix(rawFrame.transform);
        if (mFilter == null) {
            mInput.onDrawFrame(rawFrame.textureId, gLCubeBuffer, gLTextureBuffer);
        } else {
            mFilter.onDrawFrame(rawFrame.textureId, gLCubeBuffer, gLTextureBuffer);
        }
        mInputWindowSurface.swapBuffers();
        encodeAndWrite();
    }

    private boolean encodeAndWrite() {
        ByteBuffer[] outputBuffers = mediaCodec.getOutputBuffers();
        MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
        int outputBufferIndex = mediaCodec.dequeueOutputBuffer(bufferInfo, TIME_OUT);
        while (outputBufferIndex >= 0) {
            VLog.d("while (outputBufferIndex >= 0)");
            ByteBuffer outputBuffer = outputBuffers[outputBufferIndex];
            byte[] outData = new byte[bufferInfo.size];
            outputBuffer.get(outData);
            try {
                if (bufferInfo.flags == MediaCodec.BUFFER_FLAG_CODEC_CONFIG) {
                    configByte = new byte[bufferInfo.size];
                    configByte = outData;
                } else if (bufferInfo.flags == MediaCodec.BUFFER_FLAG_KEY_FRAME) {
                    byte[] keyframe = new byte[bufferInfo.size + configByte.length];
                    System.arraycopy(configByte, 0, keyframe, 0, configByte.length);
                    System.arraycopy(outData, 0, keyframe, configByte.length, outData.length);
                    mOutput.write(keyframe, 0, keyframe.length);
                } else {
                    mOutput.write(outData, 0, outData.length);
                }
            } catch (Exception e) {
                e.printStackTrace();
            } catch (Error e) {
                e.printStackTrace();
            }
            mediaCodec.releaseOutputBuffer(outputBufferIndex, false);
            outputBufferIndex = mediaCodec.dequeueOutputBuffer(bufferInfo, 1100);
        }

        VLog.d("handFrameUpdate end");
        return (bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0;
    }

    private long computePresentationTime(long frameIndex, int frameRate) {
        long timestamp = 132 + frameIndex * 1000000000 / frameRate;
        VLog.d("timestamp =" + timestamp);
        return timestamp;
    }

    private int calculateEncodeTimes(long timestamp) {
        int result = 0;
        if (lastFrameTimestamp > 0) {
            long preTimestampSpit = 1000 / mVideoParams.positionFrameRate;
            long preTimestamp = lastFrameTimestamp + preTimestampSpit;
            if (timestamp >= preTimestamp) {
                int timestampTimes = (int) (((timestamp - lastFrameTimestamp) * 1.0f) / preTimestampSpit);
                result = timestampTimes;
                lastFrameTimestamp = lastFrameTimestamp + timestampTimes * preTimestampSpit;
            }
        } else {
            result = 1;
            lastFrameTimestamp = timestamp;
        }
        VLog.ld().append("calculateEncodeTimes positionFrameRate")
                .append(mVideoParams.positionFrameRate)
                .append("calculateEncodeTimes positionFrameRate")
                .append(mVideoParams.positionFrameRate)
                .append("calculateEncodeTimes lastFrameTimestamp")
                .append(lastFrameTimestamp)
                .append("calculateEncodeTimes result")
                .append(result)
                .showLog();
        return result;
    }

    private void release() {
        if (mInputWindowSurface != null) {
            mInputWindowSurface.release();
            mInputWindowSurface = null;
        }
        if (mEglCore != null) {
            mEglCore.release();
            mEglCore = null;
        }
        if (mFilter != null) {
            mFilter.destroy();
            mFilter = null;
        }
        if (mInput != null) {
            mInput.destroy();
            mInput = null;
        }
        if (mediaCodec != null) {
            mediaCodec.stop();
            mediaCodec.release();
            mediaCodec = null;
        }
        if (mOutput != null) {
            try {
                mOutput.flush();
                mOutput.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
            mOutput = null;
        }
        if (mRawFrameList != null) {
            mRawFrameList.clear();
            mRawFrameList = null;
        }
    }


    @Override
    public void onRenderer(EGLContext sharedContext, int textureId, float[] transform, long timestamp, int width, int height, FilterType type) {
        if (!mIsRenderer) {
            return;
        }
        VLog.e("onRenderer    (EGLContext sharedContext");
        if (!mIsPrepare) {
            VLog.d("prepare");
            mPrepareParams = new PrepareParams(sharedContext, width, height, FilterFactory.getFilter(AppApplication.get().getApplicationContext(), type));
        }
        if (timestamp == 0) {
            // Seeing this after device is toggled off/on with power button.  The
            // first frame back has a zero timestamp.
            // MPEG4Writer thinks this is cause to abort() in native code, so it's very
            // important that we just ignore the frame.
            return;
        }
        pushRawFrame(new RawFrame(textureId, transform, System.currentTimeMillis()));
        VLog.d("offer success");
    }


    private void pushRawFrame(RawFrame frame) {
        if (frame != null) {
            VLog.d(frame.toString());
            if (mRawFrameList != null) {
//                int len = mRawFrameList.size();
//                if (len < 5) {
                synchronized (mRawFrameList) {
                    mRawFrameList.offer(frame);
                }
//                }
            }
        }
    }

    private RawFrame pollRawFrame() {
        RawFrame frame = null;
        if (mRawFrameList != null) {
            int len = mRawFrameList.size();
            if (len > 0) {
                synchronized (mRawFrameList) {
                    frame = mRawFrameList.poll();
                }
            }
        }
        return frame;
    }

    private static class RawFrame {
        float[] transform;
        long timestamp;
        int textureId;

        public RawFrame(int textureId, float[] transform, long timestamp) {
            this.textureId = textureId;
            this.transform = transform;
            this.timestamp = timestamp;
        }

        @Override
        public String toString() {
            return "RawFrame{" +
                    "transform=" + Arrays.toString(transform) +
                    ", timestamp=" + timestamp +
                    ", textureId=" + textureId +
                    '}';
        }
    }

    private static class PrepareParams {
        EGLContext sharedContext;
        int width;
        int height;
        GPUImageFilter filter;

        public PrepareParams(EGLContext sharedContext, int width, int height, GPUImageFilter filter) {
            this.sharedContext = sharedContext;
            this.width = width;
            this.height = height;
            this.filter = filter;
        }
    }
}
