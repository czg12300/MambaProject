#ifndef E_GPUIMAGE_CURVEFILTER_H
#define E_GPUIMAGE_CURVEFILTER_H
#include "GPUImageFilter.h"

namespace e 
{
    extern shader_t kGPUImageCurveVertexShaderString;
    extern shader_t kGPUImageCurveFragmentShaderString;
    
    typedef unsigned char byte;
    
	class GPUImageCurveFilter : public GPUImageFilter
	{
	public:
		DEFINE_CREATE(GPUImageCurveFilter)
	CONSTRUCTOR_ACCES:
		GPUImageCurveFilter(void);
		virtual ~GPUImageCurveFilter(void);
		virtual bool Initialize(void);
	protected:
		bool MakeCurveTexture(void);
		virtual void RenderToTexture(const float* vertices, const float* texCoords);
	protected:
		static int Calculate(byte* samples
			, const int sampleCount
			, const double* points
			, const int pointCount);

		static int Calculate(double* samples
			, const int sampleCount
			, const double* points
			, const int pointCount);

		static void Plot(double* samples
			, const int sampleCount
			, const double* points
			, const int pointCount
			, int p1
			, int p2
			, int p3
			, int p4);
	protected:
		GLuint _curveTexture;
		GLint _filterCurveTextureUniform;
	};

	typedef Ptr<GPUImageCurveFilter> GPUImageCurveFilterPtr;
}
#endif