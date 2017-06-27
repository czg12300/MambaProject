
package com.framework;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.provider.MediaStore;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.text.Editable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;


import com.framework.test.AudioThread;
import com.mamba.framework.R;

import java.lang.ref.WeakReference;

/**
 * 描述：音频编辑
 *
 * @author walljiang
 * @since 2017/04/07 14:24
 */

public class AudioEditFragment extends Fragment implements View.OnClickListener, Handler.Callback {

    private final int FILE_SELECT_CODE_1 = 1;

    private final int FILE_SELECT_CODE_2 = 2;

    private final String TAG = "AudioEditFragment";

    Button mBtnChoose1, mBtnChoose2, mBtnMerge, mBtnMix, mBtnRate_2, mBtnRate_1, mBtnRate1,
            mBtnRate2, mBtnRecord, mBtnFFDecode, mBtnFdkDecode, mBtnFdkEncode, mBtnVideoMix,
            mBtnVideoTranscode, mBtnVideoFilter;

    EditText mEtpath1, mEtpath2, mEtVol1, mEtVol2, mEtRatioVideo1, mEtRatioVideo2, mEtVideoFilter;

    TextView mTVProgress;

    ThreadHandler mThreadHandler;

    Handler mUIHandler;

    AudioThread mAudioRecordThread;

