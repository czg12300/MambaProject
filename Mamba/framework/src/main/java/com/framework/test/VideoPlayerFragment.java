package com.framework.test;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.provider.MediaStore;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;


import com.framework.VideoClipJni;
import com.mamba.framework.R;

import java.io.File;

/**
 * @author jake
 * @since 2016/12/30 下午3:04
 */

public class VideoPlayerFragment extends Fragment implements View.OnClickListener, SurfaceHolder.Callback {
    private Button mBtnChooseFile;
    private Button mBtnStart;
    private Button mBtnPause;
    private Button mBtnStop;
    private TextView evPath;
    private Handler uiHandler;
    private Handler threadHandler;
    private HandlerThread handlerThread;
    private SurfaceView surfaceView;
    private SurfaceHolder surfaceHolder;
    private JakeMediaPlayer mPlayer;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        handlerThread = new HandlerThread("work");
        handlerThread.start();
        threadHandler = new Handler(handlerThread.getLooper(), new Handler.Callback() {
            @Override
            public boolean handleMessage(Message msg) {
                handlerThreadMessage(msg);
                return false;
            }
        });
        uiHandler = new Handler(new Handler.Callback() {
            @Override
            public boolean handleMessage(Message msg) {
                handleUiMessage(msg);
                return false;
            }
        });
        return inflater.inflate(R.layout.fragment_videoplayer, null);
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        if (threadHandler != null) {
            threadHandler.getLooper().quit();
        }
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        findViews(view);
        setListener();
        mPlayer = new JakeMediaPlayer();
        evPath.setText(getVideoPathFromCache());
        surfaceHolder = surfaceView.getHolder();
        surfaceHolder.addCallback(this);

    }

    private void setListener() {
        mBtnChooseFile.setOnClickListener(this);
        mBtnStart.setOnClickListener(this);
        mBtnPause.setOnClickListener(this);
        mBtnStop.setOnClickListener(this);
    }

    private void findViews(View view) {
        mBtnChooseFile = (Button) view.findViewById(R.id.btn_choose);
        mBtnStart = (Button) view.findViewById(R.id.btn_key_frame);
        mBtnPause = (Button) view.findViewById(R.id.btn_demuxing);
        mBtnStop = (Button) view.findViewById(R.id.btn_muxing);
        evPath = (TextView) view.findViewById(R.id.ev_path);
        surfaceView = (SurfaceView) view.findViewById(R.id.surface);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        Log.d("tag", "onActivityResult");
        if (resultCode == Activity.RESULT_OK) {//是否选择，没选择就不会继续
            Uri uri = data.getData();//得到uri，后面就是将uri转化成file的过程。
            String path = getPath(getActivity(), uri);
            cacheVideoPath(path);
            evPath.setText(path);
        }
    }

    private String getVideoPathFromCache() {
        return getActivity().getSharedPreferences("video", Context.MODE_PRIVATE).getString("video_path_", null);
    }

    private void cacheVideoPath(String path) {
        SharedPreferences sp = getActivity().getSharedPreferences("video", Context.MODE_PRIVATE);
        sp.edit().putString("video_path_", path).apply();
    }

    private String getPath(Context context, Uri uri) {
        String[] filePathColumn = {MediaStore.Video.Media.DATA};
        Cursor cursor = context.getContentResolver().query(uri, null, null, null, null);
        cursor.moveToFirst();
        String videoPath = cursor.getString(1);
        cursor.close();
        return videoPath;
    }

    @Override
    public void onClick(View v) {
        if (v == mBtnChooseFile) {
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
            intent.setType("video/*");//设置类型，我这里是任意类型，任意后缀的可以这样写。
            startActivityForResult(intent, 1);
        } else if (v == mBtnStart) {
            play(getVideoPathFromCache(), surfaceHolder.getSurface());
//            mPlayer.setDataSource(getVideoPathFromCache());
//            mPlayer.prepare();
//            mPlayer.start();
        } else if (v == mBtnPause) {
            mPlayer.pause();
        } else if (v == mBtnStop) {
            mPlayer.stop();
        }
    }

    private String getVideoName(String path) {
        return path.substring(path.lastIndexOf("/"), path.length() - 4);
    }

    private static final int MSG_SHOW_PB = 0x1;
    private static final int MSG_HIDE_PB = 0x2;
    private static final int MSG_SHOW_RESULT = 0x3;
    private static final int MSG_THREAD_EXTRACT_VIDEO = 0x1;
    private static final int MSG_THREAD_DEMUXING = 0x2;
    private static final int MSG_THREAD_MUXING = 0x3;
    private static final int MSG_THREAD_CUT = 0x4;
    private static final int FLAG_KEY_FRAME = 0x1;
    private static final int FLAG_DEMUXING = 0x2;

    private void handleUiMessage(Message msg) {
        switch (msg.what) {
            case MSG_SHOW_PB:
                break;
            case MSG_HIDE_PB:
                break;
            case MSG_SHOW_RESULT:
                break;
        }
    }

    private void handlerThreadMessage(Message msg) {
        String videoPath = (String) msg.obj;
        File picPath = new File(Environment.getExternalStorageDirectory() + "/AClip", getVideoName(videoPath));
        if (!picPath.exists()) {
            picPath.mkdirs();
        }
        String path = picPath.getAbsolutePath();
        String result;
        Log.d("ndk-log", "   path=" + path);
        Message message = uiHandler.obtainMessage();
        switch (msg.what) {
            case MSG_THREAD_EXTRACT_VIDEO:
                result = ""+VideoClipJni.demuxingKeyFrame(videoPath, path);
                message.obj = result;
                message.what = MSG_SHOW_RESULT;
                message.sendToTarget();
                break;
            case MSG_THREAD_DEMUXING:
                result = ""+VideoClipJni.demuxing(videoPath, path, path);
                message.obj = result;
                message.what = MSG_SHOW_RESULT;
                message.sendToTarget();
                break;
            case MSG_THREAD_MUXING:
                result = "" + VideoClipJni.muxing(path + "/out.h264", path + "/out.aac", path + "/out.mp4");
                message.obj = result;
                message.what = MSG_SHOW_RESULT;
                message.sendToTarget();
                break;
        }
    }

    public native int play(String path, Surface surface);

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
//        mPlayer.setSurface(surfaceHolder.getSurface());

    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {

    }
}
