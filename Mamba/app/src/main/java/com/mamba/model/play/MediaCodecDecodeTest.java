package com.mamba.model.play;

import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.media.Image;
import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.os.Build;
import android.support.annotation.NonNull;
import android.support.annotation.RequiresApi;
import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;

import com.framework.ndk.media.FfmpegMediaExtractor;
import com.framework.ndk.media.FfmpegMediaFormat;
import com.framework.ndk.videoutils.FfmpegFormatUtils;
import com.framework.ndk.videoutils.PixelTransUtils;
import com.framework.utils.FileUtil;
import com.mamba.gloable.FolderManager;
import com.mamba.model.VLog;
import com.mamba.ui.test.CodecOutputSurface;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * @author jake
 * @since 2017/7/18 上午10:52
 */
@RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
public class MediaCodecDecodeTest implements Runnable, Test {
    private static final String MIME_TYPE = MediaFormat.MIMETYPE_VIDEO_AVC;
    private static final int TIME_OUT = 1000;
    private MediaCodec mediaCodec;
    private MediaExtractor extractor1;
    private FfmpegMediaExtractor extractor;
    private int width;
    private int height;
    private volatile boolean isStop = true;
    private DataOutputStream mOutput;
    private String filePath;
    Surface surface;
    private MediaCodec encoder;
    private int frameRate;
    private int frameIndex;
    private String h264;
    private String result;
    boolean isA = true;

    //    CodecOutputSurface outputSurface;
    public void toggle() {
        isA = !isA;
    }

    public void setSurface(Surface surface) {
        this.surface = surface;
    }

    public void start(String filePath) {
        if (isStop) {
            VLog.d("mpe4 file =" + filePath);
            this.filePath = filePath;
            new Thread(this).start();
            isStop = false;

        }
    }

