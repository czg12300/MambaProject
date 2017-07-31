package com.mamba.model.record;

import android.app.Activity;
import android.content.Intent;
import android.graphics.ImageFormat;
import android.media.AudioFormat;
import android.media.MediaRecorder;
import android.opengl.GLSurfaceView;

import com.mamba.gloable.FolderManager;
import com.mamba.model.VLog;
import com.mamba.model.record.camera.CameraImp;
import com.mamba.model.record.encode.JakeMediaRecorder;
import com.mamba.model.record.encode.audio.AudioCodecParameters;
import com.mamba.model.record.encode.video.VideoCodecHolder;
import com.mamba.model.record.encode.video.VideoCodecParameters;
import com.mamba.model.record.randerer.CameraRenderer;
import com.mamba.model.record.randerer.gpuimage.filter.GPUImageFilter;
import com.mamba.ui.PlayActivity;
import com.mamba.ui.test.MainActivity;

import java.io.File;
import java.lang.ref.WeakReference;

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
    private WeakReference<Activity> activityWeakReference;

    public RecordHolder(Activity activity) {
        VideoCodecHolder.useMediaCodec = true;
        activityWeakReference = new WeakReference<Activity>(activity);
        cameraRenderer = new CameraRenderer();
        mediaRecorder = new JakeMediaRecorder();
        cameraRenderer.setFrameAvailableListener(mediaRecorder.getFrameAvailableListener());
        mediaRecorder.setCallback(callback);
    }

    private VideoCodecParameters createVideoCodecParameters() {
        return VideoCodecParameters.Builder.create()
                .setBitRate((int) (2.0 * 1024 * 1024))
                .setCodecType(VideoCodecParameters.CodecType.H264)
                .setFrameRate(25)//硬编码一般支持25和30帧，其他帧率不支持，如果导致视频播放速度不正确
                .setSpeedFrameRate(25)
                .setKeyIFrameInterval(4)
                .setWidth(OUT_WIDTH)
                .setHeight(OUT_HEIGHT)
                .setOutFile(getOutH264())
                .build();
    }

    private AudioCodecParameters createAudioCodecParameters() {
        return AudioCodecParameters.Builder.create()
                .setBitRate(96000)
                .setAudioSource(MediaRecorder.AudioSource.MIC)
                .setChannelLayout(AudioFormat.CHANNEL_IN_MONO)
                .setAudioFormat(AudioFormat.ENCODING_PCM_16BIT)
                .setSpeed(1.0f)
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

    private JakeMediaRecorder.Callback callback = new JakeMediaRecorder.Callback() {
        @Override
        public void onStart() {

        }

        @Override
        public void onStop(String outFile, boolean isSuccess) {
            VLog.d("JakeMediaRecorder  onStop outFile:" + outFile);
            if (isSuccess) {
                if (activityWeakReference != null && activityWeakReference.get() != null) {
//                    File file = new File(outFile);
//                    outFile.replace(file.getName(), "out.mp4");
                    Intent intent = new Intent(activityWeakReference.get(), PlayActivity.class);
                    intent.putExtra("out_file", outFile);
                    activityWeakReference.get().startActivity(intent);
                }
            }
        }
    };

    public void resume() {
        if (cameraRenderer.getCameraImp() != null) {
            cameraRenderer.getCameraImp().startPreview();
        }
    }

    public void pause() {
        if (cameraRenderer.getCameraImp() != null) {
            cameraRenderer.getCameraImp().stopPreview();
        }
        mediaRecorder.stop();
    }
}
