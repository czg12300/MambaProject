//
// Created by yongfali on 2016/11/25.
//

#include "include/GPUImagePinkBlurFilter3.h"
#include <sstream>

namespace e
{
	GPUImagePinkBlurFilter3::GPUImagePinkBlurFilter3()
	{
		_filterName = "GPUImagePinkBlurFilter3";
		_texelThresholdUniform = -1;
		_texelThresholdUniform2 = -1;
		_texelThreshold = 0.1f;

#ifndef USE_CREATE_FUNCTION
		Initialize();
#endif
	}

	GPUImagePinkBlurFilter3::~GPUImagePinkBlurFilter3()
	{

	}

	string GPUImagePinkBlurFilter3::CreateVertexShaderString(int blurRadius, float threshold)
	{
		if (blurRadius < 1)
		{
			return string(kGPUImageVertexShaderString);
		}

		ostringstream os;
		const int count = 2 * blurRadius + 1;
		os << "attribute vec4 position;" << endl;
		os << "attribute vec4 inputTextureCoordinate;" << endl;
		os << "uniform float texelWidthOffset;" << endl;
		os << "uniform float texelHeightOffset;" << endl;
		os << "varying vec2 blurCoordinates[" << count << "];" << endl;
		os << "void main()" << endl;
		os << "{" << endl;
		os << "    gl_Position = position;" << endl;
		os << "    vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);" << endl;
		os << "    blurCoordinates[0] = inputTextureCoordinate.xy;" << endl;
		os << "    for (int i = 1; i < " << blurRadius + 1 << "; i++) " << endl;
		os << "    { " << endl;
		os << "		    int m = (i - 1) - " << blurRadius << ";" << endl;
		os << "			highp vec2 offset = singleStepOffset * float(m);" << endl;
		os << "	        blurCoordinates[i] = inputTextureCoordinate.xy + offset;" << endl;
		os << "		    blurCoordinates["<<count << "-i] = inputTextureCoordinate.xy - offset;" << endl;
		os << "    }" << endl;
		os << "}" << endl;
		//LOGD("--------\n%s", os.str().c_str());
		return os.str();
	}

	string GPUImagePinkBlurFilter3::CreateFragmentShaderString(int blurRadius, float threshold)
	{
		if (blurRadius < 1)
		{
			return string(kGPUImagePassthroughFragmentShaderString);
		}

		ostringstream os;
		const int count = 2 * blurRadius + 1;
		os << "precision highp float;" << endl;
		os << "uniform sampler2D inputImageTexture;" << endl;
		os << "uniform float threshold;" << endl;
		os << "varying highp float texelWidthOffset;" << endl;
		os << "varying highp float texelHeightOffset;" << endl;
		os << "varying highp vec2 blurCoordinates[" << count << "];" << endl;
		os << "void main()" << endl;
		os << "{" << endl;
		os << "	   lowp float factor = min(threshold, 1.0);" << endl;
		os << "    lowp float cc = texture2D(inputImageTexture, blurCoordinates[0]).g;" << endl;
		os << "	   lowp float color = cc;" << endl;
		os << "	   lowp float weight = 1.0;" << endl;
		os << "	   for (int i = 1; i < " << count << "; i++)" << endl;
		os << "	   {" << endl;
		os << "    		lowp float sc = texture2D(inputImageTexture, blurCoordinates[i]).g;" << endl;
		os << "		    lowp float tc = clamp(1.0 - abs(distance(sc, cc))/(2.5*factor), 0.0, 1.0);" << endl;
		os << "			color += tc * sc;" << endl;
		os << "			weight += tc;" << endl;
		os << "		}" << endl;
		os << "		color = color / weight;" << endl;
		os << "     gl_FragColor = vec4(color, color, color, 1.0);" << endl;
		os << "}" << endl;

		//LOGD("#########\n%s", os.str().c_str());
		return os.str();
	}

	bool GPUImagePinkBlurFilter3::Initialize()
	{
		//LOGD("GPUImagePinkBlurFilter3::Initialize come in");
        string vShaderString = CreateVertexShaderString(_blurRadius, _sigma);
        string fShaderString = CreateFragmentShaderString(_blurRadius, _sigma);

        return SwitchProgram(vShaderString.c_str(), fShaderString.c_str());
	}

	void GPUImagePinkBlurFilter3::SetThresholdFactor(float threshold)
	{
		_texelThreshold = max(0.0f, min(threshold, 1.0f)) / 10.0f;
	}

	bool GPUImagePinkBlurFilter3::SwitchProgram(const char* vShaderString, const char* fShaderString)
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
        _filterInputTextureUniform = _filterProgram->GetUniformLocation("inputImageTexture");
        _verticalPassTexelWidthOffsetUniform = _filterProgram->GetUniformLocation("texelWidthOffset");
        _verticalPassTexelHeightOffsetUniform = _filterProgram->GetUniformLocation("texelHeightOffset");
		_texelThresholdUniform = _filterProgram->GetUniformLocation("threshold");

        glEnableVertexAttribArray(_filterPositionAttribute);
        glEnableVertexAttribArray(_filterTextureCoordinateAttribute);

        _filterProgram2 = new GLProgram(vShaderString, fShaderString);
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
        _horizontalPassTexelWidthOffsetUniform = _filterProgram2->GetUniformLocation("texelWidthOffset");
        _horizontalPassTexelHeightOffsetUniform = _filterProgram2->GetUniformLocation("texelHeightOffset");
		_texelThresholdUniform2 = _filterProgram2->GetUniformLocation("threshold");

        glEnableVertexAttribArray(_filterPositionAttribute2);
        glEnableVertexAttribArray(_filterTextureCoordinateAttribute2);
        glFinish();

        return true;
	}

	void GPUImagePinkBlurFilter3::SetProgramUniforms(int index)
	{
		GPUImageGaussianBlurFilter::SetProgramUniforms(index);

		if (index == 0){
			glUniform1f(_texelThresholdUniform, _texelThreshold);
		}else{
			glUniform1f(_texelThresholdUniform2, _texelThreshold);
		}
	}
}
