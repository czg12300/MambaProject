package com.framework.test;

import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.Toast;


import com.framework.VideoClipJni;
import com.mamba.framework.R;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.util.Calendar;

public class CameraEncodeFragment extends Fragment implements PreviewCallback, Handler.Callback {
    SurfaceView sView;
    RelativeLayout mButtonsLayout;
    RelativeLayout mMainLayout;
    Button mStartButton, mStopButton;
    final int MSG_CHECK_PROESS = 10001;// "msg_check_proess";
    final int MSG_CHECK_TOUCH = 10002;// "msg_check_touch";
    final int MSG_WRITE_YUVDATA = 10003;
    final int MSG_STOP_YUVDATA = 10004;
    private boolean mIsStartPre = false;
    private SurfaceCameraHolder mCameraHolder;
    private Handler mThreadHandler;
    private ImageView mIvResult;
    AudioThread mAudioThread;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        HandlerThread worker = new HandlerThread("worker");
        worker.start();
        mThreadHandler = new Handler(worker.getLooper(), this);
        return inflater.inflate(R.layout.fragment_camera_encode, null);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        mCameraHolder.setRotation(getActivity().getWindowManager().getDefaultDisplay().getRotation());
        String message = newConfig.orientation == Configuration.ORIENTATION_LANDSCAPE ? "屏幕设置为：横屏" : "屏幕设置为：竖屏";
        L.d("onConfigurationChanged" + message);
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        if (mThreadHandler != null) {
            mThreadHandler.getLooper().quit();
        }
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        sView = (SurfaceView) view.findViewById(R.id.surfaceid);
        mCameraHolder = new SurfaceCameraHolder(sView);
        mCameraHolder.getSurfaceHolder().setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        mCameraHolder.setPreviewCallback(this);
        mCameraHolder.setRotation(getActivity().getWindowManager().getDefaultDisplay()
                .getRotation());
//        mCameraHolder.setJpegPictureCallback(myJpegCallback);
        mCameraHolder.addCameraListener(new BaseCameraHolder.CameraListener() {
            @Override
            public void onCameraOpened(Camera camera) {
                Camera.Parameters parameters = mCameraHolder.getDefaultParameters();
                parameters.setPreviewFormat(ImageFormat.YV12);
                mCameraHolder.setCameraParameters(parameters);
            }
        });
        mButtonsLayout = (RelativeLayout) view.findViewById(R.id.buttonsid);
        mStartButton = (Button) view.findViewById(R.id.button1);
        mIvResult = (ImageView) view.findViewById(R.id.iv_result);
        mStartButton.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                Calendar cc = Calendar.getInstance();
                cc.setTimeInMillis(System.currentTimeMillis());
                File picPath = new File(Environment.getExternalStorageDirectory() + File.separator + "AClip/camera");
                if (!picPath.exists()) {
                    picPath.mkdirs();
                }
                File videoPath = new File(picPath, "video");
                if (!videoPath.exists()) {
                    videoPath.mkdirs();
                }
                Camera.Size size = mCameraHolder.getCamera().getParameters().getPreviewSize();
                File file = new File(videoPath, "out.h264");
                File afile = new File(videoPath, "out.aac");
//                File file = new File(videoPath, System.currentTimeMillis() + ".h264");
                int ret = VideoClipJni.videoRecordStart(file.getAbsolutePath(), size.width, size.height,"90",25,12);
                if (ret < 0) {
                    if (file.exists()) {
                        file.delete();
                    }
                    return;
                }
                else {
                    //开始录制
//                    mAudioRecord.startRecording();
                    count = 0;
                    mIsStartPre = true;
                    mStartButton.setEnabled(false);
                    mStopButton.setEnabled(true);
                }
                if(mAudioThread == null || mAudioThread.getRecordFlag()){
                    mAudioThread = new AudioThread(afile.getAbsolutePath());
                    mAudioThread.start();
                }else{
                    Toast.makeText(getActivity(),"已经开始录制音频",Toast.LENGTH_SHORT).show();
                }
            }

        });
        mStopButton = (Button) view.findViewById(R.id.button2);
        mStopButton.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {

                mIsStartPre = false;
                mStartButton.setEnabled(true);
                mStopButton.setEnabled(false);
                mThreadHandler.sendEmptyMessage(MSG_STOP_YUVDATA);
                String v = Environment.getExternalStorageDirectory() + File.separator + "AClip/camera/video/out.h264";
                String a = Environment.getExternalStorageDirectory() + File.separator + "AClip/camera/video/out.aac";
                String m = Environment.getExternalStorageDirectory() + File.separator + "AClip/camera/video/out.mp4";
                if(mAudioThread != null){
                    mAudioThread.setRecordFlag(false);
                }
//                VideoClipJni.muxing(v,a,m);
                //停止录制
//                mAudioRecord.stop();
            }

        });
        mStopButton.setEnabled(false);
        view.findViewById(R.id.add).setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                int max = mCameraHolder.getMaxZoom();
                int position = mCameraHolder.getZoom();
                int temp = max / 10;
                int zoom = position + temp;
                if (zoom > max) {
                    zoom = max;
                }
                L.d("parameters Zoom max=" + max);
                L.d("parameters Zoom position=" + position);
                L.d("parameters Zoom temp=" + temp);
                L.d("parameters Zoom zoom=" + zoom);
                mCameraHolder.changeZoom(zoom);
            }
        });
        view.findViewById(R.id.sub).setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                int max = mCameraHolder.getMaxZoom();
                int position = mCameraHolder.getZoom();
                int temp = max / 10;
                int zoom = position - temp;
                if (zoom < 0) {
                    zoom = 0;
                }
                L.d("parameters Zoom max=" + max);
                L.d("parameters Zoom position=" + position);
                L.d("parameters Zoom temp=" + temp);
                L.d("parameters Zoom zoom=" + zoom);
                mCameraHolder.changeZoom(zoom);
            }
        });
    }

    Camera.PictureCallback myJpegCallback = new Camera.PictureCallback() {

        @Override
        public void onPictureTaken(byte[] data, Camera camera) {
            // 根据拍照所得的数据创建位图
            Bitmap b = null;
            if (null != data) {
                b = BitmapFactory.decodeByteArray(data, 0, data.length);//data是字节数据，将其解析成位图
                Matrix m = new Matrix();
                m.setRotate(90, (float) b.getWidth() / 2, (float) b.getHeight() / 2);
                final Bitmap bm = Bitmap.createBitmap(b, 0, 0, b.getWidth(), b.getHeight(), m, true);
                mIvResult.setImageBitmap(bm);
            }
            // 加载布局文件
        }
    };
    int count = 0;

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
//                camera.addCallbackBuffer(data);
        if (mIsStartPre == true) {
            L.d("onPreviewFrame ,this is frame " + (count++));
            //处理data
            ByteArrayOutputStream baos;
            byte[] rawImage;
            Bitmap bitmap;
            Camera.Size previewSize = camera.getParameters().getPreviewSize();//获取尺寸,格式转换的时候要用到
            BitmapFactory.Options newOpts = new BitmapFactory.Options();
            newOpts.inJustDecodeBounds = true;
            YuvImage yuvimage = new YuvImage(
                    data,
                    ImageFormat.NV21,
                    previewSize.width,
                    previewSize.height,
                    null);
            baos = new ByteArrayOutputStream();
            yuvimage.compressToJpeg(new Rect(0, 0, previewSize.width, previewSize.height), 100, baos);// 80--JPG图片的质量[0-100],100最高
            rawImage = baos.toByteArray();
            //将rawImage转换成bitmap
            BitmapFactory.Options options = new BitmapFactory.Options();
            options.inPreferredConfig = Bitmap.Config.RGB_565;
            bitmap = BitmapFactory.decodeByteArray(rawImage, 0, rawImage.length, options);
//
            mIvResult.setImageBitmap(bitmap);
//            ImageUtils.saveImageData(rawImage);
//
//            Camera.Size p = camera.getParameters().getPictureSize();
//            Camera.Size previewSize = camera.getParameters().getPreviewSize();
//            L.d("camera.getParameters().getPreviewFrameRate()=" + camera.getParameters().getPreviewFrameRate());
//            L.d("camera.getParameters().getJpegQuality()=" + camera.getParameters().getJpegQuality());
//            L.d("camera.getParameters().getPreviewSize()=" + v.width + ":" + v.height);
//            L.d("camera.getParameters().getPictureSize()=" + p.width + ":" + p.height);
//            byte[] dest=new byte[data.length];
//            YV12RotateNegative90(dest,data,previewSize.width,previewSize.height);
//            int bufferReadResult = mAudioRecord.read(mAudioRecordBuffer, 0, mAudioRecordBuffer.length);
//            Message amsg = new Message();
//            Bundle abl = new Bundle();
//            abl.putByteArray("message_audio_data", shorts2Bytes(mAudioRecordBuffer));
//            amsg.setData(abl);
//            amsg.what = MSG_WRITE_PCM;
//            mThreadHandler.sendMessage(amsg);
            Message msg = new Message();
            Bundle bl = new Bundle();
            bl.putByteArray("message_yuv_data", data);
            msg.setData(bl);
            msg.what = MSG_WRITE_YUVDATA;
            mThreadHandler.sendMessage(msg);

//            if (count > 5) {
//                mIsStartPre = false;
//                mStartButton.setEnabled(true);
//                mStopButton.setEnabled(false);
//            }
        }
    }


    @Override
    public boolean handleMessage(Message msg) {
        switch (msg.what) {
            case MSG_WRITE_YUVDATA:
                byte[] bytedata = msg.getData().getByteArray("message_yuv_data");
                if (bytedata != null) {
                    VideoClipJni.videoRecording(bytedata);
                }
                break;
            case MSG_STOP_YUVDATA:
                VideoClipJni.videoRecordEnd();

//                VideoClipJni.audioRecordEnd();
                break;
        }
        return true;
    }
}
