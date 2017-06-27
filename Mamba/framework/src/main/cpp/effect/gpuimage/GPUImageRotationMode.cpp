//
// Created by liyongfa on 2016/11/18.
//

#include "include/GPUImageRotationMode.h"

namespace e
{
    GPUImageRotationMode::GPUImageRotationMode()
    {
        this->angle = 0;
        this->flipX = 0;
        this->flipY = 0;
    }

    GPUImageRotationMode::GPUImageRotationMode(int angle, int flipX, int flipY)
    {
        this->angle = angle;
        this->flipX = flipX;
        this->flipY = flipY;
    }

    GPUImageRotationMode kGPUImageNoRotation(0, 0, 0);
    GPUImageRotationMode kGPUImageRotateLeft(90, 0, 0);
    GPUImageRotationMode kGPUImageRotateRight(270, 0, 0);
    GPUImageRotationMode kGPUImageFlipX(0, 1, 0);
    GPUImageRotationMode kGPUImageFlipY(0, 0, 1);
    GPUImageRotationMode kGPUImageRotateRightFlipX(270, 1, 0);
    GPUImageRotationMode kGPUImageRotateRightFlipY(270, 0, 1);
    GPUImageRotationMode kGPUImageRotate90(90, 0, 0);
    GPUImageRotationMode kGPUImageRotate180(180, 0, 0);
    GPUImageRotationMode kGPUImageRotate270(270, 0, 0);

    bool GPUImageRotationSwapsWidthAndHeight(const GPUImageRotationMode & rotation)
    {
        return (rotation.angle==90) || (rotation.angle==270);
    }
}
