//
// Created by yongfali on 2016/4/9.
//

#include "include/GPUImageCameraFilter.h"
#include "include/GPUImageBase.h"
#include "Bitmap.h"

namespace e 
{
	// // BT.601, which is the standard for SDTV.
	// const GLfloat kColorConversion601Default[9] = {
	//     1.164,  1.164, 1.164,
	//     0.0,   -0.392, 2.017,
	//     1.596, -0.813, 0.0
	// };

	// BT.601 full range (ref: http://www.equasys.de/colorconversion.html)
	const GLfloat kColorConversion601FullRangeDefault[9] = {
	    1.0,    1.0,    1.0,
	    0.0,    -0.343, 1.765,
	    1.4,    -0.711, 0.0
	};

	// // BT.709, which is the standard for HDTV.
	// const GLfloat kColorConversion709Default[9] = {
	//     1.164,  1.164, 1.164,
	//     0.0,   -0.213, 2.112,
	//     1.793, -0.533,   0.0
	// };	

	shader_t kGPUImageCameraVertexShaderString = SHADER_STRING
	(
		attribute vec4 position;
		attribute vec4 inputTextureCoordinate;

		varying vec2 textureCoordinate;

		void main()
      	{
			gl_Position = position;
			textureCoordinate = inputTextureCoordinate.xy;
		}
	);

	shader_t kGPUImageCameraFragmentShaderString = SHADER_STRING
	(
		uniform sampler2D luminanceTexture;//luminance texture
		uniform sampler2D chrominanceTexture;//chrominance texture
		uniform mediump mat3 colorConversionMatrix;
		varying highp vec2 textureCoordinate;

		void main() 
      	{
			mediump vec3 yuv;
			yuv.x = texture2D(luminanceTexture, textureCoordinate).r;
			yuv.zy = texture2D(chrominanceTexture, textureCoordinate).ra - vec2(0.5, 0.5);
			mediump vec3 rgb = colorConversionMatrix * yuv;
			gl_FragColor = vec4(rgb, 1.0);
		}
	);

	inline GLfloat flip(GLfloat i)
	{
		return 1.0f - i;
	}

	GPUImageCameraFilter::GPUImageCameraFilter(void)
	{
		_outputSize = Size(640, 360);
		_luminanceTexture = 0;
		_chrominanceTexture = 0;
#ifndef TEXTURE_CACHE_ENABLE
		_eglContext = 0;
#else
        _textureCache = GPUImageTextureCache::Singleton();
#endif
		_filterPositionAttribute = 0;
		_filterTextureCoordinateAttribute = 0;
		_luminanceTextureUniform = -1;
		_chrominanceTextureUniform = -1;
		_conversionMatrixUniform = -1;
		_inputRotation = kGPUImageNoRotation;
		memcpy(_conversionMatrix, kColorConversion601FullRangeDefault, sizeof(_conversionMatrix));
		_frameBufferCache = GPUImageFramebufferCache::Singleton();

		_isNeedCrop = false;
		
#ifndef USE_CREATE_FUNCTION
		Initialize();
#endif
	}

	GPUImageCameraFilter::~GPUImageCameraFilter(void)
	{
#ifndef TEXTURE_CACHE_ENABLE
		if (_eglContext != eglGetCurrentContext())
		{
			return;
		}

        if (_luminanceTexture)
		{
			glDeleteTextures(GL_TEXTURE_2D, &_luminanceTexture);
			_luminanceTexture = 0;
		}

		if (_chrominanceTexture)
		{
			glDeleteTextures(GL_TEXTURE_2D, &_chrominanceTexture);
			_chrominanceTexture = 0;
		}
#else
        _luminanceTexture = 0;
        _chrominanceTexture = 0;
        _textureCache = 0;
        _frameBufferCache = 0;
#endif
	}

	bool GPUImageCameraFilter::Initialize(void)
	{
		return Initialize(kGPUImageCameraVertexShaderString, kGPUImageCameraFragmentShaderString);
	}

