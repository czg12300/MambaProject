//
// Created by yongfali on 2016/3/25.
//

#include "include/GPUImageBase.h"

namespace e 
{
	void GetOpenGLError(const char* operation, const char* function)
	{
		GLenum status = glGetError();
		while (status != GL_NO_ERROR)
		{
			LOGE("%s %s error: 0x%x", function, operation, status);
			status = glGetError();
		}
	}
}