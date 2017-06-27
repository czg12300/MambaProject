#ifndef E_GPUIMAGE_GAUSSIANBLUR_H
#define E_GPUIMAGE_GAUSSIANBLUR_H
#include "GPUImageTwoPassTextureSamplingFilter.h"

namespace e
{
    class GPUImageGaussianBlurFilter : public GPUImageTwoPassTextureSamplingFilter
    {
    public:
        DEFINE_CREATE(GPUImageGaussianBlurFilter)
    CONSTRUCTOR_ACCES:
        GPUImageGaussianBlurFilter(void);
		virtual ~GPUImageGaussianBlurFilter(void);
    public:
        virtual bool Initialize(void);
		virtual void RenderToTexture(const GLfloat* vertices, const GLfloat* texCoords);
		void SetTexelSpacingMultiplier(float texelSpacingMultiplier);
		void SetBlurRadius(float blurRadius);

		virtual string CreateVertexShaderString(int blurRadius, float sigma);
		virtual string CreateFragmentShaderString(int blurRadius, float sigma);
	protected:
		bool SwitchProgram(const char* vShaderString, const char* fShaderString);
    protected:
		float _sigma;
        float _blurRadius;
        float _texelSpacingMultiplier;
		bool  _isNeedUpdateProgram;
    };

    typedef Ptr<GPUImageGaussianBlurFilter> GPUImageGaussianBlurFilterPtr;
}
#endif