#ifndef E_GPUIMAGE_PINK_BLURFILTER_H
#define E_GPUIMAGE_PINK_BLURFILTER_H

#include "GPUImageFilter.h"
namespace e
{
	//多采样单通道高斯模糊，适用于性能好的设备
	class GPUImagePinkBlurFilter : public GPUImageFilter
	{
	public:
		DEFINE_CREATE(GPUImagePinkBlurFilter)
	CONSTRUCTOR_ACCES:
		GPUImagePinkBlurFilter();
		virtual ~GPUImagePinkBlurFilter();
	public:
		virtual bool Initialize(void);
		virtual void SetProgramUniforms(int index);
		virtual void SetupFilter(Size size);
		void SetTexelSpacingMultiplier(float multiplier);
	private:
		GLint sampleOffsetsUniform;
		GLint sampleWeightsUniform;
		GLfloat sampleOffsetWidth;
		GLfloat sampleOffsetHeight;
		GLfloat texelSpacingMultiplier;
		GLfloat texelKernels[20][2];
	};

    typedef Ptr<GPUImagePinkBlurFilter> GPUImagePinkBlurFilterPtr;
}

#endif //E_GPUIMAGE_PINK_BLURFILTER_H
