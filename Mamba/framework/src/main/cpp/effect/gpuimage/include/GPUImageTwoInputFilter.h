#ifndef E_GPUIMAGE_TWOINPUTFILTER_H
#define E_GPUIMAGE_TWOINPUTFILTER_H
#include "GPUImageFilter.h"

namespace e {

	extern shader_t kGPUImageTwoInputVertexShaderString;
	extern shader_t kGPUImageTowInputFragmentShaderString;

	class GPUImageTwoInputFilter : public GPUImageFilter
	{
	public:
		DEFINE_CREATE(GPUImageTwoInputFilter)
	CONSTRUCTOR_ACCES:
		GPUImageTwoInputFilter(void);
		virtual ~GPUImageTwoInputFilter(void);
	public:
		virtual bool Initialize(void);
		virtual bool Initialize(const char* vShaderString, const char* fShaderString);
		virtual void InitializeAttributes(void);
		//input
		virtual void SetInputSize(Size textureSize, int index);
		virtual void SetInputRotation(GPUImageRotationMode & inputRotate, int index);
		virtual void SetInputFramebuffer(Ptr<GPUImageFramebuffer> & frameBuffer, int index);

		virtual void SetProgramUniforms(const int index);
		virtual int NextAvailableTextureIndex(void);
		virtual void NewFrameReady(Time frameTime, int texutreIndex);
		virtual void RenderToTexture(const GLfloat *vertices, const GLfloat *texCoords);
		virtual Size GetSizeOfRotated(Size rotateSize, int index);

		virtual void Reset(void);
		
		void DisableFirstFrameCheck(void);
		void DisableSecondFrameCheck(void);
	protected:
		Ptr<GPUImageFramebuffer> _secondInputFramebuffer;
		GLint _filterSecondTextureCoordinateAttribute;
		GLint _filterInputTextureUniform2;
		GPUImageRotationMode _inputRotation2;
		Time _firstFrameTime;
		Time _secondFrameTime;

		bool _hasSetFirstTexture;
		bool _hasReceivedFirstFrame;
		bool _hasReceivedSecondFrame;
		bool _firstFrameCheckDisabled;
		bool _secondFrameCheckDisabled;
	};
}

#endif//E_GPUIMAGE_TWOINPUTFILTER_H