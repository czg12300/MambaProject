package com.mamba.model.record.encode.audio;

import android.os.Build;

import com.mamba.model.record.encode.RecorderCallback;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;

/**
 * 音频录制
 *
 * @author jake
 * @since 2017/7/6 下午5:09
 */

public class AudioCodecHolder {
    private RecorderCallback recorderCallback;
    private AudioRecordTask mAudioRecordTask;
    private AudioEncoder mEncoder;

    public void start(AudioCodecParameters audioCodecParameters) {
        mAudioRecordTask = new AudioRecordTask();
        mAudioRecordTask.setAudioFrameCallback(audioRecordCallback);
        boolean isUseFfmpeg = Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN || !isSupportMediaCodec() || false;
        isUseFfmpeg = false;
        if (isUseFfmpeg) {
            mEncoder = new FfmpegAudioEncoder();
        } else {
            mEncoder = new MediaCodecAudioEncoder();
        }
        mEncoder.setCallback(callback);
        mEncoder.startEncode(audioCodecParameters);
        mAudioRecordTask.startTask();
    }

    public void setRecorderCallback(RecorderCallback recorderCallback) {
        this.recorderCallback = recorderCallback;
    }

    AudioEncoder.Callback callback = new AudioEncoder.Callback() {
        @Override
        public void onStart() {
            if (recorderCallback != null) {
                recorderCallback.onStart();
            }
        }

        @Override
        public void onStop() {
            if (recorderCallback != null) {
                recorderCallback.onStop();
            }
        }
    };

    private AudioRecordCallback audioRecordCallback = new AudioRecordCallback() {
        @Override
        public void onStart() {

        }

        @Override
        public void onFrameAvailable(byte[] data) {

            if (mEncoder != null && data != null) {
                mEncoder.offerRawFrame(AudioFrame.create(data, data.length));
            }
        }

        @Override
        public void onStop() {
            if (mEncoder != null) {
                mEncoder.stopEncode();
            }
        }
    };

    public void stop() {
        if (mAudioRecordTask != null) {
            mAudioRecordTask.stopTask();
        }
    }


    public boolean isSupportMediaCodec() {
        boolean isHardcode = false;
        //读取系统配置文件/system/etc/media_codecc.xml
        File file = new File("/system/etc/media_codecs.xml");
        InputStream inFile = null;
        try {
            inFile = new FileInputStream(file);
        } catch (Exception e) {
            // TODO: handle exception
        }

        if (inFile != null) {
            XmlPullParserFactory pullFactory;
            try {
                pullFactory = XmlPullParserFactory.newInstance();
                XmlPullParser xmlPullParser = pullFactory.newPullParser();
                xmlPullParser.setInput(inFile, "UTF-8");
                int eventType = xmlPullParser.getEventType();
                while (eventType != XmlPullParser.END_DOCUMENT) {
                    String tagName = xmlPullParser.getName();
                    switch (eventType) {
                        case XmlPullParser.START_TAG:
                            if ("MediaCodec".equals(tagName)) {
                                String componentName = xmlPullParser.getAttributeValue(0);

                                if (componentName.startsWith("OMX.")) {
                                    if (!componentName.startsWith("OMX.google.")) {
                                        isHardcode = true;
                                    }
                                }
                            }
                    }
                    eventType = xmlPullParser.next();
                }
            } catch (Exception e) {
                // TODO: handle exception
            }
        }
        return isHardcode;
    }
}