	bool GPUImageCameraFilter::Initialize(const char* vShaderString, const char* fShaderString)
	{
	    _filterProgram = new GLProgram(vShaderString, fShaderString);
	    CheckPointer(_filterProgram, false);

	    if (!_filterProgram->IsValid())
	    {
	    	InitializeAttributes();

	    	if (!_filterProgram->Link())
	    	{
	    		std::string log = _filterProgram->GetShaderLog(GLProgram::LOG_TYPE_PROG);
				LOGE("opengl shader program link failed:prog %s\n", log.c_str());
				log = _filterProgram->GetShaderLog(GLProgram::LOG_TYPE_VERT);
				LOGE("opengl shader program link failed:vert %s\n", log.c_str());
				log = _filterProgram->GetShaderLog(GLProgram::LOG_TYPE_FRAG);
				LOGE("opengl shader program link failed:frag %s\n", log.c_str());
				_filterProgram = NULL;
				return false;
	    	}
	    }
     
     	_filterProgram->Use();
        _filterPositionAttribute = _filterProgram->GetAttributeIndex("position");
        _filterTextureCoordinateAttribute = _filterProgram->GetAttributeIndex("inputTextureCoordinate");
        _luminanceTextureUniform = _filterProgram->GetUniformLocation("luminanceTexture");
        _chrominanceTextureUniform = _filterProgram->GetUniformLocation("chrominanceTexture");
        _conversionMatrixUniform = _filterProgram->GetUniformLocation("colorConversionMatrix");

        glEnableVertexAttribArray(_filterPositionAttribute);
        glEnableVertexAttribArray(_filterTextureCoordinateAttribute);

        LOGV("GPUImageCameraFilter initialize ok: %d,%d,%d,%d,%d"
        	, _filterPositionAttribute
            , _filterTextureCoordinateAttribute
            , _luminanceTextureUniform
            , _chrominanceTextureUniform
            , _conversionMatrixUniform);

        return true;
	}

	void GPUImageCameraFilter::InitializeAttributes(void) 
	{
	    _filterProgram->AddAttribute("position");
	    _filterProgram->AddAttribute("inputTextureCoordinate");
	}
	
	void GPUImageCameraFilter::SetOutputSize(Size size)
	{
		_outputSize = size;
		if (_outputSize.width * _inputSize.height != _outputSize.height * _inputSize.width)
		{
			_isNeedCrop = true;
		}
		else
		{
			_isNeedCrop = false;
		}
        LOGD("GPUImageCameraFilter SetOutputSize, %d x %d,crop=%d", size.width, size.height, _isNeedCrop?1:0);
	}

	void GPUImageCameraFilter::SetInputSize(Size size, int index)
	{
		if (_inputSize !=  size)
		{
			_inputSize = size;
			if (_outputSize.width * _inputSize.height != _outputSize.height * _inputSize.width)
			{
				_isNeedCrop = true;
			}
			else
			{
				_isNeedCrop = false;
			}

			CreateTextures();
			LOGD("GPUImageCameraFilter SetInputSize, %d x %d,crop=%d", size.width, size.height, _isNeedCrop?1:0);
		}
	}

	void GPUImageCameraFilter::SetInputRotationMode(GPUImageRotationMode & rotation, int index)
	{
		_inputRotation = rotation;
		LOGD("GPUImageCameraFilter SetInputRotationMode, angle = %d, flipX=%d, flipY=%d",
			 rotation.angle, rotation.flipX, rotation.flipY);
	}

	Size GPUImageCameraFilter::GetSizeOfFBO(void)
	{
#if 0
		Size filterSize = _inputSize;
		if (GPUImageRotationSwapsWidthAndHeight(_inputRotation))
		{
			filterSize.width = _inputSize.height;
			filterSize.height = _inputSize.width;
		}
#else	//do scale
		Size filterSize = _outputSize;
		if (GPUImageRotationSwapsWidthAndHeight(_inputRotation))
		{
			filterSize.width = _outputSize.height;
			filterSize.height = _outputSize.width;
		}
#endif
		return filterSize;
	}

	Size GPUImageCameraFilter::GetSizeOfRotated(Size size)
	{
		Size result = size;
		if (GPUImageRotationSwapsWidthAndHeight(_inputRotation))
		{
			result.width = size.height;
			result.height = size.width;
		}
		return result;
	}

	bool GPUImageCameraFilter::CreateTextures(void)
	{
#ifndef TEXTURE_CACHE_ENABLE
		if (_luminanceTexture)
		{
			glDeleteTextures(GL_TEXTURE_2D, &_luminanceTexture);
			_luminanceTexture = 0;
		}

		if (_chrominanceTexture)
		{
			glDeleteTextures(GL_TEXTURE_2D, &_chrominanceTexture);
			_chrominanceTexture = 0;
		}

		_eglContext = eglGetCurrentContext();

		glActiveTexture(GL_TEXTURE4);
		glGenTextures(1, &_luminanceTexture);
		glBindTexture(GL_TEXTURE_2D, _luminanceTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, _inputSize.width, _inputSize.height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);

		glActiveTexture(GL_TEXTURE5);
		glGenTextures(1, &_chrominanceTexture);
		glBindTexture(GL_TEXTURE_2D, _chrominanceTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, _inputSize.width/2, _inputSize.height/2, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, NULL);
#else
        if (_luminanceTexture == 0)
        {
            _luminanceTexture = _textureCache->Fetch(_inputSize
                , kGPUImageTextureOptionsLuminance
                , 4);
        }

        if (_chrominanceTexture == 0)
        {
            Size size(_inputSize.width/2, _inputSize.height/2);
            _chrominanceTexture = _textureCache->Fetch(size
                , kGPUImageTextureOptionsChrominance
                , 5);
        }
#endif
		return true;
	}

