#include "include/GPUImageTwoInputFilter.h"
#include "include/GPUImageBase.h"

namespace e 
{
	shader_t kGPUImageTwoInputVertexShaderString = SHADER_STRING
	(
		attribute vec4 position;
		attribute vec4 inputTextureCoordinate;
		attribute vec4 inputTextureCoordinate2;

		varying vec2 textureCoordinate;
		varying vec2 textureCoordinate2;

		void main() 
		{
			gl_Position = position;
			textureCoordinate = inputTextureCoordinate.xy;
			textureCoordinate2 = inputTextureCoordinate2.xy;
		}
	);

	shader_t kGPUImageTwoInputFragmentShaderString = SHADER_STRING
	(
		uniform sampler2D inputImageTexture;//luminance texture
		uniform sampler2D inputImageTexture2;//chrominance texture
		varying highp vec2 textureCoordinate;
		varying highp vec2 textureCoordinate2;

		void main() 
        {
			lowp vec4 color = texture2D(inputImageTexture, textureCoordinate);
            lowp vec4 color2 = texture2D(inputImageTexture2, textureCoordinate2);
			gl_FragColor = mix(color, color2, color2.a);
		}
	);

	GPUImageTwoInputFilter::GPUImageTwoInputFilter(void)
	{
		_filterSecondTextureCoordinateAttribute = 0;
		_filterInputTextureUniform2 = -1;
		_inputRotation2 = kGPUImageNoRotation;
		_hasSetFirstTexture = false;
		_hasReceivedFirstFrame = false;
		_hasReceivedSecondFrame = false;
		_firstFrameCheckDisabled = false;
		_secondFrameCheckDisabled = false;
	}

	GPUImageTwoInputFilter::~GPUImageTwoInputFilter(void)
	{
		_secondInputFramebuffer = 0;
	}

	bool GPUImageTwoInputFilter::Initialize(void)
	{
		return Initialize(kGPUImageTwoInputVertexShaderString, kGPUImageTwoInputFragmentShaderString);
	}

	bool GPUImageTwoInputFilter::Initialize(const char* vShaderString, const char* fShaderString)
	{
	    if (!GPUImageFilter::Initialize(vShaderString, fShaderString))
	    {
			LOGE("GPUImageTwoInputFilter initialize failed!");
	        return false;
	    }

	    _firstFrameTime = kTimeInvalid;
	    _secondFrameTime = kTimeInvalid;
	    _filterSecondTextureCoordinateAttribute = _filterProgram->GetAttributeIndex("inputTextureCoordinate2");
	    _filterInputTextureUniform2 = _filterProgram->GetUniformLocation("inputImageTexture2");
	    glEnableVertexAttribArray(_filterSecondTextureCoordinateAttribute);

	    // LOGV("GPUImageTwoInputFilter Initialize ok: %d,%d",
	    // 	_filterSecondTextureCoordinateAttribute,
	    // 	_filterInputTextureUniform2);
        return true;
	}

	void GPUImageTwoInputFilter::InitializeAttributes(void) 
	{
		GPUImageFilter::InitializeAttributes();
		_filterProgram->AddAttribute("inputTextureCoordinate2");
	}

	void GPUImageTwoInputFilter::SetProgramUniforms(const int index) 
	{
        GPUImageFilter::SetProgramUniforms(index);
	}

    int GPUImageTwoInputFilter::NextAvailableTextureIndex(void) 
    {
        return _hasSetFirstTexture?1:0;
    }

	void GPUImageTwoInputFilter::SetInputRotation(GPUImageRotationMode & inputRotation, int index) 
	{
		if (index == 0) {
			_inputRotation = inputRotation;
		} else {
			_inputRotation2 = inputRotation;
		}
	}

	void GPUImageTwoInputFilter::SetInputSize(Size size, int index)
	{
		if (index == 0) {
			GPUImageFilter::SetInputSize(size, index);
			if (size == kSizeZero) {
				_hasSetFirstTexture = false;
			}
		}
		//LOGD("GPUImageTwoInputFilter set input size:%d x %d, %d", textureSize.width, textureSize.height, index);
	}

