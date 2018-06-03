package com.mamba.model.recordOld;

import android.graphics.ImageFormat;
import android.opengl.GLSurfaceView;
import android.text.TextUtils;

import com.framework.ndk.videoutils.FfmpegFormatUtils;
import com.framework.utils.FileUtil;
import com.mamba.gloable.FolderManager;
import com.mamba.model.VLog;
import com.mamba.model.recordOld.camera.CameraImp;
import com.mamba.model.recordOld.encode.OnRecorderListener;
import com.mamba.model.recordOld.encode.video.VideoParams;
import com.mamba.model.recordOld.encode.video.VideoRecorder;
import com.mamba.model.recordOld.renderer.CameraRenderer;
import com.mamba.model.recordOld.renderer.gpuimage.filter.GPUImageFilter;

import java.util.LinkedList;
import java.util.List;

/**
 * 处理视频录制的业务逻辑
 *
 * @author jake
 * @since 2017/6/21 下午3:56
 */

public class RecordHolder {
    private CameraRenderer cameraRenderer;
    private static final int DEFAULT_FRAME_RATE = 25;
    private VideoRecorder videoRecorder;
    private List<VideoFile> mRecordList;
    private static int sOutWidth = 720;
    private static int sOutHeight = 1280;
    private static int sPreviewWidth = 1280;
    private static int sPreviewHeight = 720;
    private static int sBitrate = (int) (3.1 * 1024 * 1024);

    public enum Speed {
        NORMAL, X2, X3, X4, X5, X6, _X4
    }

    public static void setOutSize(int width, int height) {
        sOutWidth = width;
        sOutHeight = height;
    }

    public static void setPreviewSize(int width, int height) {
        sPreviewWidth = width;
        sPreviewHeight = height;
    }

    public static void setBitrate(int bitrate) {
        sBitrate = bitrate;
    }

    public RecordHolder() {
        cameraRenderer = new CameraRenderer();
        videoRecorder = new VideoRecorder();
        cameraRenderer.setSurfaceRenderer(videoRecorder);
        videoRecorder.setOnRecorderListener(onRecorderListener);
        mRecordList = new LinkedList<>();
    }

    private OnRecorderListener onRecorderListener = new OnRecorderListener() {
        @Override
        public void onStart(long id, int type) {

        }

        @Override
        public void onFail(long id, int type) {
            VLog.ld().append("OnRecorderListener onFail").showLog();
            for (VideoFile vf : mRecordList) {
                if (vf.id == id) {
                    switch (type) {
                        case TYPE_AUDIO:
                            vf.audioRecordState = VideoFile.STATE_FAIL;
                            if (vf.videoRecordState == VideoFile.STATE_SUCCESS || vf.videoRecordState == VideoFile.STATE_FAIL) {
                                FileUtil.deleteFile(vf.h264);
                                mRecordList.remove(vf);
                            }
                            break;
                        case TYPE_VIDEO:
                            vf.videoRecordState = VideoFile.STATE_FAIL;
                            if (vf.audioRecordState == VideoFile.STATE_SUCCESS || vf.audioRecordState == VideoFile.STATE_FAIL) {
                                FileUtil.deleteFile(vf.audio);
                                mRecordList.remove(vf);
                            }
                            break;
                    }

                    break;
                }
            }
        }

        @Override
        public void onSuccess(long id, int type) {
            VLog.ld().append("OnRecorderListener onSuccess").showLog();
            for (VideoFile vf : mRecordList) {
                if (vf.id == id) {
                    switch (type) {
                        case TYPE_AUDIO:
                            vf.audioRecordState = VideoFile.STATE_SUCCESS;
                            if (vf.videoRecordState == VideoFile.STATE_SUCCESS) {
                                FfmpegFormatUtils.format(vf.h264, vf.audio, vf.outFile);
                                mRecordList.remove(vf);
                            }
                            break;
                        case TYPE_VIDEO:
                            vf.videoRecordState = VideoFile.STATE_SUCCESS;
                            if (vf.audioRecordState == VideoFile.STATE_SUCCESS) {
                                FfmpegFormatUtils.format(vf.h264, vf.audio, vf.outFile);
                                mRecordList.remove(vf);
                            } else if (vf.audioRecordState == VideoFile.STATE_NO) {
                                FfmpegFormatUtils.formatVideoStream(vf.h264, vf.outFile);
                                mRecordList.remove(vf);
                            }
                            break;
                    }

                    break;
                }
            }
        }
    };

    private VideoParams createVideoCodecParameters(long id, Speed speed, String outFile) {
        return VideoParams.VideoCodecParametersBuilder.create()
                .setId(id)
                .setBitRate(sBitrate)
                .setCodecType(VideoParams.CodecType.H264)
                .setFrameRate(DEFAULT_FRAME_RATE)
                .setKeyIFrameInterval(1)
                .setWidth(sOutWidth)
                .setHeight(sOutHeight)
                .setOutFile(outFile)
                .setPositionFrameRate(transSpeed(speed))
                .build();
    }

    private String getOutFile() {
        String file = FolderManager.ROOT_FOLDER + System.currentTimeMillis() + ".mp4";
        return file;
    }

    private String getOutH264File() {
        String file = FolderManager.ROOT_FOLDER + System.currentTimeMillis() + ".h264";
        return file;
    }

    private CameraImp.CameraImpParameters createCameraImpParameters() {
        return CameraImp.CameraImpParametersBuilder.create()
                .setPreviewFormat(ImageFormat.NV21)
                .setPreviewSize(new CameraImp.Size(sPreviewWidth, sPreviewHeight))
                .build();
    }

    public CameraImp getCameraImp() {
        return cameraRenderer.getCameraImp();
    }

    public void setGlSurfaceView(GLSurfaceView surfaceView) {
        cameraRenderer.setGLSurfaceView(surfaceView);
        cameraRenderer.getCameraImp().setParameters(createCameraImpParameters());
    }

    public void setFilter(GPUImageFilter filter) {
        cameraRenderer.setFilter(filter);
    }

    public void start(Speed speed) {
        VideoFile videoFile = new VideoFile(System.currentTimeMillis(), getOutH264File(), null, getOutFile());
        VideoParams videoParams = createVideoCodecParameters(videoFile.id, speed, videoFile.h264);
        mRecordList.add(videoFile);
        videoRecorder.start(videoParams);
    }

    private int transSpeed(Speed speed) {
        int frameRate = DEFAULT_FRAME_RATE;
        switch (speed) {
            case NORMAL:
                frameRate /= 1;
                break;
            case X2:
                frameRate /= 2;
                break;
            case X3:
                frameRate /= 3;
                break;
            case X4:
                frameRate /= 4;
                break;
            case X5:
                frameRate /= 5;
                break;
            case X6:
                frameRate /= 6;
                break;
            case _X4:
                frameRate *= 4;
                break;
        }
        return frameRate;
    }

    public void stop() {
        videoRecorder.stop();
    }


    private static class VideoFile {
        public static final int STATE_NO = -0x01;
        public static final int STATE_START = 0x01;
        public static final int STATE_SUCCESS = 0x02;
        public static final int STATE_FAIL = 0x03;
        long id;
        String h264;
        String audio;
        String outFile;
        int audioRecordState;
        int videoRecordState;

        public VideoFile(long id, String h264, String audio, String outFile) {
            this.id = id;
            this.h264 = h264;
            this.audio = audio;
            this.outFile = outFile;
            audioRecordState = !TextUtils.isEmpty(audio) ? STATE_START : STATE_NO;
            videoRecordState = !TextUtils.isEmpty(h264) ? STATE_START : STATE_NO;
        }
    }
}
