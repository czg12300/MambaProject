package com.mamba.model.record.encode.video;

import android.os.Build;

import com.mamba.model.VLog;
import com.mamba.model.record.RecordHolder;
import com.mamba.model.record.encode.RecorderCallback;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * 编码器持有类
 *
 * @author jake
 * @since 2017/6/21 下午5:18
 */

public class VideoCodecHolder {
    private int positionFrameRate;
    private long lastFrameTimestamp = 0;
    private boolean isEncoding = false;
    private VideoCodecParameters mVideoCodecParameters;
    private VideoEncoder mEncoder;
    private RecorderCallback recorderCallback;
    public static boolean useMediaCodec = true;

    public VideoCodecHolder() {
        boolean isUseFfmpeg = Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN || !isSupportMediaCodec() || useMediaCodec;
        if (isUseFfmpeg) {
            mEncoder = new FfmpegVideoEncoder();
        } else {
            mEncoder = new MediaCodecEncoder();
        }
        mEncoder.setCallback(encodeCallback);
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


    public void setRecorderCallback(RecorderCallback recorderCallback) {
        this.recorderCallback = recorderCallback;
    }


    public void start(VideoCodecParameters parameters) {
        mVideoCodecParameters = parameters;
        if (mVideoCodecParameters == null) {
            throw new NullPointerException("VideoCodecParameters is null,you have to setup VideoCodecParameters");
        }
        positionFrameRate = mVideoCodecParameters.speedFrameRate > 0 ? mVideoCodecParameters.speedFrameRate : mVideoCodecParameters.frameRate;
        mEncoder.startEncode(parameters);
        isEncoding = true;
    }


    public void stop() {
        if (!isEncoding) {
            return;
        }
        isEncoding = false;
        mEncoder.stopEncode();
    }


    private int calculateEncodeTimes(long timestamp) {
        int result = 0;
        if (lastFrameTimestamp > 0) {
            long preTimestampSpit = 1000 / positionFrameRate;
            long preTimestamp = lastFrameTimestamp + preTimestampSpit;
            if (timestamp >= preTimestamp) {
                int timestampTimes = (int) (((timestamp - lastFrameTimestamp) * 1.0f) / preTimestampSpit);
                result = timestampTimes;
                lastFrameTimestamp = lastFrameTimestamp + timestampTimes * preTimestampSpit;
            }
        } else {
            result = 1;
            lastFrameTimestamp = timestamp;
        }
        VLog.d("calculateEncodeTimes result  " + result);
        return result;
    }

    private MediaCodecEncoder.Callback encodeCallback = new MediaCodecEncoder.Callback() {
        @Override
        public void onStart() {
            lastFrameTimestamp = 0;
            VLog.d("onStart");
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

    public void onFrameAvailable(VideoFrame frame) {
        if (isEncoding && mEncoder != null && frame != null) {
            int writeTime = calculateEncodeTimes(frame.timestamp);
            if (writeTime > 0) {
                frame.setWriteTimes(writeTime);
                mEncoder.offerRawFrame(frame);
            }
        }
    }
}
