#include "include/GPUImageGaussianBlurFilter.h"
#include <sstream>

namespace e 
{
    GPUImageGaussianBlurFilter::GPUImageGaussianBlurFilter(void)
    {
		_sigma = 5.0f;
        _blurRadius = 4.0f;
		_isNeedUpdateProgram = false;

		SetTexelSpacingMultiplier(1.0f);
//#ifndef USE_CREATE_FUNCTION
//        Initialize();
//#endif
    }

    GPUImageGaussianBlurFilter::~GPUImageGaussianBlurFilter(void)
    {

    }

    void GPUImageGaussianBlurFilter::SetBlurRadius(float blurRadius)
    {
        if (blurRadius < 1.0f || blurRadius > 10.0f) return;

        float value = ROUND(ROUND(blurRadius / 2.0) * 2.0);
        if (_blurRadius != value)
        {
            _blurRadius = value;
			_isNeedUpdateProgram = true;
        }
    }

    void GPUImageGaussianBlurFilter::SetTexelSpacingMultiplier(float texelSpacingMultiplier)
    {
        _texelSpacingMultiplier = texelSpacingMultiplier;

        SetVerticalTexelSpacing(texelSpacingMultiplier);
        SetHorizontalTexelSpacing(texelSpacingMultiplier);

        SetupFilter(GetSizeOfFBO());
    }

    string GPUImageGaussianBlurFilter::CreateVertexShaderString(int blurRadius, float sigma)
    {
        if (blurRadius < 1)
        {
            return string(kGPUImageVertexShaderString);
        }

        float* weights = new float[blurRadius + 1];
        float sum = 0.0f;
        for (int i = 0; i < blurRadius + 1; i++)
        {
            weights[i] = (1.0 / sqrt(2.0 * M_PI * pow(sigma, 2.0))) * exp(-pow(i, 2.0) / (2.0 * pow(sigma, 2.0)));
            if (i == 0) {
                sum += weights[i];
            } else {
                sum += 2.0 * weights[i];
            }
        }

        for (int i = 0; i < blurRadius + 1; i++)
        {
            weights[i] = weights[i] / sum;
        }

        int n = min(blurRadius / 2 + (blurRadius % 2), 7);
        float* offsets = new float[n];

        for (int i = 0; i < n; i++)
        {
            float w1 = weights[i * 2 + 1];
            float w2 = weights[i * 2 + 2];
            float weight = w1 + w2;
            offsets[i] = (w1 * (i * 2 + 1) + w2 * (i * 2 + 2)) / weight;
        }

		ostringstream os;
		os << "attribute vec4 position;" << endl;
		os << "attribute vec4 inputTextureCoordinate;" << endl;
		os << "uniform float texelWidthOffset;" << endl;
		os << "uniform float texelHeightOffset;" << endl;
		os << "varying vec2 blurCoordinates[" << (1 + n * 2) << "];" << endl;
		os << "void main()" << endl;
		os << "{" << endl;
		os << "    gl_Position = position;" << endl;
		os << "    highp vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);" << endl;
		os << "    blurCoordinates[0] = inputTextureCoordinate.xy;" << endl;

        for (int i = 0; i < n; i++)
        {
			os << "blurCoordinates["<<(i*2+1) << "] = inputTextureCoordinate.xy + singleStepOffset * " << offsets[i] << ";" << endl;
			os << "blurCoordinates["<<(i*2+2) << "] = inputTextureCoordinate.xy - singleStepOffset * " << offsets[i] << ";" << endl;
        }

		os << "}" << endl;
        delete[] offsets;
        delete[] weights;
        return os.str();
    }

