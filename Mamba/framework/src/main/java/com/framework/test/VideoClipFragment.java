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
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;


import com.framework.PathGeneratorUtil;
import com.framework.VideoClipJni;
import com.mamba.framework.R;

import java.io.File;

/**
 * @author jake
 * @since 2016/12/30 下午3:04
 */

public class VideoClipFragment extends Fragment implements View.OnClickListener {
    private Button mBtnChooseFile;
    private Button mBtnKeyFrame;
    private Button mBtnDemuxing;
    private Button mBtnMuxing;
    private Button mBtnRemuxer;
    private Button mBtnCut;
    private Button mBtnRate;
    private Button mBtnAddWatermark;
    private TextView evPath;
    private ProgressBar progressBar;
    private TextView tvResult;
    private Handler uiHandler;
    private Handler threadHandler;
    private HandlerThread handlerThread;
    private EditText evStart;
    private EditText evEnd;
    private EditText evRate;
    private EditText evMoment;
    private TextView evReEncode;
    private Button btnReEncode;
    private boolean cutAudio = false;
    private boolean isSlowVideoSpeed = false;
    private boolean isSlowVideoMoment = false;
    private Button mBtnMerge,mBtnReverse;
    private Button mBtnMoment;
    private TextView mTvSrc1, mTvSrc2,mTvReverse;
    private final String TAG = "VideoClipFragment";

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
        return inflater.inflate(R.layout.fragment_videoclip, null);
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
        evPath.setText(getVideoPathFromCache());
        String text = VideoClipJni.sayHello("jake");
        VideoClipJni.helloJni("jake say ,hello jni!");
        tvResult.setText(text);
    }

    private void setListener() {
        mBtnChooseFile.setOnClickListener(this);
        mBtnKeyFrame.setOnClickListener(this);
        mBtnDemuxing.setOnClickListener(this);
        mBtnMuxing.setOnClickListener(this);
        mBtnCut.setOnClickListener(this);
        mBtnRate.setOnClickListener(this);
        mBtnAddWatermark.setOnClickListener(this);
        mBtnRemuxer.setOnClickListener(this);
        mBtnMoment.setOnClickListener(this);
        mBtnReverse.setOnClickListener(this);
    }

    private void findViews(View view) {
        tvResult = (TextView) view.findViewById(R.id.tv_hello);
        mBtnChooseFile = (Button) view.findViewById(R.id.btn_choose);
        mBtnKeyFrame = (Button) view.findViewById(R.id.btn_key_frame);
        mBtnDemuxing = (Button) view.findViewById(R.id.btn_demuxing);
        mBtnMuxing = (Button) view.findViewById(R.id.btn_muxing);
        mBtnRate = (Button) view.findViewById(R.id.btn_rate);
        mBtnMoment = (Button) view.findViewById(R.id.btn_moment);
        mBtnCut = (Button) view.findViewById(R.id.btn_cut);
        mBtnRemuxer = (Button) view.findViewById(R.id.btn_remuxing);
        mBtnAddWatermark = (Button) view.findViewById(R.id.btn_add_watermark);
        evPath = (TextView) view.findViewById(R.id.ev_path);
        evEnd = (EditText) view.findViewById(R.id.ev_end);
        evMoment = (EditText) view.findViewById(R.id.ev_moment);
        evStart = (EditText) view.findViewById(R.id.ev_start);
        progressBar = (ProgressBar) view.findViewById(R.id.progressBar);
        evReEncode = (TextView) view.findViewById(R.id.tv_reencode);
        btnReEncode = (Button) view.findViewById(R.id.btn_reencode);
        btnReEncode.setOnClickListener(this);
        evRate = (EditText) view.findViewById(R.id.ev_rate);
        ((Switch) view.findViewById(R.id.av_switch)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                cutAudio = isChecked;
            }
        });
        ((Switch) view.findViewById(R.id.rate_switch)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                isSlowVideoSpeed = isChecked;
            }
        });
        ((Switch) view.findViewById(R.id.moment_switch)).setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                isSlowVideoMoment = isChecked;
            }
        });
        mBtnMerge = (Button) view.findViewById(R.id.btn_merge);
        mTvSrc1 = (TextView) view.findViewById(R.id.tv_src1);
        mTvSrc2 = (TextView) view.findViewById(R.id.tv_src2);
        mBtnMerge.setOnClickListener(this);
        mTvReverse = (TextView) view.findViewById(R.id.tv_reverse);
        mBtnReverse = (Button) view.findViewById(R.id.btn_reverse);
        mBtnReverse.setOnClickListener(this);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode == Activity.RESULT_OK) {//是否选择，没选择就不会继续
            Uri uri = data.getData();//得到uri，后面就是将uri转化成file的过程。
            Log.d(TAG, "onActivityResult:uri:" + uri);
            String path = getPath(getActivity(), uri);
            Log.d(TAG, "onActivityResult:path:" + path);
            cacheVideoPath(path);
            evPath.setText(path);
            setMergeTextView(path);
            setReverseTextView(path);
            setReEncodeText(path);
        }
    }

    private void setReEncodeText(String path){
        if(path != null){
            File file = new File(path);
            evReEncode.setTag(R.id.tv_reencode,path);
            evReEncode.setText(file.getName());
        }
    }

    private void setReverseTextView(String path){
        if(path != null){
            if(mTvReverse != null){
                File file = new File(path);
                mTvReverse.setTag(R.id.tv_reverse,path);
                mTvReverse.setText(file.getName());
            }
        } else {
            mTvReverse.setText("未选择");
        }
    }

    private void setMergeTextView(String path) {
        if (path != null) {
            File file = new File(path);
            if (mTvSrc1.getTag(R.id.tv_src1) == null) {
                mTvSrc1.setTag(R.id.tv_src1, path);
                mTvSrc1.setText(file.getName());
                mTvSrc2.setTag(R.id.tv_src2, null);
                mTvSrc2.setText("源视频2");
            } else if (mTvSrc2.getTag(R.id.tv_src2) == null) {
                mTvSrc2.setTag(R.id.tv_src2, path);
                mTvSrc2.setText(file.getName());
            } else {
                mTvSrc1.setTag(R.id.tv_src1, path);
                mTvSrc1.setText(file.getName());
                mTvSrc2.setTag(R.id.tv_src2, null);
                mTvSrc2.setText("源视频2");
            }
        }
    }

    private String getVideoPathFromCache() {
        return getActivity().getSharedPreferences("video", Context.MODE_PRIVATE).getString("video_path", null);
    }

    private void cacheVideoPath(String path) {
        SharedPreferences sp = getActivity().getSharedPreferences("video", Context.MODE_PRIVATE);
        sp.edit().putString("video_path", path).apply();
    }

    private String getPath(Context context, Uri uri) {
//        String[] filePathColumn = {MediaStore.Video.Media.DATA};
        String[] projection = new String[]{
                MediaStore.Video.Media.DISPLAY_NAME,
                MediaStore.Video.Media.SIZE,
                MediaStore.Images.Media.DATA,//绝对路径
                MediaStore.Video.Media.MIME_TYPE,
        };
        Cursor cursor = context.getContentResolver().query(uri, projection, null, null, null);
        String videoPath = null;
        if (cursor.moveToFirst())
            videoPath = cursor.getString(2);
        Log.i(TAG, videoPath == null ? "videoPath == null" : "videoPath：" + videoPath);
//        cursor.close();
        return videoPath;
    }

    @Override
    public void onClick(View v) {
        if (v == mBtnChooseFile) {
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
            intent.setType("video/*");//设置类型，我这里是任意类型，任意后缀的可以这样写。
            startActivityForResult(Intent.createChooser(intent, "视频选取"), 1);
        } else if (v == mBtnKeyFrame) {
            String videoPath = evPath.getText().toString();
            if (TextUtils.isEmpty(videoPath)) {
                Toast.makeText(v.getContext(), "请选择视频文件", Toast.LENGTH_SHORT).show();
                return;
            }
            uiHandler.sendEmptyMessage(MSG_SHOW_PB);
            Message threadMsg = threadHandler.obtainMessage();
            threadMsg.what = MSG_THREAD_EXTRACT_VIDEO;
            threadMsg.obj = videoPath;
            threadMsg.sendToTarget();
        } else if (v == mBtnDemuxing) {
            String videoPath = evPath.getText().toString();
            if (TextUtils.isEmpty(videoPath)) {
                Toast.makeText(v.getContext(), "请选择视频文件", Toast.LENGTH_SHORT).show();
                return;
            }
            uiHandler.sendEmptyMessage(MSG_SHOW_PB);
            Message threadMsg = threadHandler.obtainMessage();
            threadMsg.what = MSG_THREAD_DEMUXING;
            threadMsg.obj = videoPath;
            threadMsg.sendToTarget();
        } else if (v == mBtnMuxing) {
            String videoPath = evPath.getText().toString();
            if (TextUtils.isEmpty(videoPath)) {
                Toast.makeText(v.getContext(), "请选择视频文件", Toast.LENGTH_SHORT).show();
                return;
            }
            uiHandler.sendEmptyMessage(MSG_SHOW_PB);
            Message threadMsg = threadHandler.obtainMessage();
            threadMsg.what = MSG_THREAD_MUXING;
            threadMsg.obj = videoPath;
            threadMsg.sendToTarget();
        } else if (v == mBtnRemuxer) {
            String videoPath = evPath.getText().toString();
            if (TextUtils.isEmpty(videoPath)) {
                Toast.makeText(v.getContext(), "请选择视频文件", Toast.LENGTH_SHORT).show();
                return;
            }
            uiHandler.sendEmptyMessage(MSG_SHOW_PB);
            Message threadMsg = threadHandler.obtainMessage();
            threadMsg.what = MSG_THREAD_REMUXER;
            threadMsg.obj = videoPath;
            threadMsg.sendToTarget();
        } else if (v == mBtnCut) {
            if (TextUtils.isEmpty(evEnd.getText()) || TextUtils.isEmpty(evStart.getText())) {
                Toast.makeText(v.getContext(), "请输入开始和结束时间", Toast.LENGTH_SHORT).show();
                return;
            }
            String videoPath = evPath.getText().toString();
            if (TextUtils.isEmpty(videoPath)) {
                Toast.makeText(v.getContext(), "请选择视频文件", Toast.LENGTH_SHORT).show();
                return;
            }
            uiHandler.sendEmptyMessage(MSG_SHOW_PB);
            Message threadMsg = threadHandler.obtainMessage();
            threadMsg.what = MSG_THREAD_CUT;
            threadMsg.obj = videoPath;
            threadMsg.sendToTarget();
        } else if (v == mBtnRate) {

            String videoPath = evPath.getText().toString();
            if (TextUtils.isEmpty(videoPath)) {
                Toast.makeText(v.getContext(), "请选择视频文件", Toast.LENGTH_SHORT).show();
                return;
            }
            if (TextUtils.isEmpty(evRate.getText())) {
                Toast.makeText(v.getContext(), "请输入改变的倍数", Toast.LENGTH_SHORT).show();
                return;
            }
            uiHandler.sendEmptyMessage(MSG_SHOW_PB);
            Message threadMsg = threadHandler.obtainMessage();
            threadMsg.what = MSG_THREAD_RATE;
            threadMsg.obj = videoPath;
            threadMsg.sendToTarget();
        } else if (v == mBtnMoment) {

            String videoPath = evPath.getText().toString();
            if (TextUtils.isEmpty(videoPath)) {
                Toast.makeText(v.getContext(), "请选择视频文件", Toast.LENGTH_SHORT).show();
                return;
            }
            if (TextUtils.isEmpty(evMoment.getText())) {
                Toast.makeText(v.getContext(), "请输入改变的时刻", Toast.LENGTH_SHORT).show();
                return;
            }
            uiHandler.sendEmptyMessage(MSG_SHOW_PB);
            Message threadMsg = threadHandler.obtainMessage();
            threadMsg.what = MSG_THREAD_MOMENT;
            threadMsg.obj = videoPath;
            threadMsg.sendToTarget();
        } else if (v == mBtnAddWatermark) {
            String videoPath = evPath.getText().toString();
            if (TextUtils.isEmpty(videoPath)) {
                Toast.makeText(v.getContext(), "请选择视频文件", Toast.LENGTH_SHORT).show();
                return;
            }
            uiHandler.sendEmptyMessage(MSG_SHOW_PB);
            Message threadMsg = threadHandler.obtainMessage();
            threadMsg.what = MSG_THREAD_ADD_WATERMARK;
            threadMsg.obj = videoPath;
            threadMsg.sendToTarget();
        } else if (v == mBtnMerge) {
            Object tag1 = mTvSrc1.getTag(R.id.tv_src1);
            Object tag2 = mTvSrc2.getTag(R.id.tv_src2);
            if (tag1 != null && tag2 != null) {
                String srcPath1 = (String) tag1;
                String srcPath2 = (String) tag2;
                if (isFileExist(srcPath1) && isFileExist(srcPath2)) {
                    String outPath = getMergeVideoPath(srcPath1, srcPath2);
                    Message threadMsg = threadHandler.obtainMessage(MSG_THREAD_MERGE_VIDEO);
                    Bundle data = new Bundle();
                    data.putString("SRC_PATH1", srcPath1);
                    data.putString("SRC_PATH2", srcPath2);
                    data.putString("OUT_PATH", outPath);
                    threadMsg.setData(data);
                    threadMsg.sendToTarget();
                } else {
                    if (!isFileExist(srcPath1)) {
                        mTvSrc1.setText("源视频1");
                        mTvSrc1.setTag(R.id.tv_src1, null);
                    } else if (!isFileExist(srcPath2)) {
                        mTvSrc2.setText("源视频2");
                        mTvSrc2.setTag(R.id.tv_src2, null);
                    }
                }
            } else {
                Toast.makeText(getActivity(), "请先要选择视频文件", Toast.LENGTH_SHORT).show();
            }
        } else if( v == mBtnReverse ){
            Object tag = mTvReverse.getTag(R.id.tv_reverse);
            if(tag != null){
                String path = (String) tag;
                if(isFileExist(path)){
                    Message threadMsg = threadHandler.obtainMessage(MSG_THREAD_REVERSE);
                    threadMsg.obj = path;
                    threadMsg.sendToTarget();
                }
            }else{
                Toast.makeText(getActivity(),"未指定逆序源文件",Toast.LENGTH_SHORT).show();
            }
        } else if(v == btnReEncode){
            Object tag = evReEncode.getTag(R.id.tv_reencode);
            if(tag != null){
                String path = (String) tag;
                if(isFileExist(path)){
                    Message threadMsg = threadHandler.obtainMessage(MSG_THREAD_REENCODE);
                    Bundle data = new Bundle();
                    data.putString("SRC_PATH",path);
                    data.putString("OUT_PATH", PathGeneratorUtil.generateReencodeFile(path));
                    threadMsg.setData(data);
                    threadMsg.sendToTarget();
                }
            }
        }
    }

    private boolean isFileExist(String path) {
        if (path == null) {
            return false;
        }
        File file = new File(path);
        if (!file.exists()) {
            return false;
        }
        return true;
    }

    private String getMergeVideoPath(String src1, String src2) {
        if (src1 != null && src2 != null) {
            File srcfile1 = new File(src1);
            File srcFile2 = new File(src2);
            File outFile = new File(Environment.getExternalStorageDirectory() + File.separator + "AClip/" + "videoMerge", srcfile1.getName().split("\\.")[0] + "_" + srcFile2.getName().split("\\.")[0] + "." + srcfile1.getName().split("\\.")[1]);
            if (!outFile.getParentFile().exists()) {
                outFile.getParentFile().mkdir();
            }
            return outFile.getAbsolutePath();
        }
        return null;
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
    private static final int MSG_THREAD_RATE = 0x5;
    private static final int MSG_THREAD_ADD_WATERMARK = 0x6;
    private static final int MSG_THREAD_MERGE_VIDEO = 0x07;
    private static final int MSG_THREAD_REMUXER = 0x08;
    private static final int MSG_THREAD_MOMENT = 0x09;
    private static final int MSG_THREAD_REVERSE = 0x10;
    private static final int MSG_THREAD_REENCODE = 0X11;
    private static final int FLAG_KEY_FRAME = 0x1;
    private static final int FLAG_DEMUXING = 0x2;

    private void handleUiMessage(Message msg) {
        switch (msg.what) {
            case MSG_SHOW_PB:
                mBtnDemuxing.setEnabled(false);
                mBtnRemuxer.setEnabled(false);
                mBtnKeyFrame.setEnabled(false);
                mBtnMuxing.setEnabled(false);
                mBtnCut.setEnabled(false);
                mBtnRate.setEnabled(false);
                mBtnMoment.setEnabled(false);
                mBtnAddWatermark.setEnabled(false);
                progressBar.setVisibility(View.VISIBLE);
                break;
            case MSG_HIDE_PB:
                mBtnDemuxing.setEnabled(true);
                mBtnKeyFrame.setEnabled(true);
                mBtnRemuxer.setEnabled(true);
                mBtnMoment.setEnabled(true);
                mBtnMuxing.setEnabled(true);
                mBtnCut.setEnabled(true);
                mBtnRate.setEnabled(true);
                mBtnAddWatermark.setEnabled(true);
                progressBar.setVisibility(View.INVISIBLE);
                break;
            case MSG_SHOW_RESULT:
                String result = (String) msg.obj;
                tvResult.setText(result);
                uiHandler.sendEmptyMessage(MSG_HIDE_PB);
                break;
        }
    }

    private void handlerThreadMessage(Message msg) {
        String result = null;
        String path = null;
        String videoPath = null;
        if (msg.obj != null) {
            videoPath = (String) msg.obj;
            File picPath = new File(Environment.getExternalStorageDirectory() + File.separator + "AClip/", getVideoName(videoPath));
            if (!picPath.exists()) {
                picPath.mkdirs();
            }
            path = picPath.getAbsolutePath();
            Log.d("ndk-log", "   path=" + path);
        }
        switch (msg.what) {
            case MSG_THREAD_REENCODE:
            {
                Bundle data = msg.getData();
                if(data != null){
                    String srcPath = data.getString("SRC_PATH");
                    String outPath = data.getString("OUT_PATH");
                    if(srcPath != null && outPath != null){
                        int r = VideoClipJni.reEncode(srcPath,outPath);
                        if(r >= 0){
                            result = "生成视频逆序成功";
                        }else{
                            result = "生成视频逆序失败";
                        }
                    }else{
                        result = "先输入文件";
                    }
                    showResult(result);
                }
            }
            break;
            case MSG_THREAD_REVERSE:{
                if(videoPath != null){
                    File file = new File(videoPath);
                    String fileName;
                    if(file.exists()){
                        fileName = file.getName();
                        File outFile = new File(Environment.getExternalStorageDirectory() + File.separator + "AClip/reverse2/","reverse_"+fileName);
                        if(!outFile.getParentFile().exists()){
                            outFile.getParentFile().mkdir();
                        }
                        int r = VideoClipJni.reverse(videoPath,outFile.getAbsolutePath());
                        if(r >= 0){
                            result = "生成视频逆序成功";
                        }else{
                            result = "生成视频逆序失败";
                        }
                    }else{
                        result = "逆序源视频不存在";
                    }
                    showResult(result);
                }
            }
            break;
            case MSG_THREAD_MERGE_VIDEO: {
                Bundle data = msg.getData();
                String srcPath1 = (String) data.get("SRC_PATH1");
                String srcPath2 = (String) data.get("SRC_PATH2");
                String outPath = (String) data.get("OUT_PATH");
                int r = VideoClipJni.videoMerge(srcPath1, srcPath2, outPath);
                if (r == 0) {
                    result = "视频合成成功";
                } else {
                    result = "视频合成失败";
                }
                showResult(result);
            }
            break;
            case MSG_THREAD_EXTRACT_VIDEO:
                result =""+ VideoClipJni.demuxingKeyFrame(videoPath, path);
                showResult(result);
                break;
            case MSG_THREAD_DEMUXING:
                result = ""+VideoClipJni.demuxing(videoPath, path + "/out.h264", path + "/out.aac");
                showResult(result);
                break;
            case MSG_THREAD_REMUXER:
                result = ""+VideoClipJni.remuxing(videoPath, path + "/out.flv");
                showResult(result);
                break;
            case MSG_THREAD_MUXING:
                result =""+ VideoClipJni.muxing(path + "/out.h264", path + "/out.aac", path + "/out.mp4");
                showResult(result);
                break;
            case MSG_THREAD_CUT:
                double start = Double.valueOf(evStart.getText().toString());
                double end = Double.valueOf(evEnd.getText().toString());
                if (cutAudio) {
                    result = ""+VideoClipJni.cutAudio(start, end, videoPath, path + "/cut_" + start + "_" + end + ".aac");
                } else {
                    result = ""+VideoClipJni.cutVideo(start, end, videoPath, path + "/cut_" + start + "_" + end + ".mp4");
                }
                showResult(result);
                break;
            case MSG_THREAD_RATE:
                int rate = Integer.valueOf(evRate.getText().toString());
                if (isSlowVideoSpeed) {
                    result = ""+VideoClipJni.slowVideoSpeed(videoPath, path + "/slow_rate_" + rate + ".mp4", rate, null);
                } else {
                    result =""+ VideoClipJni.fastVideoSpeed(videoPath, path + "/fast_rate_" + rate + ".mp4", rate, null);
                }
                showResult(result);
                break;
            case MSG_THREAD_MOMENT:
                double second = Double.valueOf(evMoment.getText().toString());
                if (isSlowVideoMoment) {
                    result = ""+VideoClipJni.relativeVideoMoment(videoPath, path + "/relative_moment_" + second + ".mp4", second);
                } else {
                    result =""+ VideoClipJni.repeatVideoMoment(videoPath, path + "/repeat_moment_" + second + ".mp4", second);
                }
                showResult(result);
                break;
            case MSG_THREAD_ADD_WATERMARK:
                String watermarkPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/AClip/Watermark/flag.png";
                String temp = "movie=" + watermarkPath + "[wm];[in][wm]overlay=5:5[out]";
                result = ""+VideoClipJni.addWatermark(temp, videoPath, path + "/out.mp4");
                showResult(result);
                break;
        }
    }

    private void showResult(String msg) {
        Message message = uiHandler.obtainMessage();
        message.obj = msg;
        message.what = MSG_SHOW_RESULT;
        message.sendToTarget();
    }
}
