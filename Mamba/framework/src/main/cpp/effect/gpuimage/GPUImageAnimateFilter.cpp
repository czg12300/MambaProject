//
// Created by liyongfa on 2016/4/17.
//

#include "include/GPUImageAnimateFilter.h"
#include "include/GPUImageMath.h"
#include "CVFaceTracker.h"

namespace e
{
	//不要改变这个数值
	const float kDefaultRefWidth = 720.0f;
	//const float kDefaultRefHeight = 960.0f;
	const char* kAnimatePath = "/storage/emulated/0/hivideo/effect/008";
	
	const char* const kGPUImageAnimateVertexShaderString = SHADER_STRING
	(
		attribute vec4 position;
		attribute vec4 inputTextureCoordinate;

		varying vec2 textureCoordinate;

		uniform mat4 modelViewProjection;

		void main() 
		{
			gl_Position = modelViewProjection * position;
			textureCoordinate = inputTextureCoordinate.xy;
		}				
	);

	const char* const kGPUImageAnimateFragmentShaderString = SHADER_STRING
	(
		uniform sampler2D inputImageTexture;
		varying highp vec2 textureCoordinate;

		void main() 
        {
			gl_FragColor = texture2D(inputImageTexture, textureCoordinate);
		}		
	);

	GPUImageAnimateFilter::GPUImageAnimateFilter(void)
	{
		_modelViewProjectionUniform = -1;
		_animeTexture = 0;
		_animeConfig = NULL;
		_animeManager = new AnimateManager();
		assert(_animeManager);
		memset(&_trackResult, 0, sizeof(_trackResult));
		_loadAnimate = false;

#ifndef USE_CREATE_FUNCTION
		Initialize();
#endif
		//测试
#if 1
		SetAnimatePath(kAnimatePath);
#endif
	}

	GPUImageAnimateFilter::~GPUImageAnimateFilter(void)
	{
		SafeDelete(&_animeManager);
	}

	bool GPUImageAnimateFilter::Initialize(void)
	{
		return Initialize(kGPUImageAnimateVertexShaderString, kGPUImageAnimateFragmentShaderString);
	}	

	bool GPUImageAnimateFilter::Initialize(const char* vShaderString, const char* fShaderString)
	{
		if (!GPUImageFilter::Initialize(vShaderString, fShaderString))
		{
			LOGE("GPUImageAnimateFilter initialize failed!");
			return false;
		}

		_modelViewProjectionUniform = _filterProgram->GetUniformLocation("modelViewProjection");
		LOGD("GPUImageAnimateFilter initialize ok: %d", _modelViewProjectionUniform);
		return true;
	}

	void GPUImageAnimateFilter::SetAnimatePath(const char* path)
	{
		if (_animePath != path)
		{
			_animePath = path;
			_loadAnimate = true;
		}
		
	}

	void GPUImageAnimateFilter::SetCacheSize(const int size)
	{
		if (_animeManager)
		{
			_animeManager->SetCacheSize(size);
		}
	}

	bool GPUImageAnimateFilter::LoadAnimate(void)
	{
		//加载特效
		if (!_animeManager->Load(_animePath.c_str()))
		{
			_loadAnimate = false;
			LOGE("GPUImageAnimateFilter load animes failed!");
			return false;
		}
		_loadAnimate = false;
		return true;
	}

	int GPUImageAnimateFilter::OnTrackProc(void* data)
	{
		memcpy(&_trackResult, data, sizeof(_trackResult));
        //LOGD("%f,%f,%f,%f",_trackResult.x1, _trackResult.y1, _trackResult.x2, _trackResult.y2);
		return 0;
	}

	void GPUImageAnimateFilter::SetProgramUniforms(int textureIndex)
	{

	}

	Size GPUImageAnimateFilter::GetSizeOfFBO(void) const
	{
		Size filterSize = _inputTextureSize;
		if (GPUImageRotationSwapsWidthAndHeight(_inputRotation))
		{
			filterSize.width = _inputTextureSize.height;
			filterSize.height = _inputTextureSize.width;
		}
		return filterSize;
	}

