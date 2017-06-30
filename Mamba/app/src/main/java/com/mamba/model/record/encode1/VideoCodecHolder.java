package com.mamba.model.record.encode1;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.support.annotation.NonNull;
import android.text.TextUtils;
import android.view.Surface;

import com.framework.ndk.videoutils.FfmpegFormatUtils;
import com.framework.utils.FileUtil;
import com.mamba.model.VLog;
import com.mamba.model.record.encode.VideoCodecParameters;
import com.mamba.model.record.encode.VideoCodecRenderer;

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

public class VideoCodecHolder implements IFrameAvailableListener {
    private MediaCodec mEncoder;
    private DataOutputStream mOutput;
    private int positionFrameRate;
    private long lastFrameTimestamp = 0;
    private boolean isEncoding = false;
    private VideoCodecParameters mVideoCodecParameters;
    private MediaCodecEncodeTask encodeTask;

    public VideoCodecHolder() {
        encodeTask = new MediaCodecEncodeTask();
        encodeTask.setCallback(encodeCallback);
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
        MediaFormat format = MediaFormat.createVideoFormat(transCodecType(), mVideoCodecParameters.width, mVideoCodecParameters.height);
        format.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible);
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

    public void start(VideoCodecParameters parameters) throws IOException {
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
        mEncoder.start();
        encodeTask.startTask(mEncoder);
        isEncoding = true;
    }

    private String getTempH264File(String outFile) {
        if (TextUtils.isEmpty(outFile)) {
            return null;
        }
        File file = new File(outFile);
        File h264 = new File(file.getParent(), file.getName() + "temp.h264");
        return h264.getAbsolutePath();
    }


    public void stop() throws IOException {
        if (!isEncoding) {
            return;
        }
        isEncoding = false;
        encodeTask.stopTask();
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

    private MediaCodecEncodeTask.Callback encodeCallback = new MediaCodecEncodeTask.Callback() {
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
        public void onEncode(byte[] data) {
            if (data == null || data.length <= 0) {
                return;
            }
            try {
                mOutput.write(data);
            } catch (Exception e) {
                e.printStackTrace();
            } catch (Error e) {
                e.printStackTrace();
            }
        }

        @Override
        public void onFinish() {
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

    @Override
    public void onFrameAvailable(VideoFrame frame) {
        if (isEncoding && encodeTask != null) {
            encodeTask.addRawData(frame);
        }
    }
}