	void GPUImageCameraFilter::UpdateTextures(unsigned char* data, int width, int height, int format)
	{
		unsigned char* plane[2] = {data, data + width * height};
		//Y-plane
		glActiveTexture(GL_TEXTURE4);
#ifndef TEXTURE_CACHE_ENABLE
		glBindTexture(GL_TEXTURE_2D, _luminanceTexture);
#else
        glBindTexture(GL_TEXTURE_2D, _luminanceTexture->GetTexture());
#endif
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, _inputSize.width, _inputSize.height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, plane[0]);	
		//UV-plane
		glActiveTexture(GL_TEXTURE5);
#ifndef TEXTURE_CACHE_ENABLE
		glBindTexture(GL_TEXTURE_2D, _chrominanceTexture);
#else
        glBindTexture(GL_TEXTURE_2D, _chrominanceTexture->GetTexture());
#endif
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, _inputSize.width/2, _inputSize.height/2, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, plane[1]);
	}

	void GPUImageCameraFilter::ConvertYUV2RGBAOutput(void)
	{
		//color conversion 不做rotation动作
		static const GLfloat squareVertices[] = {
			-1.0f, -1.0f,//A
			 1.0f, -1.0f,//B
			-1.0f,  1.0f,//C
			 1.0f,  1.0f //D
		};
		
		_filterProgram->Use();
		_outputFramebuffer = GPUImageFramebufferCache::Singleton()->FetchFramebuffer(GetSizeOfFBO(), false);
		_outputFramebuffer->Active();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE4);
#ifndef TEXTURE_CACHE_ENABLE
		glBindTexture(GL_TEXTURE_2D, _luminanceTexture);
#else
        glBindTexture(GL_TEXTURE_2D, _luminanceTexture->GetTexture());
#endif
		glUniform1i(_luminanceTextureUniform, 4);

		glActiveTexture(GL_TEXTURE5);
#ifndef TEXTURE_CACHE_ENABLE
		glBindTexture(GL_TEXTURE_2D, _chrominanceTexture);
#else
        glBindTexture(GL_TEXTURE_2D, _chrominanceTexture->GetTexture());
