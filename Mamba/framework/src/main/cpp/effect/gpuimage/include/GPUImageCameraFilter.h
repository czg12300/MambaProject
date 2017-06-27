//
// Created by yongfali on 2016/4/9.
//

#ifndef E_GPUIMAGE_CAMERAFILTER_H
#define E_GPUIMAGE_CAMERAFILTER_H

#include "GPUImageBase.h"
#include "GPUImageOutput.h"

namespace e 
{
#ifndef SHADER_STRING
#	define SHADER_STRING(x) #x
#endif

	extern shader_t kGPUImageCameraVertexShaderString;
	extern shader_t kGPUImageCameraFragmentShaderString;

	class GPUImageCameraFilter : public GPUImageOutput
	{
	public:
		DEFINE_CREATE(GPUImageCameraFilter)
	CONSTRUCTOR_ACCES:
		GPUImageCameraFilter(void);
		virtual ~GPUImageCameraFilter(void);
	public:
        //set if do scale
		virtual void SetOutputSize(Size size);
		//rendering
		virtual void SetInputSize(Size size, int index);
		virtual void SetInputRotationMode(GPUImageRotationMode & rotationMode, int index);
		virtual void RenderToTexture(void* data, int width, int height, int format);
		//init
		virtual bool Initialize(void);
		virtual bool Initialize(const char *vShaderString, const char* fShaderString);
	protected:
		virtual void InitializeAttributes(void);
		virtual Size GetSizeOfFBO(void);
		virtual Size GetSizeOfRotated(Size size);
		//virtual Size GetOutputFrameSize(void) const;

		bool CreateTextures(void);
		void UpdateTextures(unsigned char* data, int width, int height, int format);
		void ConvertYUV2RGBAOutput(void);
		void CallNextTargets(Time time);
		//此函数只能用于这个类中
		GLfloat* GetTexCoords(const GPUImageRotationMode & rotateMode);
		GLfloat* GetCropTexCoords(const GPUImageRotationMode & rotateMode);
	protected:
		Size _inputSize;
		Size _outputSize;
		Ptr<GLProgram> _filterProgram;
        Ptr<GPUImageFramebufferCache> _frameBufferCache;
#ifndef TEXTURE_CACHE_ENABLE
		EGLContext _eglContext;
		GLuint _luminanceTexture;
		GLuint _chrominanceTexture;
#else
        Ptr<GPUImageTexture> _luminanceTexture;
        Ptr<GPUImageTexture> _chrominanceTexture;
        Ptr<GPUImageTextureCache> _textureCache;
#endif
    	GLuint _filterPositionAttribute;
    	GLuint _filterTextureCoordinateAttribute;
    	GLint _luminanceTextureUniform;
    	GLint _chrominanceTextureUniform;
    	GLint _conversionMatrixUniform;
	
		GPUImageRotationMode _inputRotation;
		GLfloat _conversionMatrix[9];

		bool _isNeedCrop;
	};

	typedef Ptr<GPUImageCameraFilter> GPUImageCameraFilterPtr;
}

#endif //_E_GPUIMAGE_COLORFILTER_H_
