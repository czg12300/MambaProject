package com.mamba.model.record.encode;

import android.text.TextUtils;

import com.framework.ndk.videoutils.FfmpegFormatUtils;
import com.framework.utils.FileUtil;
import com.mamba.model.record.encode.audio.AudioCodecParameters;
import com.mamba.model.record.encode.video.IFrameAvailableListener;
import com.mamba.model.record.encode.video.VideoCodecHolder;
import com.mamba.model.record.encode.video.VideoCodecParameters;
import com.mamba.model.record.encode.video.VideoFrame;

import java.io.File;
import java.util.Vector;

/**
 * @author jake
 * @since 2017/7/6 上午10:31
 */

public class JakeMediaRecorder {
    private VideoCodecHolder mVideoCodecHolder;
    private String mOutFile;
    private String mOutVideo;
    private String mOutAudio;
    private boolean hasRecordAudio = false;

    public JakeMediaRecorder() {
        mVideoCodecHolder = new VideoCodecHolder();
    }

    public void start(String outFile, VideoCodecParameters videoCodecParameters, AudioCodecParameters audioCodecParameters) {
        mOutFile = outFile;
        mOutVideo = videoCodecParameters.outFile;
        mOutAudio = videoCodecParameters.outFile;
        mVideoCodecHolder.start(videoCodecParameters);
        mVideoCodecHolder.setRecorderCallback(recorderCallback);
    }

    public void stop() {
        mVideoCodecHolder.stop();
    }

    private RecorderCallback recorderCallback = new RecorderCallback() {
        Vector<Boolean> finish = new Vector<>();

        @Override
        public void onStart() {

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
                }
            } else {
                boolean isSuccess = FfmpegFormatUtils.formatVideoStream(mOutVideo, mOutFile);
                FileUtil.deleteFile(mOutVideo);
                if (!isSuccess) {
                    FileUtil.deleteFile(mOutFile);
                }
            }
        }
    };
    private IFrameAvailableListener frameAvailableListener = new IFrameAvailableListener() {
        @Override
        public void onFrameAvailable(VideoFrame frame) {
            if (mVideoCodecHolder != null) {
                mVideoCodecHolder.onFrameAvailable(frame);
            }
        }
    };

    public IFrameAvailableListener getFrameAvailableListener() {
        return frameAvailableListener;
    }

}
