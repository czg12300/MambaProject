//
// Created by liyongfa on 2016/11/18.
//

#ifndef E_GPUIMAGE_ROTATIONMODE_H
#define E_GPUIMAGE_ROTATIONMODE_H

namespace e
{
    struct GPUImageRotationMode
    {
        int angle;
        int flipX;
        int flipY;

        GPUImageRotationMode();
        GPUImageRotationMode(int angle, int flipX, int flipY);
    };

    extern GPUImageRotationMode kGPUImageNoRotation;
    extern GPUImageRotationMode kGPUImageRotateLeft;
    extern GPUImageRotationMode kGPUImageRotateRight;
    extern GPUImageRotationMode kGPUImageFlipX;
    extern GPUImageRotationMode kGPUImageFlipY;
    extern GPUImageRotationMode kGPUImageRotateRightFlipX;
    extern GPUImageRotationMode kGPUImageRotateRightFlipY;
    extern GPUImageRotationMode kGPUImaeRotate90;
    extern GPUImageRotationMode kGPUImageRotate180;
    extern GPUImageRotationMode kGPUImageRotate270;

    bool GPUImageRotationSwapsWidthAndHeight(const GPUImageRotationMode & rotation);
}

#endif //E_GPUIMAGE_ROTATIONMODE_H
