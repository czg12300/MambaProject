//
// Created by yongfali on 2016/11/14.
//

#ifndef E_GPUIMAGEHIGHPASSFILTER_H
#define E_GPUIMAGEHIGHPASSFILTER_H
#include "GPUImageFilter.h"

namespace e
{
	extern shader_t kGPUImageHighpassVertexShaderString;
	extern shader_t kGPUImageHighpassFragmentShaderString;

	class GPUImageHighpassFilter
		: public GPUImageFilter
	{
	public:
		DEFINE_CREATE(GPUImageHighpassFilter)
	CONSTRUCTOR_ACCES:
		GPUImageHighpassFilter();
		~GPUImageHighpassFilter();
	public:
		virtual bool Initialize();
	private:
		virtual void SetupFilter(Size size);
		virtual void SetProgramUniforms(int index);
		void CalcKernels(int radius, float sigma);
	private:
		GLint _texelKernelsUniform;
		GLint _texelWidthOffsetUniform;
		GLint _texelHeightOffsetUniform;
		GLfloat _texelWidthOffset;
		GLfloat _texelHeightOffset;

		GLint _radius;
		GLfloat _sigma;
		GLfloat* _kernels;
		bool _isNeedUpdateKernel;
	};
}

#endif //E_GPUIMAGEHIGHPASSFILTER_H