    string GPUImageGaussianBlurFilter::CreateFragmentShaderString(int blurRadius, float sigma)
    {
		if (blurRadius < 1)
		{
			return string(kGPUImagePassthroughFragmentShaderString);
		}

		int radius = (int)(blurRadius + 0.5f);
		float* weights = new float[radius + 1];
		float sum = 0.0f;
		for (int i = 0; i < blurRadius + 1; i++)
		{
			weights[i] = (1.0 / sqrt(2.0 * M_PI * pow(sigma, 2.0))) * exp(-pow(i, 2.0) / (2.0 * pow(sigma, 2.0)));
			if (i == 0) {
				sum += weights[i];
			} else {
				sum += 2.0 * weights[i];
			}
		}

		for (int i = 0; i < blurRadius + 1; i++)
		{
			weights[i] = weights[i] / sum;
		}

		int n = min((radius/2) + (radius % 2), 7);
		int m = (radius / 2) + (radius % 2);

		ostringstream os;
		os << "uniform sampler2D inputImageTexture;" << endl;
		os << "varying highp float texelWidthOffset;" << endl;
		os << "varying highp float texelHeightOffset;" << endl;
		os << "varying highp vec2 blurCoordinates[" << (1 + (n * 2)) << "];" << endl;
		os << "void main()" << endl;
		os << "{" << endl;
		os << "    lowp vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);" << endl;
		os << "    sum += texture2D(inputImageTexture, blurCoordinates[0]) * " << weights[0] << ";" << endl;

		for (int i = 0; i < n; i++)
		{
			float w1 = weights[i * 2 + 1];
			float w2 = weights[i * 2 + 2];
			float w = w1 + w2;
			os << "    sum += texture2D(inputImageTexture, blurCoordinates[" << (i*2)+1 << "]) * " << w << ";" << endl;
			os << "    sum += texture2D(inputImageTexture, blurCoordinates[" << (i*2)+2 << "]) * " << w << ";" << endl;
		}

		if (m > n)
		{
			os << "highp vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);" << endl;

			for (int i = n; i < m; i++)
			{
				float w1 = weights[i * 2 + 1];
				float w2 = weights[i * 2 + 2];
				float w = w1 + w2;
				float offset = (w1 * (i * 2 + 1) + w2 * (i * 2 + 2)) / w;
				os << "    sum += texture2D(inputImageTexture, blurCoordinates[0] + singleStepOffset * " << offset << ") * " << w << ";" << endl;
				os << "    sum += texture2D(inputImageTexture, blurCoordinates[0] - singleStepOffset * " << offset << ") * " << w << ";" << endl;
			}
		}

		os << "    gl_FragColor = sum;" << endl;
		os << "}" << endl;

		delete[] weights;
		return os.str();
    }

    bool GPUImageGaussianBlurFilter::Initialize(void)
    {
        string vShaderString = CreateVertexShaderString(_blurRadius, _sigma);
        string fShaderString = CreateFragmentShaderString(_blurRadius, _sigma);

        return SwitchProgram(vShaderString.c_str(), fShaderString.c_str());
    }

    bool GPUImageGaussianBlurFilter::SwitchProgram(const char* vShaderString, const char* fShaderString)
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

        glEnableVertexAttribArray(_filterPositionAttribute2);
        glEnableVertexAttribArray(_filterTextureCoordinateAttribute2);
        glFinish();

//        LOGV("GPUImageGaussianBlurFilter initialize ok: %d,%d,%d,%d"
//             , _filterPositionAttribute2
//             , _filterTextureCoordinateAttribute2
//             , _filterInputTextureUniform2
//             , _filterInputTexture2Uniform2);
        return true;
    }

    void GPUImageGaussianBlurFilter::RenderToTexture(const GLfloat* vertices, const GLfloat* texCoords)
    {
		if (_isNeedUpdateProgram)
		{
			string vShaderString = CreateVertexShaderString(_blurRadius, _sigma);
			string fShaderString = CreateFragmentShaderString(_blurRadius, _sigma);

			if (!SwitchProgram(vShaderString.c_str(), fShaderString.c_str()))
			{
				LOGE("GPUImageGaussianBlurFilter SwitchProgram failed!");
			}

			_isNeedUpdateProgram = false;
			LOGD("GPUImageGaussianBlurFilter SwitchProgram ok");
		}

        GPUImageTwoPassTextureSamplingFilter::RenderToTexture(vertices, texCoords);
    }
}