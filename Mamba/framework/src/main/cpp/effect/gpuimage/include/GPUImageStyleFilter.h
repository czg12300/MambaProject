//
// Created by yongfali on 2017/3/16.
//

#ifndef KUGOUEFFECT_GPUIMAGESTYLEFILTER_H
#define KUGOUEFFECT_GPUIMAGESTYLEFILTER_H

#include "GPUImageBase.h"
#include "GPUImageFilter.h"

namespace e
{
	typedef unsigned char byte;
	class GPUImageStyleFilter : public GPUImageFilter
	{
	public:
		DEFINE_CREATE(GPUImageStyleFilter)
	CONSTRUCTOR_ACCES:
		GPUImageStyleFilter(void);
		~GPUImageStyleFilter(void);
	public:
		virtual bool Initialize(void);
		void SetSamplePoints(int channel, double* points, int count);
	protected:
		void MakeSamplesDefault(void);
		void UpdateCurveTexture(void);
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
#ifndef TEXTURE_CACHE_ENABLE	
		EGLContext _eglContext;
		GLuint _curveTexture;
#else
		Ptr<GPUImageTexture> _curveTexture;
		Ptr<GPUImageTextureCache> _textureCache;
#endif
		GLint _filterCurveTextureUniform;
		
		double* _samplePoints[4];
		bool _isNeedUpdate;
	};
}

#endif //KUGOUEFFECT_GPUIMAGESTYLEFILTER_H
