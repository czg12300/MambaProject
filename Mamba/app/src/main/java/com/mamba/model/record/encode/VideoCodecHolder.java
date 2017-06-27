package com.mamba.model.record.encode;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.support.annotation.NonNull;
import android.support.v4.app.NavUtils;
import android.text.TextUtils;
import android.view.Surface;

import com.framework.ndk.videoutils.FfmpegFormatUtils;
import com.framework.utils.FileUtil;
import com.mamba.model.VLog;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * 编码器持有类
 *
 * @author jake
 * @since 2017/6/21 下午5:18
 */

public class VideoCodecHolder {
    private final int TIMEOUT_USEC = 10000;
    private MediaCodec mEncoder;
    private VideoCodecRenderer mVideoCodecRenderer;
    private DataOutputStream mOutput;
    private int positionFrameRate;
    private long lastFrameTimestamp = 0;
    private MediaCodec.BufferInfo mBufferInfo;
    private boolean isEncoding = false;
    private Surface mInputSurface;
    private VideoCodecParameters mVideoCodecParameters;

    public VideoCodecHolder() {

    }

    public void setPositionFrameRate(int positionFrameRate) {
        if (positionFrameRate > 0) {
            this.positionFrameRate = positionFrameRate;
        }
    }

    public int getPositionFrameRate() {
        return positionFrameRate;
    }

