package com.mamba.model.play;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;

import com.framework.ndk.videoutils.FfmpegFormatUtils;
import com.framework.utils.FileUtil;
import com.mamba.gloable.FolderManager;
import com.mamba.model.VLog;

import java.io.DataOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * @author jake
 * @since 2017/7/18 上午10:52
 */

public class MediaCodecDecodeTest implements Runnable {
    private static final String MIME_TYPE = MediaFormat.MIMETYPE_VIDEO_AVC;
    private MediaCodec mediaCodec;
    private MediaExtractor extractor;
    private int width;
    private int height;
    ByteBuffer readBuffer;
    private volatile boolean isStop = true;
    private DataOutputStream mOutput;
    private String filePath;
    Surface surface;
    private MediaCodec encoder;
    private int frameRate;
    private int frameIndex;
    private String h264;
    private String result;

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

    private String getOutYuv(String name) {
        String file = FolderManager.ROOT_FOLDER + name + ".h264";
        VLog.d("yuv file =" + file);
        return file;
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
        FfmpegFormatUtils.formatVideoStream(h264, result);
        release();
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
        if (extractor != null) {
            extractor.release();
            extractor = null;
        }
    }

    long last;

    private void loopDecode() {
        ByteBuffer[] inputBuffers = mediaCodec.getInputBuffers();
        ByteBuffer[] outputBuffers = mediaCodec.getOutputBuffers();
        MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
        boolean isEOS = false;
        while (true) {
            if (!isEOS) {
                int inIndex = mediaCodec.dequeueInputBuffer(10);
                if (inIndex >= 0) {
                    ByteBuffer buffer = inputBuffers[inIndex];
                    int sampleSize = extractor.readSampleData(buffer, 0);
                    if (sampleSize < 0) {
                        // We shouldn't stop the playback at this point, just pass the EOS
                        // flag to mediaCodec, we will get it again from the
                        // dequeueOutputBuffer
                        mediaCodec.queueInputBuffer(inIndex, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                        isEOS = true;
                    } else {
                        mediaCodec.queueInputBuffer(inIndex, 0, sampleSize, extractor.getSampleTime(), 0);
                        extractor.advance();
                    }
                }
            }

            int outIndex = mediaCodec.dequeueOutputBuffer(info, 10);
            switch (outIndex) {
                case MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED:
                    outputBuffers = mediaCodec.getOutputBuffers();
                    break;
                case MediaCodec.INFO_OUTPUT_FORMAT_CHANGED:
//                    VLog.d("New format " + mediaCodec.getOutputFormat());
                    break;
                case MediaCodec.INFO_TRY_AGAIN_LATER:
//                    VLog.d("dequeueOutputBuffer timed out!");
                    break;
                default:
                    if (last == 0) {
                        last = System.currentTimeMillis();
                    }
                    long now = System.currentTimeMillis();
                    VLog.d("解码时间：" + (now - last));
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
        if (is) {
            is = false;
            return;
        } else {
            is = true;
        }
        ByteBuffer[] inputBuffers = encoder.getInputBuffers();
        ByteBuffer[] outputBuffers = encoder.getOutputBuffers();
        MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();
        int inIndex = encoder.dequeueInputBuffer(10);
        if (inIndex >= 0) {
            ByteBuffer inputBuffer = inputBuffers[inIndex];
            inputBuffer.clear();
            inputBuffer.put(data);
            encoder.queueInputBuffer(inIndex, 0, data.length, computePresentationTime(frameIndex++), 0);
        }
        int outIndex = encoder.dequeueOutputBuffer(info, 10);
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
                outIndex = encoder.dequeueOutputBuffer(info, 10);
            }
        }
    }

    private void prepare() throws IOException {
        extractor = new MediaExtractor();
        extractor.setDataSource(filePath);
        int numTracks = extractor.getTrackCount();
        MediaFormat format = null;
        for (int i = 0; i < numTracks; ++i) {
            format = extractor.getTrackFormat(i);
            if (format.getString(MediaFormat.KEY_MIME).startsWith("video")) {
                extractor.selectTrack(i);
                break;
            }
        }
        if (format == null) {
            throw new IllegalArgumentException("can't find format!");
        }
        String mime = format.getString(MediaFormat.KEY_MIME);
        mediaCodec = MediaCodec.createDecoderByType(mime);
        mediaCodec.configure(format, surface, null, 0);
        mediaCodec.start();
        frameRate = format.getInteger(MediaFormat.KEY_FRAME_RATE);
        width = format.getInteger(MediaFormat.KEY_WIDTH);
        height = format.getInteger(MediaFormat.KEY_HEIGHT);
        readBuffer = ByteBuffer.allocate(width * height * 3 / 2);
//        readBuffer = ByteBuffer.allocate(format.getInteger(MediaFormat.KEY_MAX_INPUT_SIZE));
        VLog.d(format.toString());

        h264 = filePath + ".h264";
        result = filePath + "result.mp4";
        FileUtil.createFile(h264, true);
        frameIndex = 0;
        mOutput = new DataOutputStream(new FileOutputStream(h264));
        MediaFormat mediaFormat = MediaFormat.createVideoFormat(format.getString(MediaFormat.KEY_MIME), width, height);
        mediaFormat.setInteger(MediaFormat.KEY_COLOR_FORMAT, MediaCodecInfo.CodecCapabilities.COLOR_FormatYUV420Flexible);
        mediaFormat.setInteger(MediaFormat.KEY_I_FRAME_INTERVAL, 1);
        mediaFormat.setInteger(MediaFormat.KEY_FRAME_RATE, format.getInteger(MediaFormat.KEY_FRAME_RATE));
        mediaFormat.setInteger(MediaFormat.KEY_BIT_RATE, (int) (2.0 * 1024 * 1024));
        VLog.d(mediaFormat.toString());
        encoder = MediaCodec.createEncoderByType(format.getString(MediaFormat.KEY_MIME));
        encoder.configure(mediaFormat, null, null, MediaCodec.CONFIGURE_FLAG_ENCODE);
        encoder.start();
    }

    private long computePresentationTime(int frameIndex) {
        return 132 + frameIndex * 1000000 / frameRate;
    }

    private Listener listener;

    public void setListener(Listener listener) {
        this.listener = listener;
    }

    public static interface Listener {
        void onStart();

        void onFinish();
    }
}