	void GPUImageAnimateFilter::SetupFilter(Size size)
	{
		
	}
	
	void GPUImageAnimateFilter::CalcMVPMatrix(void)
	{
		if (_trackResult.located)
		{
			Size size = GetSizeOfFBO();
			const int major = (size.width>=size.height) ? 0 : 1;
			const int minor = (size.width>=size.height) ? 1 : 0;
			const float dim[2] = {fminf(size.width, size.height), fmaxf(size.width, size.height)};
			const float clip[2] = {1.0f, dim[0] / dim[1]};
			const float factor = _animeConfig->width / kDefaultRefWidth * clip[major];
		    //计算投影矩阵
		    CalcOrtho2D(_projectionMatrix, -clip[major], clip[major], -clip[minor], clip[minor], 100, -100);
		    //计算观察矩阵
			CalcLookAt(_viewMatrix, Vector3(0,0,1), Vector3(0,0,0), Vector3(0,1,0));
			//根据人脸大小来定缩放比例
			const int index = _animeConfig->ref_point;
			const float sc = _trackResult.face_width / _animeConfig->ref_face_size * factor;
			const float tx = _trackResult.points[index*2+0] * clip[major];
			const float ty = _trackResult.points[index*2+1] * clip[minor];
			const float ox = _animeConfig->offset_x;
			const float oy = _animeConfig->offset_y;
			//计算模型矩阵
			_modelMatrix.Identity();
			//注意：左乘矩阵，最终的执行效果为逆方向（平移->旋转->缩放->偏移）
			_modelMatrix.Translate(ox, oy, 0);
			_modelMatrix.Scale(sc, sc, 1);
			_modelMatrix.RotateZ(_trackResult.angle_z);
			_modelMatrix.RotateY(_trackResult.angle_y);
			_modelMatrix.RotateX(_trackResult.angle_x);
			_modelMatrix.Translate(tx, ty, 0);
			//最终的变换矩阵
			_modelViewProjectionMatrix = _projectionMatrix * _viewMatrix * _modelMatrix;
		}
		else
		{
			_modelViewProjectionMatrix.Identity();
		}
	}
	
	void GPUImageAnimateFilter::RenderToTexture(const GLfloat* vertices, const GLfloat* textureCoordinates)
	{
		//定点和纹理坐标
		GLfloat vertices2[8] = {
			-1.0f, -1.0f,
			 1.0f, -1.0f,
			-1.0f,  1.0f,
			 1.0f,  1.0f
		};

		GLfloat textureCoordinates2[8] = {
			0.0f, 1.0f,
			1.0f, 1.0f,
			0.0f, 0.0f,
			1.0f, 0.0f
		};

		if (_loadAnimate)
		{
			LoadAnimate();
		}

		//每一帧的逻辑计算
		_animeManager->Step(!_trackResult.located, true, &_animeTexture, &_animeConfig);

		//计算变换矩阵
		CalcMVPMatrix();

        //OpenGL渲染处理部分
		_filterProgram->Use();
		_outputFramebuffer = FetchFramebuffer(GetSizeOfFBO());
		_outputFramebuffer->Active();

		SetProgramUniforms(0);

		glClearColor(_bgColorRed, _bgColorGreen, _bgColorBlue, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, _animeTexture);
		glUniform1i(_filterInputTextureUniform, 2);
		
        glUniformMatrix4fv(_modelViewProjectionUniform, 1, GL_FALSE, _modelViewProjectionMatrix.Get());

		glVertexAttribPointer(_filterPositionAttribute, 2, GL_FLOAT, GL_FALSE, 0, vertices2);
		glVertexAttribPointer(_filterTextureCoordinateAttribute, 2, GL_FLOAT, GL_FALSE, 0, textureCoordinates2);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		_firstInputFramebuffer = NULL;
	}
}
