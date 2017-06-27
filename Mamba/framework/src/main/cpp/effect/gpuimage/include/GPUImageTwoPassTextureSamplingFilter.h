#ifndef E_GPUIAMGE_TWOPASSTEXTURESAMPLINGFILTER_H
#define E_GPUIAMGE_TWOPASSTEXTURESAMPLINGFILTER_H
#include "GPUImageTwoPassFilter.h"

namespace e
{
	class GPUImageTwoPassTextureSamplingFilter : public GPUImageTwoPassFilter
	{
	public:
		DEFINE_CREATE(GPUImageTwoPassTextureSamplingFilter)
	CONSTRUCTOR_ACCES:
		GPUImageTwoPassTextureSamplingFilter(void);
		virtual ~GPUImageTwoPassTextureSamplingFilter(void);
	public:
		virtual bool Initialize(void);
		virtual bool Initialize(const char* vShaderString1
            , const char* fShaderString1
            , const char* vShaderString2
            , const char* fShaderString2);
		virtual void SetProgramUniforms(int index);
		virtual void SetupFilter(Size size);
		void SetVerticalTexelSpacing(const float texelSpacing);
		void SetHorizontalTexelSpacing(const float texelSpacing);
	protected:
		GLint _verticalPassTexelWidthOffsetUniform;
		GLint _verticalPassTexelHeightOffsetUniform;
		GLint _horizontalPassTexelWidthOffsetUniform;
		GLint _horizontalPassTexelHeightOffsetUniform;

		float _verticalPassTexelWidthOffset;
		float _verticalPassTexelHeightOffset;
		float _horizontalPassTexelWidthOffset;
		float _horizontalPassTexelHeightOffset;

		float _verticalTexelSpacing;
		float _horizontalTexelSpacing;
	};
}
#endif