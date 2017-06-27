#ifndef _E_GPUIMAGE_BILATERALFILTER_H_
#define _E_GPUIMAGE_BILATERALFILTER_H_
#include "GPUImageTwoPassTextureSamplingFilter.h"

namespace e
{
    extern shader_t kGPUImageBilateralVertexShaderString;
    extern shader_t kGPUImageBilateralFragmentShaderString;
    
	class GPUImageBilateralFilter : public GPUImageTwoPassTextureSamplingFilter
	{
	public:
		DEFINE_CREATE(GPUImageBilateralFilter)
	CONSTRUCTOR_ACCES:
		GPUImageBilateralFilter(void);
		virtual ~GPUImageBilateralFilter(void);
	public:
		virtual bool Initialize(void);
		void SetDistanceFactor(float factor);
		void SetTexelSpacingMultiplier(float multiplier);
	protected:
		virtual void SetProgramUniforms(int index);
	protected:
		GLint firstDistanceFactorUniform;
		GLint secondDistanceFactorUniform;

		float distanceFactor;
		float texelSpacingMultiplier;
	};  

	typedef Ptr<GPUImageBilateralFilter> GPUImageBilateralFilterPtr;
}
#endif