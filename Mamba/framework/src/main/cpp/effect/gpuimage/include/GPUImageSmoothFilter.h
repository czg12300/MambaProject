#ifndef E_GPUIMAGE_SMOOTH_H
#define E_GPUIMAGE_SMOOTH_H
#include "GPUImageTwoInputFilter.h"

namespace e 
{
	class GPUImageSmoothFilter : public GPUImageTwoInputFilter
	{
	public:
		DEFINE_CREATE(GPUImageSmoothFilter)
	CONSTRUCTOR_ACCES:
		GPUImageSmoothFilter(void);
		virtual ~GPUImageSmoothFilter(void);
	public:
		bool Initialize(void);
		void RenderToTexture(const GLfloat* vertices, const GLfloat* texCoords);
		void SetBlendParams(int index, float value);

		void SetSmoothFactor(float value);
		void SetLightenFactor(float value);
		void SetSoftlightFactor(float value);
		void SetSaturateFactor(float value);
	protected:
		void UpdateHLTexture(int pass = 4);
		void SetProgramUniforms(const int index);
	protected:
		GLint _blendParamsUniform;
		GLint _hardlightTextureUniform;
		GLuint _hardlightTexture;
		GLfloat _blendParams[4];
	};

	typedef Ptr<GPUImageSmoothFilter> GPUImageSmoothFilterPtr;
}

#endif