	void GPUImageTwoInputFilter::SetInputFramebuffer(Ptr<GPUImageFramebuffer> & frameBuffer, int index) 
	{
		if (index == 0) {
			_firstInputFramebuffer = frameBuffer;
			_hasSetFirstTexture = true;
		} else {
			_secondInputFramebuffer = frameBuffer;	
		}
		//LOGD("GPUImageTwoInputFilter set input framebuffer: 0x%08x, %d",
		 //index?(int)_secondInputFramebuffer.Ptr():(int)_firstInputFramebuffer.Ptr(), index);
	}

	void GPUImageTwoInputFilter::NewFrameReady(Time frameTime, int index)
	{
		if (_hasReceivedFirstFrame && _hasReceivedSecondFrame)
		{
			return;
		}

		if (index == 0)
		{
			_hasReceivedFirstFrame = true;
			_firstFrameTime = frameTime;
			if(_secondFrameCheckDisabled){
				_hasReceivedSecondFrame = true;
			}
			//TODO:
		}
		else
		{
			_hasReceivedSecondFrame = true;
			_secondFrameTime = frameTime;
			if (_firstFrameCheckDisabled){
				_hasReceivedFirstFrame = true;
			}
			//TODO:
		}

		if (_hasReceivedFirstFrame && _hasReceivedSecondFrame)
		{
			Time passOnFrameTime = (!GLTIME_IS_INDEFINITE(_firstFrameTime))?_firstFrameTime:_secondFrameTime;
			GPUImageFilter::NewFrameReady(passOnFrameTime, 0);
			_hasReceivedFirstFrame = false;
			_hasReceivedSecondFrame = false;
		}
	}

	void GPUImageTwoInputFilter::RenderToTexture(const GLfloat *vertices, const GLfloat *texCoords)
	{
		_filterProgram->Use();
		_outputFramebuffer = FetchFramebuffer(GetSizeOfFBO());
		_outputFramebuffer->Active();

		SetProgramUniforms(0);

		glClearColor(_bgColorRed, _bgColorGreen, _bgColorBlue, _bgColorAlpha);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, _firstInputFramebuffer->GetTexture());
		glUniform1i(_filterInputTextureUniform, 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, _secondInputFramebuffer->GetTexture());
		glUniform1i(_filterInputTextureUniform2, 3);

		glVertexAttribPointer(_filterPositionAttribute, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		glVertexAttribPointer(_filterTextureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, 0, texCoords);
		glVertexAttribPointer(_filterSecondTextureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		_firstInputFramebuffer = NULL;
		_secondInputFramebuffer = NULL;
	}

	Size GPUImageTwoInputFilter::GetSizeOfRotated(Size rotateSize, int index) 
	{
		Size rotatedSize = rotateSize;
		GPUImageRotationMode rotationToCheck;
		if (index == 0) {
			rotationToCheck = _inputRotation;
		} else {
			rotationToCheck = _inputRotation2;
		}

		if (GPUImageRotationSwapsWidthAndHeight(rotationToCheck)) 
		{
			rotatedSize.width = rotateSize.height;
			rotatedSize.height = rotateSize.width;
		}

		return rotatedSize;
	}

	void GPUImageTwoInputFilter::DisableFirstFrameCheck(void)
	{
		_firstFrameCheckDisabled = true;
	}

	void GPUImageTwoInputFilter::DisableSecondFrameCheck(void)
	{
		_secondFrameCheckDisabled = true;
	}

	void GPUImageTwoInputFilter::Reset(void)
	{
		GPUImageFilter::Reset();
		
		_hasSetFirstTexture = false;
		_hasReceivedFirstFrame = false;
		_hasReceivedSecondFrame = false;
		_firstFrameCheckDisabled = false;
		_secondFrameCheckDisabled = false;
	}
}