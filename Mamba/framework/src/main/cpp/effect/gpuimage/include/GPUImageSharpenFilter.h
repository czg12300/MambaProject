#ifndef E_GPUIMAGE_SHARPENFILTER_H
#define E_GPUIMAGE_SHARPENFILTER_H
#include "GPUImageFilter.h"

namespace e 
{
    extern shader_t kGPUImageSharpenVertexShaderString;
    extern shader_t kGPUImageSharpenFragmentShaderString;
    
	class GPUImageSharpenFilter : public GPUImageFilter
	{
	public:
		DEFINE_CREATE(GPUImageSharpenFilter)
	CONSTRUCTOR_ACCES:	
		GPUImageSharpenFilter();
		virtual ~GPUImageSharpenFilter();
	public:
		void SetSharpness(float sharpness);
		virtual bool Initialize(void);
		virtual void SetProgramUniforms(int index);
		virtual void RenderToTexture(const float* vertices, const float* texCoords);
	protected:
		virtual void SetupFilter(Size size);
	protected:
		GLint _sharpnessUniform;
		GLint _imageWidthFactorUniform;
		GLint _imageHeightFactorUniform;

		GLfloat _imageWidthOffset;
		GLfloat _imageHeightOffset;
		GLfloat _sharpness;
	};

	typedef Ptr<GPUImageSharpenFilter> GPUImageSharpenFilterPtr;
}

#endif//E_GPUIMAGE_SHARPENFILTER_H