
package com.framework;

import android.os.Environment;

import java.io.File;

/**
 * 描述：
 *
 * @author walljiang
 * @since 2017/04/09 11:46
 */

public class PathGeneratorUtil {

    public static String rootPath = Environment.getExternalStorageDirectory().getAbsolutePath()
            + File.separator + "AClip";

    public static String generateVideoFilterFile(String srcPath) {
        File src1 = new File(srcPath);
        if (src1.exists()) {
            String name1 = src1.getName();
            File outFile = new File(rootPath + File.separator + "videoFilter",
                    name1.split("\\.")[0] + "_filter." + name1.split("\\.")[1]);
            if (!outFile.getParentFile().exists()) {
                outFile.getParentFile().mkdir();
            }
            return outFile.getAbsolutePath();
        }
        return null;
    }

    public static String generateVideoTransCodeFile(String srcPath) {
        File src1 = new File(srcPath);
        if (src1.exists()) {
            String name1 = src1.getName();
            File outFile = new File(rootPath + File.separator + "videoTranscode",
                    name1.split("\\.")[0] + "_transcode." + name1.split("\\.")[1]);
            if (!outFile.getParentFile().exists()) {
                outFile.getParentFile().mkdir();
            }
            return outFile.getAbsolutePath();
        }
        return null;
    }

    public static String generateReencodeFile(String srcpath) {
        File src1 = new File(srcpath);
        if (src1.exists()) {
            String name1 = src1.getName();
            String[] name1s = name1.split("\\.");
            String name = name1s[0] + "_reencode." + name1s[1];
            File outFile = new File(rootPath + File.separator + "reencode", name);
            if (!outFile.getParentFile().exists()) {
                outFile.getParentFile().mkdir();
            }
            return outFile.getAbsolutePath();
        }
        return null;
    }

    public static String generateAudioFile(String src1Path, String src2Path, String suffix) {
        File src1 = new File(src1Path);
        File src2 = new File(src2Path);
        if (src1.exists() && src2.exists()) {
            String name1 = src1.getName();
            String name2 = src2.getName();
            String[] name1s = name1.split("\\.");
            String name = name1s[0] + "_" + name2.split("\\.")[0] + "." + name1s[1];
            // String name = name1s[0]+"_"+name2.split("\\.")[0]+".aac";
            File outFile = new File(rootPath + File.separator + suffix, name);
            if (!outFile.getParentFile().exists()) {
                outFile.getParentFile().mkdir();
            }
            return outFile.getAbsolutePath();
        }
        return null;
    }

    public static String generateAudioVolumnFile(String srcFile) {
        File src = new File(srcFile);
        if (src.exists()) {
            String srcName = src.getName();
            String name = srcName.split("\\.")[0] + "_vol" + ".pcm";
            File outFile = new File(rootPath + File.separator + "audioVolumn", name);
            if (!outFile.getParentFile().exists()) {
                outFile.getParentFile().mkdir();
            }
            return outFile.getAbsolutePath();
        }
        return null;
    }

    public static String generateAudioRateFile(String srcFile) {
        File src = new File(srcFile);
        if (src.exists()) {
            String srcName = src.getName();
            String name = srcName.split("\\.")[0] + "_rate" + "." + srcName.split("\\.")[1];
            File outFile = new File(rootPath + File.separator + "audioRate", name);
            if (!outFile.getParentFile().exists()) {
                outFile.getParentFile().mkdir();
            }
            return outFile.getAbsolutePath();
        }
        return null;
    }

    public static String generateAACRecordFile() {
        String time = System.currentTimeMillis() + "";
        String path = rootPath + File.separator + "audioRecord";
        File file = new File(path, time + ".aac");
        if (!file.getParentFile().exists()) {
            file.getParentFile().mkdir();
        }
        return file.getAbsolutePath();
    }

    public static String generateAACDecodeFile(String srcFile) {
        String path = rootPath + File.separator + "audioDecode";
        File src = new File(srcFile);
        File file = new File(path, src.getName().split("\\.")[0] + ".pcm");
        if (!file.getParentFile().exists()) {
            file.getParentFile().mkdir();
        }
        return file.getAbsolutePath();
    }
}
