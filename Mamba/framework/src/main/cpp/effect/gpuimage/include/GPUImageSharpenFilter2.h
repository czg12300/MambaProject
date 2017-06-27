//
// Created by yongfali on 2016/11/14.
//

#ifndef E_GPUIMAGESHARPENFILTER2_H
#define E_GPUIMAGESHARPENFILTER2_H

#include "GPUImageTwoInputFilter.h"

namespace e
{
	extern shader_t kGPUImageSharpen2VertexShaderString;
	extern shader_t kGPUImageSharpen2FragmentShaderString;

	class GPUImageSharpenFilter2
		: public GPUImageTwoInputFilter
	{
	public:
		DEFINE_CREATE(GPUImageSharpenFilter2)
	CONSTRUCTOR_ACCES:
		GPUImageSharpenFilter2();
		~GPUImageSharpenFilter2();
	public:
		virtual bool Initialize();
		virtual void SetProgramUniforms(int index);
		void SetSharpness(float value);
	protected:
		GLfloat alphaFactor;
	};
}

#endif //E_GPUIMAESHARPENFILTER2_H
