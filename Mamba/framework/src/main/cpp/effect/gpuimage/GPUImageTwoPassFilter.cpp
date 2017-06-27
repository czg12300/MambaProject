#include "include/GPUImageTwoPassFilter.h"
#include "include/GPUImageBase.h"

namespace e 
{
	GPUImageTwoPassFilter::GPUImageTwoPassFilter(void)
	{
		_filterPositionAttribute2 = 0;
		_filterTextureCoordinateAttribute2 = 0;
		_filterInputTextureUniform2 = -1;
		_filterInputTexture2Uniform2 = -1;
	}
	
	GPUImageTwoPassFilter::GPUImageTwoPassFilter(const char* firstFragmentShaderString, const char* secondFragmentShaderString)
	{
		_filterPositionAttribute2 = 0;
		_filterTextureCoordinateAttribute2 = 0;
		_filterInputTextureUniform2 = -1;
		_filterInputTexture2Uniform2 = -1;
		
		Initialize(firstFragmentShaderString, secondFragmentShaderString);
	}

	GPUImageTwoPassFilter::GPUImageTwoPassFilter(const char* firstVertexShaderString
		, const char* firstFragmentShaderString
		, const char* secondVertexShaderString
		, const char* secondFragmentShaderString)
	{
		_filterPositionAttribute2 = 0;
		_filterTextureCoordinateAttribute2 = 0;
		_filterInputTextureUniform2 = -1;
		_filterInputTexture2Uniform2 = -1;
		
		Initialize(firstVertexShaderString, firstFragmentShaderString, secondVertexShaderString, secondFragmentShaderString);
	}

	GPUImageTwoPassFilter::~GPUImageTwoPassFilter(void)
	{
		_filterProgram2 = 0;
		_outputFramebuffer2 = 0;
	}

	bool GPUImageTwoPassFilter::Initialize(void)
	{
		return false;
	}

	bool GPUImageTwoPassFilter::Initialize(const char* firstFragmentShaderString
		, const char* secondFragmentShaderString)
	{
		return Initialize(kGPUImageVertexShaderString
			, firstFragmentShaderString
			, kGPUImageVertexShaderString
			, secondFragmentShaderString);
	}

	bool GPUImageTwoPassFilter::Initialize(const char* firstVertexShaderString
		, const char* firstFragmentShaderString
		, const char* secondVertexShaderString
		, const char* secondFragmentShaderString)
	{
	    if (!GPUImageFilter::Initialize(firstVertexShaderString, firstFragmentShaderString))
	    {
			LOGE("GPUImageTwoPassFilter initialize failed");
	        return false;
	    }

	    _filterProgram2 = new GLProgram(secondVertexShaderString, secondFragmentShaderString);
	    CheckPointer(_filterProgram2, false);

	    if (!_filterProgram2->IsValid())
	    {
	    	InitializeSecondaryAttributes();

	    	if (!_filterProgram2->Link())
	    	{
				std::string log = _filterProgram2->GetShaderLog(GLProgram::LOG_TYPE_PROG);
				LOGE("opengl shader program link failed:prog %s\n", log.c_str());
				log = _filterProgram2->GetShaderLog(GLProgram::LOG_TYPE_VERT);
				LOGE("opengl shader program link failed:vert %s\n", log.c_str());
				log = _filterProgram2->GetShaderLog(GLProgram::LOG_TYPE_FRAG);
				LOGE("opengl shader program link failed:frag %s\n", log.c_str());
				_filterProgram2 = NULL;
				return false;	    		
	    	}
	    }

		_filterProgram2->Use();
	    _filterPositionAttribute2 = _filterProgram2->GetAttributeIndex("position");
	    _filterTextureCoordinateAttribute2 = _filterProgram2->GetAttributeIndex("inputTextureCoordinate");
	    _filterInputTextureUniform2 = _filterProgram2->GetUniformLocation("inputImageTexture");
	    _filterInputTexture2Uniform2 = _filterProgram2->GetUniformLocation("inputImageTexture2");

	    glEnableVertexAttribArray(_filterPositionAttribute2);
	    glEnableVertexAttribArray(_filterTextureCoordinateAttribute2);
		
	    // LOGD("GPUImageTwoPassFilter Initialize ok: %d,%d,%d,%d"
	    // 	, _filterPositionAttribute2
	    // 	, _filterTextureCoordinateAttribute2
	    // 	, _filterInputTextureUniform2
	    // 	, _filterInputTexture2Uniform2);
        return true;
	}

	void GPUImageTwoPassFilter::InitializeSecondaryAttributes(void) 
	{
		_filterProgram2->AddAttribute("position");
		_filterProgram2->AddAttribute("inputTextureCoordinate");
	}

	void GPUImageTwoPassFilter::SetProgramUniforms(const int index) 
	{
		if (index == 0){
			GPUImageFilter::SetProgramUniforms(index);
		}else{
			//TODO:
		}
	}

	void GPUImageTwoPassFilter::RenderToTexture(const GLfloat *vertices, const GLfloat *texCoords)
	{
		//first rendering
		_filterProgram->Use();
		_outputFramebuffer = FetchFramebuffer(GetSizeOfFBO());
		_outputFramebuffer->Active();
		
		SetProgramUniforms(0);

		glClearColor(_bgColorRed, _bgColorGreen, _bgColorBlue, _bgColorAlpha);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, _firstInputFramebuffer->GetTexture());
		glUniform1i(_filterInputTextureUniform, 2);

		glVertexAttribPointer(_filterPositionAttribute, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		glVertexAttribPointer(_filterTextureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, 0,texCoords);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		_firstInputFramebuffer = NULL;

		//second rendering
		_filterProgram2->Use();
		_outputFramebuffer2 = FetchFramebuffer(GetSizeOfFBO());
		_outputFramebuffer2->Active();

		SetProgramUniforms(1);
		
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, _outputFramebuffer->GetTexture());
		glVertexAttribPointer(_filterTextureCoordinateAttribute2, 2, GL_FLOAT, GL_FALSE, 0, texCoords);
		
		glUniform1i(_filterInputTextureUniform2, 3);
		glVertexAttribPointer(_filterPositionAttribute2, 2, GL_FLOAT, GL_FALSE, 0, vertices);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		_outputFramebuffer = NULL;
	}

	Ptr<GPUImageFramebuffer> & GPUImageTwoPassFilter::GetOutputFramebuffer(void) 
	{
		return _outputFramebuffer2;
	}

	void GPUImageTwoPassFilter::RemoveOutputFramebuffer(void)
	{
		_outputFramebuffer2 = NULL;
	}
}