#include "include/GPUImageTwoPassTextureSamplingFilter.h"

namespace e
{
    GPUImageTwoPassTextureSamplingFilter::GPUImageTwoPassTextureSamplingFilter(void)
    {
		_verticalPassTexelHeightOffsetUniform = -1;
		_verticalPassTexelWidthOffsetUniform = -1;
		_horizontalPassTexelHeightOffsetUniform = -1;
		_horizontalPassTexelWidthOffsetUniform = -1;

		_verticalPassTexelHeightOffset = 0.0f;
		_verticalPassTexelWidthOffset = 0.0f;
		_horizontalPassTexelHeightOffset = 0.0f;
		_horizontalPassTexelWidthOffset = 0.0f;

		_verticalTexelSpacing = 4.0f;
		_horizontalTexelSpacing = 4.0f;
    }   
    
    GPUImageTwoPassTextureSamplingFilter::~GPUImageTwoPassTextureSamplingFilter(void)
    {
        
    } 

    bool GPUImageTwoPassTextureSamplingFilter::Initialize(void)
    {
    	return false;
    }
    
	bool GPUImageTwoPassTextureSamplingFilter::Initialize(const char* vShaderString1
		, const char* fShaderString1
		, const char* vShaderString2
		, const char* fShaderString2)
	{
		if (!GPUImageTwoPassFilter::Initialize(vShaderString1, fShaderString1, vShaderString2, fShaderString2))
		{
			return false;
		}

		_verticalPassTexelWidthOffsetUniform = _filterProgram->GetUniformLocation("texelWidthOffset");
		_verticalPassTexelHeightOffsetUniform = _filterProgram->GetUniformLocation("texelHeightOffset");

		_horizontalPassTexelWidthOffsetUniform = _filterProgram2->GetUniformLocation("texelWidthOffset");
		_horizontalPassTexelHeightOffsetUniform = _filterProgram2->GetUniformLocation("texelHeightOffset");

		return true;
	}

	void GPUImageTwoPassTextureSamplingFilter::SetVerticalTexelSpacing(const float texelSpacing)
	{
		_verticalTexelSpacing = texelSpacing;

		SetupFilter(GetSizeOfFBO());
	}

	void GPUImageTwoPassTextureSamplingFilter::SetHorizontalTexelSpacing(const float texelSpacing)
	{
		_horizontalTexelSpacing = texelSpacing;

		SetupFilter(GetSizeOfFBO());
	}

	void GPUImageTwoPassTextureSamplingFilter::SetProgramUniforms(int index)
	{
		GPUImageTwoPassFilter::SetProgramUniforms(index);

		if (index == 0)
		{
			glUniform1f(_verticalPassTexelWidthOffsetUniform, _verticalPassTexelWidthOffset);
			glUniform1f(_verticalPassTexelHeightOffsetUniform, _verticalPassTexelHeightOffset);
		}
		else
		{
			glUniform1f(_horizontalPassTexelWidthOffsetUniform, _horizontalPassTexelWidthOffset);
			glUniform1f(_horizontalPassTexelHeightOffsetUniform, _horizontalPassTexelHeightOffset);
		}
	}

	void GPUImageTwoPassTextureSamplingFilter::SetupFilter(Size size)
	{
		if (GPUImageRotationSwapsWidthAndHeight(_inputRotation))
		{
			_verticalPassTexelWidthOffset = _verticalTexelSpacing / size.height;
			_verticalPassTexelHeightOffset = 0.0f;
		}
		else
		{
			_verticalPassTexelWidthOffset = 0.0f;
			_verticalPassTexelHeightOffset = _verticalTexelSpacing / size.height;
		}

		_horizontalPassTexelWidthOffset = _horizontalTexelSpacing / size.width;
		_horizontalPassTexelHeightOffset = 0.0f;
	}
}