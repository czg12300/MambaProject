package com.mamba.model.record.encode;

import com.framework.ndk.videoutils.FfmpegFormatUtils;
import com.framework.utils.FileUtil;
import com.mamba.model.VLog;
import com.mamba.model.record.encode.audio.AudioCodecHolder;
import com.mamba.model.record.encode.audio.AudioCodecParameters;
import com.mamba.model.record.encode.video.OnVideoFrameAvailableListener;
import com.mamba.model.record.encode.video.VideoCodecHolder;
import com.mamba.model.record.encode.video.VideoCodecParameters;
import com.mamba.model.record.encode.video.VideoFrame;

import java.util.Vector;

/**
 * 视频录制器
 *
 * @author jake
 * @since 2017/7/6 上午10:31
 */

public class JakeMediaRecorder {
    private VideoCodecHolder mVideoCodecHolder;
    private String mOutFile;
    private String mOutVideo;
    private String mOutAudio;
    private boolean hasRecordAudio = false;
    private AudioCodecHolder mAudioCodecHolder;

    public JakeMediaRecorder() {
        mVideoCodecHolder = new VideoCodecHolder();
        mVideoCodecHolder.setRecorderCallback(recorderCallback);
        mAudioCodecHolder = new AudioCodecHolder();
        mAudioCodecHolder.setRecorderCallback(recorderCallback);
    }

    public void start(String outFile, VideoCodecParameters videoCodecParameters, AudioCodecParameters audioCodecParameters) {
        mOutFile = outFile;
        mOutVideo = videoCodecParameters.outFile;
        mVideoCodecHolder.start(videoCodecParameters);
        if (audioCodecParameters != null) {
            mOutAudio = audioCodecParameters.outFile;
            hasRecordAudio = true;
            mAudioCodecHolder.start(audioCodecParameters);
        } else {
            hasRecordAudio = false;
        }

    }

    public void stop() {
        mVideoCodecHolder.stop();
        mAudioCodecHolder.stop();
    }

    private RecorderCallback recorderCallback = new RecorderCallback() {
        Vector<Boolean> finish = new Vector<>();

        @Override
        public void onStart() {
            if (callback != null) {
                callback.onStart();
            }
        }

        @Override
        public void onStop() {
            if (hasRecordAudio) {
                int len = 0;
                synchronized (finish) {
                    finish.add(true);
                    len = finish.size();
                }
                if (len > 1) {
                    finish.clear();
                    boolean isSuccess = FfmpegFormatUtils.format(mOutVideo, mOutAudio, mOutFile);
                    FileUtil.deleteFile(mOutVideo);
                    FileUtil.deleteFile(mOutAudio);
                    if (!isSuccess) {
                        FileUtil.deleteFile(mOutFile);
                    }
                    if (callback != null) {
                        callback.onStop(isSuccess ? mOutFile : null, isSuccess);
                    }
                }
            } else {
                boolean isSuccess = FfmpegFormatUtils.formatVideoStream(mOutVideo, mOutFile);
                FileUtil.deleteFile(mOutVideo);
                if (!isSuccess) {
                    FileUtil.deleteFile(mOutFile);
                }
                if (callback != null) {
                    callback.onStop(isSuccess ? mOutFile : null, isSuccess);
                }
            }
        }
    };
    private OnVideoFrameAvailableListener frameAvailableListener = new OnVideoFrameAvailableListener() {
        @Override
        public void onFrameAvailable(VideoFrame frame) {
            if (mVideoCodecHolder != null) {
                mVideoCodecHolder.onFrameAvailable(frame);
            }
        }
    };

    public OnVideoFrameAvailableListener getFrameAvailableListener() {
        return frameAvailableListener;
    }

    public Callback callback;

    public void setCallback(Callback callback) {
        this.callback = callback;
    }

    public static interface Callback {
        void onStart();

        void onStop(String outFile, boolean isSuccess);
    }
}
