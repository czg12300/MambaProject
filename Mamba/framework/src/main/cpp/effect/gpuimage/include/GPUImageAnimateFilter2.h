//
// Created by liyongfa on 2016/4/17.
//

#ifndef E_GPUIMAGE_ANIMEFILTER2_H
#define E_GPUIMAGE_ANIMEFILTER2_H
#include "GPUImageFilter.h"
#include "GPUImageMath.h"
#include "IFaceTracker.h"
#include "AnimateManager.h"

namespace e
{
	extern const char* const kGPUImageAnimateVertexShaderString2;
	extern const char* const kGPUImageAnimateFragmentShaderString2;
	//待优化的一个类，没完全实现
	class GPUImageAnimateFilter2: public GPUImageFilter
	{
	public:
		DEFINE_CREATE(GPUImageAnimateFilter2)
	CONSTRUCTOR_ACCES:
		GPUImageAnimateFilter2(void);
		virtual ~GPUImageAnimateFilter2(void);
	public:
		//tracker callback
		int OnTrackProc(void* data);
		virtual bool Initialize(void);
		virtual bool Initialize(const char* vShaderString, const char* fShaderString);
		virtual void RenderToTexture(const GLfloat* vertices, const GLfloat* textureCoordinates);
	protected:
		bool LoadAnimate(const char* path);
		void CalcTransformMatrix(void);

		virtual void SetProgramUniforms(int textureIndex);
		virtual Size GetSizeOfFBO(void) const;
		virtual void SetupFilter(Size size);
	protected:
		//Uniform
		GLint _modelViewProjectionUniform;
		GLuint _animeTexture;
		//保证不变形
		Matrix4 _modelMatrix;
		Matrix4 _viewMatrix;
		Matrix4 _projectionMatrix;
		Matrix4 _modelViewProjectionMatrix;

		TrackResult _trackResult;
		AnimateConfig* _animeConfig;
		AnimateManager* _animeManager;
	};

	typedef Ptr<GPUImageAnimateFilter2> GPUImageAnimateFilter2Ptr;
}

#endif //_E_GPUIMAGE_EFFECTFILTER_H_
