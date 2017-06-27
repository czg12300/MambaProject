//
// Created by yongfali on 2017/3/17.
//

#include "include/GPUImageTitleFilter.h"

namespace e
{
    shader_t kGPUImageTitleVertexShaderString = SHADER_STRING
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

	shader_t kGPUImageTitleFragmentShaderString = SHADER_STRING
	(
		varying highp vec2 textureCoordinate;
        varying highp vec2 textureCoordinate2;

		uniform sampler2D inputImageTexture; //rgba sampler texture
		uniform sampler2D titleImageTexture; //curve sampler texture

		void main()
		{
			lowp vec4 color = texture2D(inputImageTexture, textureCoordinate);
            lowp vec4 title = texture2D(titleImageTexture, textureCoordinate2);
			//gl_FragColor = mix(color, title, title.a);
			gl_FragColor = vec4(color.rgb * (1.0 - title.a) + title.rgb, color.a);
		}
	);

    GPUImageTitleFilter::GPUImageTitleFilter()
    { 
        _titleTexture = 0;
        _titleTextureUniform = 0;
#ifndef TEXTURE_CACHE_ENABLE
        _eglContext = 0;
#else
        _textureCache = GPUImageTextureCache::Singleton();
#endif
        _titleImage = 0;
        _titleSize = 0;
        _titleWidth = 0;
        _titleHeight = 0;
        _titleStride = 0;
        _titleBitCount = 0;
        _isNeedUpdate = true;

#ifndef USE_CREATE_FUNCTION
		Initialize();
#endif        
    }    

    GPUImageTitleFilter::~GPUImageTitleFilter()
    {
        if (_titleImage != 0){
            free(_titleImage);
        }
#ifndef TEXTURE_CACHE_ENABLE
        if(_eglContext != eglGetCurrentContext())
        {
            return;
        }

        if (_titleTexture != 0){
            glDeleteTextures(1, &_titleTexture);
            _titleTexture = 0;
        }
#else
        _titleTexture = 0;
        _textureCache = 0;        
#endif
    }

    bool GPUImageTitleFilter::Initialize()
    {
		if (!GPUImageFilter::Initialize(kGPUImageTitleVertexShaderString, kGPUImageTitleFragmentShaderString))
		{
			LOGE("GPUImageTitleFilter Initialize failed");
			return false;
		}

        _filterTextureCoordinateAttribute2 = _filterProgram->GetAttributeIndex("inputTextureCoordinate2");
        glEnableVertexAttribArray(_filterTextureCoordinateAttribute2);

		_titleTextureUniform = _filterProgram->GetUniformLocation("titleImageTexture");
		LOGV("GPUImageTitleFilter initialize ok : %d", _titleTextureUniform);
		return true;
    }

    void GPUImageTitleFilter::InitializeAttributes(void)
    {
        GPUImageFilter::InitializeAttributes();

        _filterProgram->AddAttribute("inputTextureCoordinate2");
    }

    void GPUImageTitleFilter::SetTitleImage(void* data, int width, int height, int stride, int bitCount)
    {
        if (!_titleImage || _titleWidth!=width || _titleHeight!=height)
        {
            AllocateBuffer(width, height);
        }
        
        if (_titleImage != 0)
        {
            const int size = width * height * 4;
            if (data != 0){
                memcpy(_titleImage, data, size);
            }else{
                memset(_titleImage, 0, size);
            }

            _isNeedUpdate = true;
        }
    }

    void GPUImageTitleFilter::AllocateBuffer(int width, int height)
    {
        if (width == 0) width = 360;
        if (height == 0) height = 640;

        const int size = width * height * 4;
        _titleImage = (char*)realloc(_titleImage, size);
        assert(_titleImage);

        if (_titleImage){
            memset(_titleImage, 0, size);
        }
        
        _titleSize = size;
        _titleWidth = width;
        _titleHeight = height;
        _titleStride = width * 4;
        _titleBitCount = 32;

        LOGV("GPUImageTitleFilter::AllocateBuffer %d x %d", width, height);
    }

    void GPUImageTitleFilter::UpdateTitleTexture()
    {
        if (!_titleImage)
        {
            Size size = GetSizeOfFBO();
            AllocateBuffer(size.width, size.height);
        }
#ifndef TEXTURE_CACHE_ENABLE
        if (_titleTexture == 0)
        {
            glGenTextures(1, &_titleTexture);
            _eglContext = eglGetCurrentContext();
        }
        glBindTexture(GL_TEXTURE_2D, _titleTexture);
#else
		if (_titleTexture == 0)
        {
			Size texSize = Size(_titleWidth, _titleHeight);
            _titleTexture = _textureCache->Fetch(texSize
                , kGPUImageTextureOptionsDefault
                , 3);
        }
		glBindTexture(GL_TEXTURE_2D, _titleTexture->GetTexture());
#endif
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _titleWidth, _titleHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, _titleImage);	
    }

    void GPUImageTitleFilter::MakeTestImage()
    {
        const int stride = 360*4;
        const int size = stride * 640;
        char* buf = new char[size];
        memset(buf, 0, size);

        for (int y=0; y<100; y++)
        {
            char* p = buf + y * 360 * 4;
            for (int x=0; x<200; x++)
            {
                p[0] = 0xff;
                p[1] = 0xff;
                p[2] = 0x00;
                p[3] = 0x80;
                p+=4;
            }
        }

        for (int y=100; y<200; y++)
        {
            char* p = buf + y * 360 * 4;
            for (int x=0; x<360; x++)
            {
                p[0] = 0xff;
                p[1] = 0x00;
                p[2] = 0x00;
                p[3] = 0x80;
                p+=4;
            }
        }

        SetTitleImage(buf, 360, 640, stride, 32);
        delete[] buf;
    }

    void GPUImageTitleFilter::RenderToTexture(const float* vertices, const float* texCoords)
    {
        static float texCoords2[] = {
            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f
        };

		_filterProgram->Use();
		_outputFramebuffer = FetchFramebuffer(GetSizeOfFBO());
		_outputFramebuffer->Active();

		if (_isNeedUpdate)
		{
			UpdateTitleTexture();
		}

		SetProgramUniforms(0);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, _firstInputFramebuffer->GetTexture());
		glUniform1i(_filterInputTextureUniform, 2);

		glActiveTexture(GL_TEXTURE3);
#ifndef TEXTURE_CACHE_ENABLE
		glBindTexture(GL_TEXTURE_2D, _titleTexture);
#else
        glBindTexture(GL_TEXTURE_2D, _titleTexture->GetTexture());        
#endif
		glUniform1i(_titleTextureUniform, 3);

		glVertexAttribPointer(_filterPositionAttribute, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		glVertexAttribPointer(_filterTextureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, 0, texCoords);
        glVertexAttribPointer(_filterTextureCoordinateAttribute2, 2, GL_FLOAT, GL_FALSE, 0, texCoords2);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		_firstInputFramebuffer = NULL;
    }

}
