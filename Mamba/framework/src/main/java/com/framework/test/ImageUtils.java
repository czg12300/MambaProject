package com.framework.test;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Created by raomengyang on 4/25/16.
 */
public class ImageUtils {
    public static final int MEDIA_TYPE_IMAGE = 1;
    public static final int MEDIA_TYPE_VIDEO = 2;

    // save image to sdcard path: Pictures/MyTestImage/
    public static void saveImageData(byte[] imageData) {
        File imageFile = CacheDirUtil.getCameraJpeg();
        if (imageFile == null) return;
        try {
            FileOutputStream fos = new FileOutputStream(imageFile);
            fos.write(imageData);
            fos.close();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}