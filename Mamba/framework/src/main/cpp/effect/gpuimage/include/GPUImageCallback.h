//
// Created by yongfali on 2016/4/13.
//

#ifndef E_GLCALLBACK_H
#define E_GLCALLBACK_H

#include "Object.h"
#include <assert.h>

namespace e
{
	class GPUImageCallback 
	{
		typedef int (*GLOBAL_FUNC)(void *);
		typedef int (Object::*OBJECT_FUNC)(void *);
		Object* o;
		union
		{
			GLOBAL_FUNC gf;
			OBJECT_FUNC mf;
		};
	public:
		GPUImageCallback(void);
		GPUImageCallback(GLOBAL_FUNC _gf);
		// 用模板构造是为了免除外部的强制转换
		template <typename T> 
		GPUImageCallback(T * _obj, int (T::*_mf)(void *))
		{
			o = _obj;
			assert(sizeof(mf) == sizeof(_mf));
			mf = (OBJECT_FUNC) _mf;
		}
	private:
		GPUImageCallback(void*, void*); // 假的构造函数, 如果报错在这里, 请检查函数原型是否匹配
	public:
		int operator()(void * _a = 0);
		bool operator==(const GPUImageCallback & _r) const;
		const GPUImageCallback & operator=(const GPUImageCallback & _r);
		operator bool() const { return o != 0; }
	};
}

#endif //E_GLCALLBACK_H
