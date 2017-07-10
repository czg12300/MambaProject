package com.mamba.model.record;

import android.graphics.ImageFormat;
import android.opengl.GLSurfaceView;

import com.mamba.gloable.FolderManager;
import com.mamba.model.record.camera.CameraImp;
import com.mamba.model.record.encode.JakeMediaRecorder;
import com.mamba.model.record.encode.audio.AudioCodecParameters;
import com.mamba.model.record.encode.video.VideoCodecParameters;
import com.mamba.model.record.randerer.CameraRenderer;
import com.mamba.model.record.randerer.gpuimage.filter.GPUImageFilter;

/**
 * 处理视频录制的业务逻辑
 *
 * @author jake
 * @since 2017/6/21 下午3:56
 */

public class RecordHolder {
    //    private static final int OUT_WIDTH = 1280;
//    private static final int OUT_HEIGHT = 720;
    private static final int OUT_WIDTH = 720;
    private static final int OUT_HEIGHT = 1280;
    private static final int PREVIEW_WIDTH = 1280;
    private static final int PREVIEW_HEIGHT = 720;
    private CameraRenderer cameraRenderer;
    private JakeMediaRecorder mediaRecorder;

    public RecordHolder() {
        cameraRenderer = new CameraRenderer();
        mediaRecorder = new JakeMediaRecorder();
        cameraRenderer.setFrameAvailableListener(mediaRecorder.getFrameAvailableListener());
    }

    private VideoCodecParameters createVideoCodecParameters() {
        return VideoCodecParameters.Builder.create()
                .setBitRate((int) (2.0 * 1024 * 1024))
                .setCodecType(VideoCodecParameters.CodecType.H264)
                .setFrameRate(25)
                .setSpeedFrameRate(25)
                .setKeyIFrameInterval(4)
                .setWidth(OUT_WIDTH)
                .setHeight(OUT_HEIGHT)
                .setOutFile(getOutH264())
                .build();
    }
    private AudioCodecParameters createAudioCodecParameters() {
        return AudioCodecParameters.Builder.create()
                .setBitRate(57000)
                .setChannelCount(1)
                .setSampleRate(44100)
                .setOutFile(getOutAudio())
                .build();
    }

    private String getOutFile() {
        String file = FolderManager.ROOT_FOLDER + System.currentTimeMillis() + ".mp4";
        return file;
    }

    private String getOutH264() {
        String file = FolderManager.ROOT_FOLDER + System.currentTimeMillis() + ".h264";
        return file;
    }
    private String getOutAudio() {
        String file = FolderManager.ROOT_FOLDER + System.currentTimeMillis() + ".aac";
        return file;
    }

    private CameraImp.CameraImpParameters createCameraImpParameters() {
        return CameraImp.CameraImpParametersBuilder.create()
                .setPreviewFormat(ImageFormat.NV21)
                .setPreviewSize(new CameraImp.Size(PREVIEW_WIDTH, PREVIEW_HEIGHT))
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

    public void startEncode() {
        mediaRecorder.start(getOutFile(), createVideoCodecParameters(), createAudioCodecParameters());
    }

    public void stopEncode() {
        mediaRecorder.stop();
    }
}
