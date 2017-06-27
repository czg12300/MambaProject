//
// Created by yongfali on 2016/4/21.
//

#include "include/GPUImageMath.h"
#include "include/GPUImageBase.h"

namespace e
{
	void CalcOrtho2D(Matrix4 & _mat
		, float _left
		, float _right
		, float _bottom
		, float _top
		, float _near
		, float _far)
	{
#ifdef _USE_OPENGL_MATRIX
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			glOrthof(_left, _right, _bottom, _top, -100, 100);//正射投影
			glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)_mat.Get());
			glPopMatrix();	
#else
			_mat.Identity();
			_mat[0] = 2.0f /(_right - _left);
			_mat[5] = 2.0f / (_top - _bottom);	
			_mat[10] = -2.0f / (_far - _near);	
			_mat[12] = (_right + _left) / (_left - _right);
			_mat[13] = (_top + _bottom) / (_bottom - _top);
			_mat[14] = (_far + _near) / (_near - _far);
			_mat[15] = 1.0f;
#endif
	}

	void CalcPerspective(Matrix4 & _mat
		, float _fovyArc
		, float _aspect
		, float _near
		, float _far)
	{
#ifdef _USE_OPENGL_MATRIX
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			gluPerspective(_fovyArc * 180 / PI, _aspect, _near, _far);
			glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)_mat.Get());
			glPopMatrix();	
#else
			float yScale = 1 / tan(_fovyArc / 2);
			float xScale = yScale / _aspect;

			_mat.Identity();
			_mat[0] = xScale;
			_mat[5] = yScale;
			_mat[10] = _far / (_near - _far);
			_mat[11] = -1.0;
			_mat[14] = _near * _far / (_near - _far);
			_mat[15] = 0.0;		
#endif	
	}

	void CalcLookAt(Matrix4 & _mat
		, const Vector3 & _eye
		, const Vector3 & _lookAt
		, const Vector3 & _up)
	{
#ifdef _USE_OPENGL_MATRIX
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			gluLookAt(_eye.x, _eye.y, _eye.z, _lookAt.x, _lookAt.y, _lookAt.z, _up.x, _up.y, _up.z);
			glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat*)_mat.Get());
			glPopMatrix();	
#else
			Vector3 z = (_eye - _lookAt).Normalize();
			Vector3 x = (_up.Cross(z)).Normalize();
			Vector3 y = z.Cross(x);
    
			float a[16] = { 
				x.x, y.x, z.x, 0,
				x.y, y.y, z.y, 0,
				x.z, y.z, z.z, 0,
				-x.Dot(_eye), -y.Dot(_eye), -z.Dot(_eye), 1,
			};
			
			_mat.Set(a);
#endif			
	}
}

