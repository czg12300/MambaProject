package com.mamba.model.record;

import android.graphics.ImageFormat;
import android.opengl.GLSurfaceView;

import com.framework.utils.FileUtil;
import com.mamba.gloable.FolderManager;
import com.mamba.model.record.camera.CameraImp;
import com.mamba.model.record.encode.VideoCodecHolder;
import com.mamba.model.record.encode.VideoCodecParameters;
import com.mamba.model.record.randerer.CameraRenderer;
import com.mamba.model.record.randerer.gpuimage.filter.GPUImageFilter;

import java.io.IOException;

/**
 * 处理视频录制的业务逻辑
 *
 * @author jake
 * @since 2017/6/21 下午3:56
 */

public class RecordHolder {
    private CameraRenderer cameraRenderer;
    private VideoCodecHolder videoCodecHolder;

    public RecordHolder() {
        cameraRenderer = new CameraRenderer();
        videoCodecHolder = new VideoCodecHolder();
    }

    private VideoCodecParameters createVideoCodecParameters() {
        return VideoCodecParameters.VideoCodecParametersBuilder.create()
                .setBitRate((int) (1.5 * 1024 * 1024))
                .setCodecType(VideoCodecParameters.CodecType.H264)
                .setFrameRate(25)
                .setKeyIFrameInterval(1)
                .setWidth(720)
                .setHeight(1280)
                .setOutFile(getOutFile())
                .build();
    }

    private String getOutFile() {
        String file = FolderManager.ROOT_FOLDER + System.currentTimeMillis() + ".mp4";
        return file;
    }

    private CameraImp.CameraImpParameters createCameraImpParameters() {
        return CameraImp.CameraImpParametersBuilder.create()
                .setPreviewFormat(ImageFormat.NV21)
                .setPreviewSize(new CameraImp.Size(1280, 720))
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
        try {
            videoCodecHolder.prepare(createVideoCodecParameters());
            cameraRenderer.setSurfaceRenderer(videoCodecHolder.getVideoCodecRenderer());
            videoCodecHolder.setPositionFrameRate(6);
            videoCodecHolder.start();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void stopEncode() {
        try {
            videoCodecHolder.stop();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
