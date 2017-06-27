//
// Created by yongfali on 2016/11/25.
//

#ifndef E_GPUIMAGE_PINKBLURFILTER_H
#define E_GPUIMAGE_PINKBLURFILTER_H
#include "GPUImageGaussianBlurFilter.h"

namespace e {

//表面模糊，类似双边滤波的
class GPUImagePinkBlurFilter3 
    : public GPUImageGaussianBlurFilter {
public:
    DEFINE_CREATE(GPUImagePinkBlurFilter3)
CONSTRUCTOR_ACCES :
    GPUImagePinkBlurFilter3();
    ~GPUImagePinkBlurFilter3();
public:
    virtual bool Initialize(void);
    void SetThresholdFactor(float threshold);
protected:
    virtual string CreateVertexShaderString(int radius, float threshold);
    virtual string CreateFragmentShaderString(int radius, float threshold);
    virtual void SetProgramUniforms(int index);

protected:
    bool SwitchProgram(const char *vShaderString, const char *fShaderString);
protected:
    GLint _texelThresholdUniform;
    GLint _texelThresholdUniform2;
    float _texelThreshold;
};
}
#endif // KUGOUEFFECT_GPUIMAGESURFACEBLURFILTER_H