    AlertDialog mConfirmDialog;

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container,
            @Nullable Bundle savedInstanceState) {
        View layout = inflater.inflate(R.layout.fragment_audio_edit, null);
        HandlerThread handlerThread = new HandlerThread(TAG);
        handlerThread.start();
        mThreadHandler = new ThreadHandler(handlerThread.getLooper(), this);
        mUIHandler = new Handler(Looper.getMainLooper(), this);
        return layout;
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        mBtnChoose1 = (Button) view.findViewById(R.id.btn_choose1);
        mBtnChoose1.setOnClickListener(this);
        mBtnChoose2 = (Button) view.findViewById(R.id.btn_choose2);
        mBtnChoose2.setOnClickListener(this);
        mBtnMerge = (Button) view.findViewById(R.id.btn_audio_volumn);
        mBtnMerge.setOnClickListener(this);
        mBtnMix = (Button) view.findViewById(R.id.btn_audio_mix);
        mBtnMix.setOnClickListener(this);
        mBtnRate_2 = (Button) view.findViewById(R.id.btn_rate_slowmost);
        mBtnRate_2.setOnClickListener(this);
        mBtnRate_1 = (Button) view.findViewById(R.id.btn_rate_slow);
        mBtnRate_1.setOnClickListener(this);
        mBtnRate1 = (Button) view.findViewById(R.id.btn_rate_speed);
        mBtnRate1.setOnClickListener(this);
        mBtnRate2 = (Button) view.findViewById(R.id.btn_rate_speedmost);
        mBtnRate2.setOnClickListener(this);
        mBtnRecord = (Button) view.findViewById(R.id.btn_audio_record);
        mBtnRecord.setOnClickListener(this);
        mBtnFFDecode = (Button) view.findViewById(R.id.btn_audio_ffdecode);
        mBtnFFDecode.setOnClickListener(this);
        mBtnFdkDecode = (Button) view.findViewById(R.id.btn_audio_fdkdecode);
        mBtnFdkDecode.setOnClickListener(this);
        mBtnFdkEncode = (Button) view.findViewById(R.id.btn_audio_fdkencode);
        mBtnFdkEncode.setOnClickListener(this);
        mBtnVideoMix = (Button) view.findViewById(R.id.btn_video_mix);
        mBtnVideoMix.setOnClickListener(this);
        mBtnVideoTranscode = (Button) view.findViewById(R.id.btn_video_transcode);
        mBtnVideoTranscode.setOnClickListener(this);
        mBtnVideoFilter = (Button) view.findViewById(R.id.btn_video_filter);
        mBtnVideoFilter.setOnClickListener(this);

        mEtpath1 = (EditText) view.findViewById(R.id.et_path1);
        mEtpath2 = (EditText) view.findViewById(R.id.et_path2);
        mEtVol1 = (EditText) view.findViewById(R.id.et_audio_vol1);
        mEtVol2 = (EditText) view.findViewById(R.id.et_audio_vol2);
        mEtRatioVideo1 = (EditText) view.findViewById(R.id.et_video1);
        mEtRatioVideo2 = (EditText) view.findViewById(R.id.et_video2);
        mEtVideoFilter = (EditText) view.findViewById(R.id.et_video_filter);

        mTVProgress = (TextView) view.findViewById(R.id.tv_progress);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mThreadHandler.removeCallbacksAndMessages(null);
        mThreadHandler.getLooper().quit();
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode == Activity.RESULT_OK) {// 是否选择，没选择就不会继续
            if (requestCode == FILE_SELECT_CODE_1) {
                Uri uri = data.getData();// 得到uri，后面就是将uri转化成file的过程。
                String path = getPath(getActivity(), uri);
                if (path != null) {
                    mEtpath1.setText(path);
                }
            } else if (requestCode == FILE_SELECT_CODE_2) {
                Uri uri = data.getData();// 得到uri，后面就是将uri转化成file的过程。
                String path = getPath(getActivity(), uri);
                if (path != null) {
                    mEtpath2.setText(path);
                }
            }
        }
    }

    private final class ThreadHandler extends Handler {

        private final WeakReference<AudioEditFragment> reference;

        public ThreadHandler(Looper looper, AudioEditFragment audioEditFragment) {
            super(looper);
            reference = new WeakReference<AudioEditFragment>(audioEditFragment);
        }

        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            AudioEditFragment audioEditFragment = reference.get();
            if (audioEditFragment != null) {
                audioEditFragment.handleThreadMessage(msg);
            }
        }
    }

    private final int MSG_BACK_AUDIO_VOLUMN = 0x00;

    private final int MSG_BACK_AUDIO_RATE = 0X01;

    private final int MSG_BACK_AUDIO_MIX = 0X02;

    private final int MSG_BACK_AUDIO_DECODE = 0x03;

    private final int MSG_BACK_VIDEO_MIX = 0x04;

    private final int MSG_BACK_VIDEO_TRANSCODE = 0x05;

    private final int MSG_BACK_VIDEO_FILTER = 0x06;

    public void handleThreadMessage(Message msg) {
        switch (msg.what) {
            case MSG_BACK_VIDEO_FILTER: {
                Bundle data = msg.getData();
                if (data != null) {
                    String path1 = data.getString("PATH");
                    String outpath = data.getString("OUTPATH");
                    String filterargs = data.getString("FILTER_ARGS");
                    if (path1 != null && outpath != null) {
                        int ret = VideoClipJni.videoFilter(path1, outpath, filterargs);
                        showMessage((ret >= 0 ? "视频滤镜成功" : "视频滤镜失败"));
                    }
                }
            }
                break;
            case MSG_BACK_VIDEO_TRANSCODE: {
                Bundle data = msg.getData();
                if (data != null) {
                    String path1 = data.getString("PATH");
                    String outpath = data.getString("OUTPATH");
                    if (path1 != null && outpath != null) {
                        int ret = VideoClipJni.videoTranscode(path1, outpath);
                        showMessage((ret >= 0 ? "视频转码成功" : "视频转码失败"));
                    }
                }
            }
                break;
            case MSG_BACK_VIDEO_MIX: {
                Bundle data = msg.getData();
                if (data != null) {
                    String path1 = data.getString("PATH1");
                    String path2 = data.getString("PATH2");
                    String outPath = data.getString("OUTPATH");
                    int mix1 = data.getInt("MIX1", 1);
                    int mix2 = data.getInt("MIX2", 1);
                    if (mix1 == 0 || mix2 == 0) {
                        showMessage((mix1 == 0 ? "MIX1" : "") + (mix2 == 0 ? " MIX2" : "")
                                + "为0不需要混合，可直接使用原视频播放");
                        return;
                    }
                    if (path1 != null && path2 != null && outPath != null) {
                        int ret = VideoClipJni.videoMix(path1, path2, outPath, mix1, mix2);
                        showMessage((ret >= 0 ? "视频混合成功" : "视频混合失败"));
                    }
                }
            }
                break;
            case MSG_BACK_AUDIO_DECODE: {
                Bundle data = msg.getData();
                if (data != null) {
                    String path = data.getString("PATH");
                    String outPath = data.getString("OUTPATH");
                    int type = data.getInt("TYPE");
                    if (path != null && outPath != null) {
                        int ret = VideoClipJni.audioDecode(path, outPath, type);
                        showMessage((ret >= 0 ? "解码成功" : "解码失败"));
                    } else {
                        showMessage("解码参数不足");
                    }
                }
            }
                break;
            case MSG_BACK_AUDIO_MIX: {
                Bundle data = msg.getData();
                if (data != null) {
                    String path1 = data.getString("PATH1");
                    String path2 = data.getString("PATH2");
                    String outPath = data.getString("OUTPATH");
                    float vol1 = data.getFloat("VOL1");
                    float vol2 = data.getFloat("VOL2");
                    if (vol1 == 0 || vol2 == 0) {
                        showMessage((vol1 == 0 ? "VOL1" : "") + (vol2 == 0 ? " VOL2" : "")
                                + "为0不需要混音，可直接使用原音播放");
                        return;
                    }
                    if (path1 != null && path2 != null && outPath != null) {
                        int ret = VideoClipJni.audioMix(path1, path2, outPath, vol1, vol2);
                        showMessage((ret >= 0 ? "混音失败" : "混音成功"));
                    }
                }
            }
                break;
            case MSG_BACK_AUDIO_RATE: {
                Bundle data = msg.getData();
                if (data != null) {
                    String path = data.getString("PATH");
                    float rate = data.getFloat("RATE", 0f);
                    String outPath = data.getString("OUTPATH");
                    if (path != null && outPath != null) {
                        int ret = VideoClipJni.audioRate(path, outPath, rate,
                                new HandleProgressListener() {
                                    @Override
                                    public void onHandle(int progress, int total) {
                                        showProgress(progress, total);
                                    }

                                    @Override
                                    public void onSuccess(String outFile) {
                                    }
                                });
                        String message = (ret >= 0 ? "音频变速成功" : "音频变速失败");
                        showMessage(message);
                    }
                }
            }
                break;
            case MSG_BACK_AUDIO_VOLUMN: {
                Bundle data = msg.getData();
                if (data != null) {
                    String path1 = data.getString("SRC_1");
                    String outFile = data.getString("OUT_FILE");
                    float vol1 = data.getFloat("VOL1");
                    if (path1 != null && outFile != null) {
                        int ret = VideoClipJni.audioVolumn(path1, outFile, vol1);
                        String message = (ret >= 0 ? "音频大小调整成功" : "音频大小调整失败");
                        showMessage(message);
                    }
                }
            }
                break;
        }
    }

    private final int MSG_UI_SHOW_MESSAGE = 0x00;

    private final int MSG_UI_SHOW_PROGRESS = 0x01;

    @Override
    public boolean handleMessage(Message msg) {
        switch (msg.what) {
            case MSG_UI_SHOW_PROGRESS: {
                Bundle data = msg.getData();
                if (data != null) {
                    int progress = data.getInt("PROGRESS");
                    int total = data.getInt("TOTAL");
                    mTVProgress.setText(progress + "/" + total);
                }
            }
                break;
            case MSG_UI_SHOW_MESSAGE: {
                Object obj = msg.obj;
                if (obj != null) {
                    String message = (String) obj;
                    Toast.makeText(getActivity(), message, Toast.LENGTH_SHORT).show();
                }
            }
                break;
        }
        return false;
    }

    private void showProgress(int progress, int total) {
        Message uiMsg = mUIHandler.obtainMessage(MSG_UI_SHOW_PROGRESS);
        Bundle data = new Bundle();
        data.putInt("PROGRESS", progress);
        data.putInt("TOTAL", total);
        uiMsg.setData(data);
        uiMsg.sendToTarget();
    }

    private void showMessage(String message) {
        Message uiMsg = mUIHandler.obtainMessage(MSG_UI_SHOW_MESSAGE);
        uiMsg.obj = message;
        uiMsg.sendToTarget();
    }

    private String getPath(Context context, Uri uri) {
        // 获取视频信息
        String[] projection = new String[] {
                MediaStore.Video.Media.DISPLAY_NAME, MediaStore.Video.Media.SIZE,
                MediaStore.Images.Media.DATA, // 绝对路径
                MediaStore.Video.Media.MIME_TYPE,
        };
        Cursor cursor = context.getContentResolver().query(uri, projection, null, null, null);
        String videoPath = null;
        if (cursor.moveToFirst())
            videoPath = cursor.getString(2);
        cursor.close();
        return videoPath;
    }

    private String getPath(int path) {
        String path1 = null;
        Editable text = (path == 1 ? mEtpath1.getText() : mEtpath2.getText());
        if (text != null && text.toString().length() > 0) {
            path1 = text.toString();
        } else {
            showMessage((path == 1 ? "第一个音频未指定" : "第二个音频未指定"));
        }
        return path1;
    }

    @Override
    public void onClick(View v) {
        int id = v.getId();
        if (id == R.id.btn_choose1) {
            showFileChooser(FILE_SELECT_CODE_1);
        } else if (id == R.id.btn_choose2) {
            showFileChooser(FILE_SELECT_CODE_2);
        } else if (id == R.id.btn_audio_volumn) {
            String path1 = getPath(1);
            float vol1 = 10.0f;
            Editable text1 = mEtVol1.getText();
            Editable text2 = mEtVol2.getText();
            if (text1 != null && text1.toString().length() > 0) {
                vol1 = Float.parseFloat(text1.toString());
            }
            if (path1 != null) {
                Message mergeMsg = mThreadHandler.obtainMessage(MSG_BACK_AUDIO_VOLUMN);
                Bundle data = new Bundle();
                data.putString("SRC_1", path1);
                data.putFloat("VOL1", vol1);
                data.putString("OUT_FILE", PathGeneratorUtil.generateAudioVolumnFile(path1));
                mergeMsg.setData(data);
                mergeMsg.sendToTarget();
            }
        } else if (id == R.id.btn_audio_mix) {
            String path1 = getPath(1);
            String path2 = getPath(2);
            float vol1 = 10.0f, vol2 = 10.0f;
            Editable text1 = mEtVol1.getText();
            Editable text2 = mEtVol2.getText();
            if (text1 != null && text1.toString().length() > 0) {
                vol1 = Float.parseFloat(text1.toString());
            }
            if (text2 != null && text2.toString().length() > 0) {
                vol2 = Integer.parseInt(text2.toString());
            }
            if (path1 != null && path2 != null) {
                Message mixMsg = mThreadHandler.obtainMessage(MSG_BACK_AUDIO_MIX);
                Bundle data = new Bundle();
                data.putString("PATH1", path1);
                data.putString("PATH2", path2);
                data.putString("OUTPATH",
                        PathGeneratorUtil.generateAudioFile(path1, path2, "audioMix"));
                data.putFloat("VOL1", vol1);
                data.putFloat("VOL2", vol2);
                mixMsg.setData(data);
                mixMsg.sendToTarget();
            }
        } else if (id == R.id.btn_rate_slowmost || id == R.id.btn_rate_slow
                || id == R.id.btn_rate_speed || id == R.id.btn_rate_speedmost) {
            Object tag = v.getTag();
            if (tag != null) {
                String path = getPath(1);
                float rate = 0f;
                int rateLevel = Integer.parseInt((String) tag);
                rate = (rateLevel <= -2 ? 0.25f
                        : rateLevel <= -1 ? 0.5f : rateLevel <= 1 ? 1f : 2f);
                if (path != null) {
                    Message rateMsg = mThreadHandler.obtainMessage(MSG_BACK_AUDIO_RATE);
                    Bundle data = new Bundle();
                    data.putString("PATH", path);
                    data.putFloat("RATE", rate);
                    data.putString("OUTPATH", PathGeneratorUtil.generateAudioRateFile(path));
                    rateMsg.setData(data);
                    rateMsg.sendToTarget();
                }
            }
        } else if (id == R.id.btn_audio_record) {
            if (mAudioRecordThread == null || !mAudioRecordThread.getRecordFlag()) {
                mAudioRecordThread = new AudioThread(PathGeneratorUtil.generateAACRecordFile());
                mAudioRecordThread.start();
                mBtnRecord.setText("停止");
            } else {
                if (mConfirmDialog == null) {
                    mConfirmDialog = new AlertDialog.Builder(getActivity()).setTitle("请确认")
                            .setMessage("音频录制中，确认取消？")
                            .setPositiveButton("确认", new DialogInterface.OnClickListener() {

                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    if (mAudioRecordThread != null
                                            && mAudioRecordThread.getRecordFlag()) {
                                        mAudioRecordThread.setRecordFlag(false);
                                        mAudioRecordThread = null;
                                        mBtnRecord.setText("录制");
                                    }
                                }
                            }).create();
                }
                mConfirmDialog.show();
            }
        } else if (id == R.id.btn_audio_fdkdecode || id == R.id.btn_audio_ffdecode) {
            Object tag = v.getTag();
            if (tag != null) {
                String path = getPath(1);
                int decodeType = Integer.parseInt((String) tag);
                if (path != null) {
                    Message decodeMsg = mThreadHandler.obtainMessage(MSG_BACK_AUDIO_DECODE);
                    Bundle data = new Bundle();
                    data.putString("PATH", path);
                    data.putString("OUTPATH", PathGeneratorUtil.generateAACDecodeFile(path));
                    data.putInt("TYPE", decodeType);
                    decodeMsg.setData(data);
                    decodeMsg.sendToTarget();
                }
            }
        } else if (id == R.id.btn_audio_fdkencode) {

        } else if (id == R.id.btn_video_mix) {
            String path1 = getPath(1);
            String path2 = getPath(2);
            int mix1 = 1, mix2 = 1;
            Editable text1 = mEtRatioVideo1.getText();
            Editable text2 = mEtRatioVideo2.getText();
            if (text1 != null && text1.toString().length() > 0) {
                mix1 = Integer.parseInt(text1.toString());
            }
            if (text2 != null && text2.toString().length() > 0) {
                mix2 = Integer.parseInt(text2.toString());
            }
            if (path1 != null && path2 != null) {
                Message mixMsg = mThreadHandler.obtainMessage(MSG_BACK_VIDEO_MIX);
                Bundle data = new Bundle();
                data.putString("PATH1", path1);
                data.putString("PATH2", path2);
                data.putString("OUTPATH",
                        PathGeneratorUtil.generateAudioFile(path1, path2, "videoMix"));
                data.putInt("MIX1", mix1);
                data.putInt("MIX2", mix2);
                mixMsg.setData(data);
                mixMsg.sendToTarget();
            }
        } else if (id == R.id.btn_video_transcode) {
            String path1 = getPath(1);
            if (path1 != null) {
                Message transcodeMsg = mThreadHandler.obtainMessage(MSG_BACK_VIDEO_TRANSCODE);
                Bundle data = new Bundle();
                data.putString("PATH", path1);
                data.putString("OUTPATH", PathGeneratorUtil.generateVideoTransCodeFile(path1));
                transcodeMsg.setData(data);
                transcodeMsg.sendToTarget();
            }
        } else if (id == R.id.btn_video_filter) {
            String path1 = getPath(1);
            String filterArgs = "edgedetect=low=0.1:high=0.4";// 加网格
            Editable args = mEtVideoFilter.getText();
            if (args != null && args.toString().trim().length() > 0) {
                filterArgs = args.toString().trim();
            }
            if (path1 != null) {
                Message filterMsg = mThreadHandler.obtainMessage(MSG_BACK_VIDEO_FILTER);
                Bundle data = new Bundle();
                data.putString("PATH", path1);
                data.putString("OUTPATH", PathGeneratorUtil.generateVideoFilterFile(path1));
                data.putString("FILTER_ARGS", filterArgs);

                filterMsg.setData(data);
                filterMsg.sendToTarget();
            }
        }
    }

    private void showFileChooser(int code) {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("*/*");
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        try {
            startActivityForResult(Intent.createChooser(intent, "选择一个mp3文件"), code);
        } catch (android.content.ActivityNotFoundException ex) {
            Toast.makeText(getActivity(), "没有文件浏览器.", Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * 此方法从c层回调
     *
     * @param progress
     * @param msg
     */
    public void callBackFromJNI(int progress, String msg) {

    }
}
