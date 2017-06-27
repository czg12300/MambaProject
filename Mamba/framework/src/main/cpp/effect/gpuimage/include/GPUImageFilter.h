//
// Created by yongfali on 2016/3/25.
//

#ifndef E_GPUIMAGE_FILTER_H
#define E_GPUIMAGE_FILTER_H

#include "GPUImageBase.h"
#include "GPUImageOutput.h"

namespace e
{
	extern shader_t kGPUImageVertexShaderString;
	extern shader_t kGPUImagePassthroughFragmentShaderString;

	//only one input
	class GPUImageFilter 
		: public GPUImageInput
		, public GPUImageOutput
	{
	public:
		DEFINE_CREATE(GPUImageFilter)
	CONSTRUCTOR_ACCES:
		GPUImageFilter(void);
		virtual ~GPUImageFilter(void);
	public:
		virtual void SetInputSize(Size size, int index);
		virtual void SetInputRotationMode(GPUImageRotationMode & rotateMode, int index);
		using GPUImageInput::SetInputFramebuffer;
		using GPUImageOutput::SetInputFramebuffer;
		virtual void SetInputFramebuffer(Ptr<GPUImageFramebuffer> & frameBuffer, int index);
		virtual void SetInputTextureOptions(GPUImageTextureOptions & textureOptions, int index);
		virtual void EndProcessing(void);
		virtual bool IsEnable(void);
		virtual int NextAvailableTextureIndex(void);
		//getter
		virtual Size GetSizeOfFBO(void) const;
		virtual Size GetOutputFrameSize(void) const;
		virtual GPUImageTextureOptions GetOutputTextureOptions(void) const;
		
		GLProgramPtr GetFilterProgram(void);
		GPUImageFramebufferPtr GetFirstInputFramebuffer(void);
		GPUImageTextureOptions GetInputTextureOptions(void) const;
		//to rotate
		virtual Size GetSizeOfRotated(Size rotateSize,int index);
		//链式调用离屏渲染
		virtual void NewFrameReady(Time frameTime, int textureIndex);
		virtual void RenderToTexture(const GLfloat* vertices, const GLfloat* texCoords);
		virtual void CallNextTargets(Time frameTime);	
		//init
		virtual bool Initialize(void);
		virtual bool Initialize(const char* vShaderString, const char* fShaderString);
		virtual bool Initialize(const char* fShaderString);

		virtual void Reset(void);
	protected:
		virtual void InitializeAttributes(void);
		virtual void SetProgramUniforms(int index);
		virtual void SetupFilter(Size size);
		virtual GPUImageFramebufferPtr FetchFramebuffer(const Size size);
	protected:
		string _filterName;
		bool _enable;
        GLfloat _bgColorRed;
        GLfloat _bgColorGreen;
        GLfloat _bgColorBlue;
        GLfloat _bgColorAlpha;
		Size _inputTextureSize;
		Ptr<GLProgram> _filterProgram;
		GLuint _filterPositionAttribute;
		GLuint _filterTextureCoordinateAttribute;
		GLint _filterInputTextureUniform;
		GPUImageRotationMode _inputRotation;
		GPUImageTextureOptions _inputTextureOptions;
		Ptr<GPUImageFramebuffer> _firstInputFramebuffer;
		Ptr<GPUImageFramebufferCache> _frameBufferCache;
	};

	typedef Ptr<GPUImageFilter> GPUImageFilterPtr;
}

#endif //_E_GLBASEFILTER_H_
