package com.mamba.model.record;

import android.graphics.ImageFormat;
import android.opengl.GLSurfaceView;

import com.framework.utils.FileUtil;
import com.mamba.gloable.FolderManager;
import com.mamba.model.record.camera.CameraImp;
import com.mamba.model.record.encode1.VideoCodecHolder;
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
    private static final int OUT_WIDTH = 90;
    private static final int OUT_HEIGHT = 160;
//    private static final int OUT_WIDTH = 720;
//    private static final int OUT_HEIGHT = 1280;
    private static final int PREVIEW_WIDTH = 1280;
    private static final int PREVIEW_HEIGHT = 720;
    private CameraRenderer cameraRenderer;
    private VideoCodecHolder videoCodecHolder;

    public RecordHolder() {
        cameraRenderer = new CameraRenderer();
        videoCodecHolder = new VideoCodecHolder();
        cameraRenderer.setFrameAvailableListener(videoCodecHolder);
        cameraRenderer.setOutputSize(OUT_WIDTH, OUT_HEIGHT);
    }

    private VideoCodecParameters createVideoCodecParameters() {
        return VideoCodecParameters.VideoCodecParametersBuilder.create()
                .setBitRate((int) (1.5 * 1024 * 1024))
                .setCodecType(VideoCodecParameters.CodecType.H264)
                .setFrameRate(25)
                .setKeyIFrameInterval(1)
                .setWidth(OUT_WIDTH)
                .setHeight(OUT_HEIGHT)
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
        try {
            videoCodecHolder.start(createVideoCodecParameters());


//            videoCodecHolder.setPositionFrameRate(100);
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
