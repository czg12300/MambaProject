#ifndef E_GPUIMAGE_TWOPASSFILTER_H
#define E_GPUIMAGE_TWOPASSFILTER_H
#include "GPUImageFilter.h"

namespace e 
{
    //two input filter
	class GPUImageTwoPassFilter : public GPUImageFilter
	{
	public:
		DEFINE_CREATE(GPUImageTwoPassFilter)
	CONSTRUCTOR_ACCES:
		GPUImageTwoPassFilter(void);
		GPUImageTwoPassFilter(const char* firstVertexShaderString
			, const char* firstFragmentShaderString
			, const char* secondVertexShaderString
			, const char* secondFragmentShaderString);
		GPUImageTwoPassFilter(const char* firstFragmentShaderString
            , const char* secondFragmentShaderString);
		virtual ~GPUImageTwoPassFilter(void);
	public:
		using GPUImageFilter::Initialize;
		virtual bool Initialize(void);
		virtual bool Initialize(const char* firstVertexShaderString
			, const char* firstFragmentShaderString
			, const char* secondVertexShaderString
			, const char* secondFragmentShaderString);
		virtual bool Initialize(const char* firstFragmentShaderString
            , const char* secondFragmentShaderString);
		virtual void RenderToTexture(const GLfloat *vertices, const GLfloat *texCoords);
		virtual Ptr<GPUImageFramebuffer> & GetOutputFramebuffer(void);
		virtual void RemoveOutputFramebuffer(void);
	protected:
		virtual void InitializeSecondaryAttributes(void);
		virtual void SetProgramUniforms(const int index);
	protected:
		Ptr<GLProgram> _filterProgram2;
		GPUImageFramebufferPtr _outputFramebuffer2;
		GLuint _filterPositionAttribute2;
		GLuint _filterTextureCoordinateAttribute2;
		GLint _filterInputTextureUniform2;
		GLint _filterInputTexture2Uniform2;
	};
}

#endif//E_GPUIMAGE_2PASSFILTER_H