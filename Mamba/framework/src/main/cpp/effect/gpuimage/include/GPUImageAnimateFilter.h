//
// Created by liyongfa on 2016/4/17.
//

#ifndef E_GPUIMAGE_ANIMEFILTER_H
#define E_GPUIMAGE_ANIMEFILTER_H
#include "GPUImageFilter.h"
#include "GPUImageMath.h"
#include "IFaceTracker.h"
#include "AnimateManager.h"

namespace e
{
	extern const char* const kGPUImageAnimateVertexShaderString;
	extern const char* const kGPUImageAnimateFragmentShaderString;

	class GPUImageAnimateFilter: public GPUImageFilter
	{
	public:
		DEFINE_CREATE(GPUImageAnimateFilter)
	CONSTRUCTOR_ACCES:
		GPUImageAnimateFilter(void);
		virtual ~GPUImageAnimateFilter(void);
	public:
		int OnTrackProc(void* data);
		void SetCacheSize(const int size);
		void SetAnimatePath(const char* path);

		virtual bool Initialize(void);
		virtual bool Initialize(const char* vShaderString, const char* fShaderString);
		virtual void RenderToTexture(const GLfloat* vertices, const GLfloat* textureCoordinates);
	protected:
		bool LoadAnimate(void);
		void CalcMVPMatrix(void);
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

		bool _loadAnimate;
		string _animePath;
		TrackResult _trackResult;
		AnimateConfig* _animeConfig;
		AnimateManager* _animeManager;
	};

	typedef Ptr<GPUImageAnimateFilter> GPUImageAnimateFilterPtr;
}

#endif //_E_GPUIMAGE_EFFECTFILTER_H_