    private void prepare() throws IOException {
        if (isA) {
            extractor1 = new MediaExtractor();
            extractor1.setDataSource(filePath);
            int numTracks = extractor1.getTrackCount();
            MediaFormat fformat1 = null;
            for (int i = 0; i < numTracks; i++) {
                fformat1 = extractor1.getTrackFormat(i);
                if (fformat1.getString(MediaFormat.KEY_MIME).startsWith("video")) {
                    extractor1.selectTrack(i);
                    break;
                }
            }
            VLog.d("fformat1:" + fformat1.toString());
            width = fformat1.getInteger(MediaFormat.KEY_WIDTH);
            height = fformat1.getInteger(MediaFormat.KEY_HEIGHT);
            try {
                frameRate = fformat1.getInteger(MediaFormat.KEY_FRAME_RATE);
            } catch (Exception e) {
                e.printStackTrace();
                frameRate = 25;
            }
        } else {
            extractor = new FfmpegMediaExtractor();
            extractor.setDataSource(filePath);
            int numTracks = extractor.getTrackCount();
            FfmpegMediaFormat fformat = null;
            for (int i = 0; i < numTracks; ++i) {
                fformat = extractor.getTrackFormat(i);
                if (fformat.getInteger(FfmpegMediaFormat.KEY_CODEC_TYPE) == FfmpegMediaFormat.CODE_TYPE_VIDEO) {
                    extractor.selectTrack(i);
                    break;
                }
            }
            if (fformat == null) {
                throw new IllegalArgumentException("can't find format!");
            }
            width = fformat.getInteger(FfmpegMediaFormat.KEY_WIDTH);
            height = fformat.getInteger(FfmpegMediaFormat.KEY_HEIGHT);
            frameRate = fformat.getInteger(FfmpegMediaFormat.KEY_FRAME_RATE);
        }
        VLog.d("isA=" + isA + "  prepare   width=" + width + "  height=" + height + "   frameRate=" + frameRate);
        mediaCodec = MediaCodec.createDecoderByType(MIME_TYPE);
        MediaFormat format = MediaFormat.createVideoFormat(MIME_TYPE, width, height);
//       format.setInteger(MediaFormat.KEY_COLOR_FORMAT,MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible);
//        format.setInteger(MediaFormat.KEY_STRIDE, width);
//        format.setInteger(MediaFormat.KEY_SLICE_HEIGHT, height);
//         outputSurface = new CodecOutputSurface(width, height);
//        mediaCodec.configure(format, surface, null, 0);
        mediaCodec.configure(format, null, null, 0);
        mediaCodec.start();
        VLog.d(format.toString());
        h264 = filePath + ".h264";
        result = filePath + "result.mp4";
        FileUtil.createFile(h264, true);
        frameIndex = 0;
        mOutput = new DataOutputStream(new FileOutputStream(h264));
        MediaFormat mediaFormat = MediaFormat.createVideoFormat(MIME_TYPE, width, height);
        mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible);
        mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);
        mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, frameRate);
        mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, (int) (2.0 * 1024 * 1024));
        VLog.d(mediaFormat.toString());
        encoder = MediaCodec.createEncoderByType(format.getString(MediaFormat.KEY_MIME));
        encoder.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
        encoder.start();
    }

    @Override
    public void run() {
        VLog.d("run");
        try {
            prepare();
        } catch (IOException e) {
            e.printStackTrace();
            return;
        }
        if (listener != null) {
            listener.onStart();
        }
        loopDecode();
        release();
        FfmpegFormatUtils.formatVideoStream(h264, result);
        isStop = true;
        if (listener != null) {
            listener.onFinish();
        }
    }

    private void release() {
        if (mediaCodec != null) {
            mediaCodec.stop();
            mediaCodec.release();
            mediaCodec = null;
        }
        if (encoder != null) {
            encoder.stop();
            encoder.release();
            encoder = null;
        }
        if (isA) {
            if (extractor1 != null) {
                extractor1.release();
                extractor1 = null;
            }
        } else {
            if (extractor != null) {
                extractor.release();
                extractor = null;
            }
        }
        try {
            mOutput.flush();
            mOutput.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    long last;

    private void loopDecode() {
        ByteBuffer[] inputBuffers = mediaCodec.getInputBuffers();
        ByteBuffer[] outputBuffers = mediaCodec.getOutputBuffers();
        MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
        VLog.d("loopDecode   start");
        boolean isEOS = false;
        while (true) {
            if (!isEOS) {
                int inIndex = mediaCodec.dequeueInputBuffer(TIME_OUT);
                if (inIndex >= 0) {
                    ByteBuffer buffer = inputBuffers[inIndex];
                    int sampleSize = -1;
                    long timestamp = -1;
                    if (isA) {
                        ByteBuffer byteBuffer = ByteBuffer.allocate(buffer.limit());
                        sampleSize = extractor1.readSampleData(buffer, 0);
                        timestamp = extractor1.getSampleTime();
                        if (sampleSize > 0) {
//                            byte[] da = new byte[sampleSize];
//                            byteBuffer.get(da);
//                            buffer.put(da);
                        }
                    } else {
//                        byte[] data=new byte[buffer.limit()];
//                        sampleSize=   extractor.readSampleData1(data);
                        byte[] data = extractor.readSampleData();
                        sampleSize = extractor.getSampleDataSize();
//                        sampleSize = extractor.readSampleData(buffer);
                        timestamp = extractor.getTimestamp();
                        buffer.clear();
                        buffer.put(data);
                    }
                    VLog.d("loopDecode  readSampleData end sampleSize" + sampleSize);
                    VLog.d("loopDecode  readSampleData end timestamp" + timestamp);
                    if (sampleSize < 0) {
                        mediaCodec.queueInputBuffer(inIndex, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                        isEOS = true;
                    } else {
                        mediaCodec.queueInputBuffer(inIndex, 0, sampleSize, timestamp, 0);
                        if (isA)
                            extractor1.advance();
                    }
                }
            }
            int outIndex = mediaCodec.dequeueOutputBuffer(info, TIME_OUT);
            VLog.d(" switch (outIndex)");
            switch (outIndex) {
                case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
                    outputBuffers = mediaCodec.getOutputBuffers();
                    VLog.d("dequeueOutputBuffer INFO_OUTPUT_BUFFERS_CHANGED!");
                    break;
                case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
                    VLog.d("New format " + mediaCodec.getOutputFormat().toString());
                    VLog.d("New format COLOR_FormatYUV420Flexible" + MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible);
                    VLog.d("New format COLOR_FormatYUV420Flexible" + MediaCodecInfo.CodecCapabilities.COLOR_TI_FormatYUV420PackedSemiPlanar);
                    VLog.d("New format COLOR_FormatYUV420Flexible" + MediaCodecInfo.CodecCapabilities.COLOR_Format32bitABGR8888);

                    break;
                case MediaCodec.INFO_TRY_AGAIN_LATER:
                    VLog.d("dequeueOutputBuffer timed out!");
                    break;
                default:
                    if (last == 0) {
                        last = System.currentTimeMillis();
                    }
                    long now = System.currentTimeMillis();
                    VLog.d("解码时间：" + (now - last) + " info.size  " + info.size);
                    last = now;
                    ByteBuffer buffer = outputBuffers[outIndex];
                    byte[] outData = new byte[info.size];
                    buffer.get(outData, 0, info.size);
                    encode(outData);
                    mediaCodec.releaseOutputBuffer(outIndex, true);
                    break;
            }
            // All decoded frames have been rendered, we can stop playing now
            if ((info.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
                VLog.d("OutputBuffer BUFFER_FLAG_END_OF_STREAM");
                break;
            }
        }
    }

    boolean is = false;

    private void encode(byte[] data) {
//        if (is) {
//            is = false;
//            return;
//        } else {
//            is = true;
//        }
        ByteBuffer[] inputBuffers = encoder.getInputBuffers();
        ByteBuffer[] outputBuffers = encoder.getOutputBuffers();
        MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
        int inIndex = encoder.dequeueInputBuffer(TIME_OUT);
        if (inIndex >= 0) {
            ByteBuffer inputBuffer = inputBuffers[inIndex];
            inputBuffer.clear();
            inputBuffer.put(data);
            encoder.queueInputBuffer(inIndex, 0, data.length, computePresentationTime(frameIndex++), 0);
        }
        int outIndex = encoder.dequeueOutputBuffer(info, TIME_OUT);
        if (outIndex >= 0) {
            while (outIndex >= 0) {
                VLog.d("while (outputBufferIndex >= 0)");
                ByteBuffer outputBuffer = outputBuffers[outIndex];
                byte[] outData = new byte[info.size];
                outputBuffer.get(outData);
                try {
                    mOutput.write(outData);
                } catch (Exception e) {
                    e.printStackTrace();
                } catch (Error e) {
                    e.printStackTrace();
                }
                encoder.releaseOutputBuffer(outIndex, false);
                outIndex = encoder.dequeueOutputBuffer(info, TIME_OUT);
            }
        }
    }


    private long computePresentationTime(int frameIndex) {
        return 132 + frameIndex * 1000000 / frameRate;
    }

    private Test.Listener listener;

    public void setListener(Test.Listener listener) {
        this.listener = listener;
    }


}