    @NonNull
    private MediaFormat getMediaFormat() {
        mBufferInfo = new MediaCodec.BufferInfo();
        MediaFormat format = MediaFormat.createVideoFormat(transCodecType(), mVideoCodecParameters.width, mVideoCodecParameters.height);
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatSurface);
        format.setInteger(MediaFormat.KEY_BIT_RATE, mVideoCodecParameters.bitRate);
        format.setInteger(MediaFormat.KEY_FRAME_RATE, mVideoCodecParameters.frameRate);
//        format.setInteger(MediaFormat.KEY_CAPTURE_RATE, mVideoCodecParameters.frameRate);
        format.setInteger(MediaFormat.KEY_REPEAT_PREVIOUS_FRAME_AFTER, 40);
        if (mVideoCodecParameters.keyIFrameInterval > 0) {
            format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, mVideoCodecParameters.keyIFrameInterval);
        } else {
            format.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);
        }
        return format;
    }

    private String transCodecType() {
        String result = MediaFormat.MIMETYPE_VIDEO_AVC;
        if (mVideoCodecParameters != null) {
            switch (mVideoCodecParameters.codecType) {
                case H264:
                    result = MediaFormat.MIMETYPE_VIDEO_AVC;
                    break;
                case H265:
                    result = MediaFormat.MIMETYPE_VIDEO_HEVC;
                    break;
            }
        }
        return result;
    }

    public void prepare(VideoCodecParameters parameters) throws IOException {
        if (!isEncoding) {
            stop();
        }
        mVideoCodecParameters = parameters;
        if (mVideoCodecParameters == null) {
            throw new NullPointerException("VideoCodecParameters is null,you have to setup VideoCodecParameters");
        }
        setPositionFrameRate(mVideoCodecParameters.frameRate);
        mEncoder = mEncoder.createEncoderByType(transCodecType());
        mEncoder.configure(getMediaFormat(), null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
        mInputSurface = mEncoder.createInputSurface();
        mEncoder.start();
        mVideoCodecRenderer = new VideoCodecRenderer(mInputSurface, mVideoCodecParameters.width, mVideoCodecParameters.height);
        mVideoCodecRenderer.setOnRendererListener(onRendererListener);
    }

    private String getTempH264File(String outFile) {
        if (TextUtils.isEmpty(outFile)) {
            return null;
        }
        File file = new File(outFile);
        File h264 = new File(file.getParent(), file.getName() + "temp.h264");
        return h264.getAbsolutePath();
    }

    public void start() {
        mVideoCodecRenderer.start();
        isEncoding = true;
    }


    public VideoCodecRenderer getVideoCodecRenderer() {
        return mVideoCodecRenderer;
    }

    public void stop() throws IOException {
        if (!isEncoding) {
            return;
        }
        mVideoCodecRenderer.stopEncode();
        isEncoding = false;

    }


    private int calculateEncodeTimes(long timestamp) {
        int result = 0;
        if (lastFrameTimestamp > 0) {
            long preTimestampSpit = 1000 / positionFrameRate;
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
        VLog.d("calculateEncodeTimes positionFrameRate" + positionFrameRate);
        VLog.d("calculateEncodeTimes lastFrameTimestamp" + lastFrameTimestamp);
        VLog.d("calculateEncodeTimes result" + result);
        return result;
    }

    private VideoCodecRenderer.OnRendererListener onRendererListener = new VideoCodecRenderer.OnRendererListener() {
        @Override
        public int getRendererFrameTimes(long timestamp) {
            return calculateEncodeTimes(timestamp);
        }

        @Override
        public int getPositionFrameRate() {
            return positionFrameRate;
        }

        @Override
        public void onStart() {
            lastFrameTimestamp = 0;
            VLog.d("onStart");
            try {
                File h264 = new File(getTempH264File(mVideoCodecParameters.outFile));
                FileUtil.createFile(h264.getAbsolutePath(), true);
                mOutput = new DataOutputStream(new FileOutputStream(h264));
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onRenderer() {
            encode(false);

        }

        @Override
        public void onStop() {
            VLog.d("onStop");
            //Flush encoder
            mEncoder.signalEndOfInputStream();
            encode(true);
            mEncoder.stop();
            mEncoder.release();
            if (mOutput != null) {
                try {
                    mOutput.flush();
                    mOutput.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }

            }
            String h264 = getTempH264File(mVideoCodecParameters.outFile);
            if (FileUtil.isExist(h264)) {
                boolean isSuccess = FfmpegFormatUtils.formatVideoStream(h264, mVideoCodecParameters.outFile);
                FileUtil.deleteFile(h264);
                if (!isSuccess) {
                    FileUtil.deleteFile(mVideoCodecParameters.outFile);
                }
            }
        }
    };

    private void encode(boolean endOfStream) {
        VLog.d("encode    endOfStream" + endOfStream);
        ByteBuffer[] encoderOutputBuffers = mEncoder.getOutputBuffers();
        while (true) {
            int encoderStatus = mEncoder.dequeueOutputBuffer(mBufferInfo, TIMEOUT_USEC);
            VLog.d("encode    encoderStatus" + encoderStatus);
            if (encoderStatus == MediaCodec.INFO_TRY_AGAIN_LATER) {
                // no output available yet
                if (!endOfStream) {
                    break;      // out of while
                } else {
                    VLog.d("no output available, spinning to await EOS");
                }
            } else if (encoderStatus == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
                // not expected for an encoder
                encoderOutputBuffers = mEncoder.getOutputBuffers();
            } else if (encoderStatus == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                // should happen before receiving buffers, and should only happen once
//                if (mMuxerStarted) {
//                    throw new RuntimeException("format changed twice");
//                }
//                MediaFormat newFormat = mEncoder.getOutputFormat();
//                VLog.d("encoder output format changed: " + newFormat);
//
//                // now that we have the Magic Goodies, start the muxer
//                mTrackIndex = mMuxer.addTrack(newFormat);
//                mMuxer.start();
//                mMuxerStarted = true;
            } else if (encoderStatus < 0) {
                VLog.d("unexpected result from encoder.dequeueOutputBuffer: " + encoderStatus);
                // let's ignore it
            } else {
                ByteBuffer encodedData = encoderOutputBuffers[encoderStatus];
                if (encodedData == null) {
                    throw new RuntimeException("encoderOutputBuffer " + encoderStatus +
                            " was null");
                }

//                if ((mBufferInfo.flags & MediaCodec.BUFFER_FLAG_CODEC_CONFIG) != 0) {
//                    // The codec config data was pulled out and fed to the muxer when we got
//                    // the INFO_OUTPUT_FORMAT_CHANGED status.  Ignore it.
//                    VLog.d("ignoring BUFFER_FLAG_CODEC_CONFIG");
//                    mBufferInfo.size = 0;
//                }

                if (mBufferInfo.size != 0) {
//                 adjust the ByteBuffer values to match BufferInfo (not needed?)
                    encodedData.position(mBufferInfo.offset);
                    encodedData.limit(mBufferInfo.offset + mBufferInfo.size);
                    try {
                        byte[] bytes = new byte[mBufferInfo.size];
                        encodedData.get(bytes);
//                        mOutput.writeInt(mBufferInfo.size);
                        mOutput.write(bytes);
                        encodedData.clear();
                    } catch (Exception e) {
                        e.printStackTrace();
                    } catch (Error e) {
                        e.printStackTrace();
                    }
                    VLog.d("sent " + mBufferInfo.size + " bytes to muxer, ts=" +
                            mBufferInfo.presentationTimeUs);
                }

                mEncoder.releaseOutputBuffer(encoderStatus, false);

                if ((mBufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                    if (!endOfStream) {
                        VLog.d("reached end of stream unexpectedly");
                    } else {
                        VLog.d("end of stream reached");
                    }
                    break;      // out of while
                }
            }
        }
    }
}