#endif
		glUniform1i(_chrominanceTextureUniform, 5);

		glUniformMatrix3fv(_conversionMatrixUniform, 1, GL_FALSE, _conversionMatrix);

		glVertexAttribPointer(_filterPositionAttribute, 2, GL_FLOAT, GL_FALSE, 0, squareVertices);
		glVertexAttribPointer(_filterTextureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, 0, _isNeedCrop?GetCropTexCoords(_inputRotation):GetTexCoords(_inputRotation));

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	void GPUImageCameraFilter::RenderToTexture(void* data, int width, int height, int format)
	{
		SetInputSize(Size(width, height), 0);

		UpdateTextures((unsigned char*)data, width, height, format);

		ConvertYUV2RGBAOutput();

		CallNextTargets(Time());
	}

	void GPUImageCameraFilter::CallNextTargets(Time time)
	{
		TargetList::iterator it = _targets->begin();
		for(; it != _targets->end(); it++)
		{
			GPUImageInput* target = it->target;
			if (target->IsEnable())
			{
				target->SetInputSize(GetSizeOfFBO(), it->index);
				target->SetInputRotationMode(kGPUImageNoRotation, it->index);
				target->SetInputFramebuffer(GetOutputFramebuffer(), it->index);
			}
		}

		//release framebuffer
		_outputFramebuffer = NULL;
		
		for (it = _targets->begin(); it != _targets->end(); it++)
		{			
			GPUImageInput* target = it->target;
			if (target->IsEnable())
			{
				target->NewFrameReady(time, it->index);
			}
		}
	}

	GLfloat* GPUImageCameraFilter::GetTexCoords(const GPUImageRotationMode & rotateMode)
	{
		static float kTexCoords[4][8] = {
            {// no rotate
                0.0f, 1.0f,
                1.0f, 1.0f,
                0.0f, 0.0f,
                1.0f, 0.0f
            },
            {// rotate 90        
                1.0f, 1.0f,
                1.0f, 0.0f,
                0.0f, 1.0f,
                0.0f, 0.0f
            },
            {// rotate 180        
                1.0f, 0.0f,
                0.0f, 0.0f,
                1.0f, 1.0f,
                0.0f, 1.0f
            },
            {// rotate 270        
                0.0f, 0.0f,
                0.0f, 1.0f,
                1.0f, 0.0f,
                1.0f, 1.0f
            }
        };

        static GLfloat texCoords[8] = {0};
		if (rotateMode.angle == 0){
			memcpy(texCoords, kTexCoords[0], sizeof(GLfloat)*8);
		}else if (rotateMode.angle == 90){
			memcpy(texCoords, kTexCoords[1], sizeof(GLfloat)*8);
		}else if (rotateMode.angle == 180){
			memcpy(texCoords, kTexCoords[2], sizeof(GLfloat)*8);
		}else if (rotateMode.angle == 270){
			memcpy(texCoords, kTexCoords[3], sizeof(GLfloat)*8);
		}

		if (rotateMode.flipX) {
			texCoords[0] = flip(texCoords[0]);
			texCoords[2] = flip(texCoords[2]);
			texCoords[4] = flip(texCoords[4]);
			texCoords[6] = flip(texCoords[6]);
		}

		if (rotateMode.flipY) {
			texCoords[1] = flip(texCoords[1]);
			texCoords[3] = flip(texCoords[3]);
			texCoords[5] = flip(texCoords[5]);
			texCoords[7] = flip(texCoords[7]);
		}

		return texCoords;
	}

	GLfloat* GPUImageCameraFilter::GetCropTexCoords(const GPUImageRotationMode & rotateMode)
	{
		Size s0 = _inputSize;
		Size s1 = _outputSize;
		float k0 = (float)s0.width / (float)s0.height;
		float k1 = (float)s1.width / (float)s1.height;
		float x0 = 0.0f, x1 = 1.0f, y0 = 0.0f, y1 = 1.0f;

		if (k0 > k1)//裁宽
		{
			int w = s0.height * s1.width / s1.height;
			x0 = (1.0f - w / (float)s0.width) / 2.0f;
			x1 = 1.0f - x0;
			y0 = 0.0f;
			y1 = 1.0f;
		}
		else if (k0 < k1)//裁高
		{
			int h = s0.width * s1.height / s1.width;
			x0 = 0.0f;
			x1 = 1.0f;
			y0 = (1.0f - h / (float)s0.height) / 2.0f;
			y1 = 1.0f - y0;
		}

		static float kTexCoords[4][8] = {
            {// no rotate
                x0, y0,	//0.0f, 1.0f,
                x1, y0,	//1.0f, 1.0f,
                x0, y1,	//0.0f, 0.0f,
                x1, y1,	//1.0f, 0.0f
            },
            {// rotate 90
                x1, y1, //1.0f, 1.0f,
                x1, y0, //1.0f, 0.0f,
                x0, y1, //0.0f, 1.0f,
                x0, y0, //0.0f, 0.0f
            },
            {// rotate 180        
                x1, y0,//1.0f, 0.0f,
                x0, y0,//0.0f, 0.0f,
                x1, y1,//1.0f, 1.0f,
                x0, y1,//0.0f, 1.0f
            },
            {// rotate 270        
                x0, y0,//0.0f, 0.0f,
                x0, y1,//0.0f, 1.0f,
                x1, y0,//1.0f, 0.0f,
                x1, y1,//1.0f, 1.0f
            }
        };

        static GLfloat texCoords[8] = {0};
		if (rotateMode.angle == 0){
			memcpy(texCoords, kTexCoords[0], sizeof(GLfloat)*8);
		}else if (rotateMode.angle == 90){
			memcpy(texCoords, kTexCoords[1], sizeof(GLfloat)*8);
		}else if (rotateMode.angle == 180){
			memcpy(texCoords, kTexCoords[2], sizeof(GLfloat)*8);
		}else if (rotateMode.angle == 270){
			memcpy(texCoords, kTexCoords[3], sizeof(GLfloat)*8);
		}

		if (rotateMode.flipX) {
			texCoords[0] = flip(texCoords[0]);
			texCoords[2] = flip(texCoords[2]);
			texCoords[4] = flip(texCoords[4]);
			texCoords[6] = flip(texCoords[6]);
		}

		if (rotateMode.flipY) {
			texCoords[1] = flip(texCoords[1]);
			texCoords[3] = flip(texCoords[3]);
			texCoords[5] = flip(texCoords[5]);
			texCoords[7] = flip(texCoords[7]);
		}

		return texCoords;
	}
}