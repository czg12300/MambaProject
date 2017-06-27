//
// Created by yongfali on 2016/4/13.
//

#include "include/GPUImageCallback.h"

namespace e 
{
	GPUImageCallback::GPUImageCallback()
	{
		o = 0;
		mf = 0;
		assert(sizeof(mf) >= sizeof(gf));
	}

	GPUImageCallback::GPUImageCallback(GLOBAL_FUNC _gf)
	{
		o = 0;
		mf = 0;
		gf = _gf;
	}

	int GPUImageCallback::operator()(void * _a)
	{
		assert(o != 0 || gf != 0);
		return o ? (o->*mf)(_a) : (gf ? gf(_a) : 0);
	}

	bool GPUImageCallback::operator==(const GPUImageCallback & _r) const
	{
		return  o == _r.o && mf == _r.mf;
	}

	const GPUImageCallback & GPUImageCallback::operator=(const GPUImageCallback & _r)
	{
		o = _r.o;
		mf = _r.mf;
		return *this;
	}	
}
