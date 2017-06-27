//
// Created by liyongfa on 2017/3/18.
//

#include "include/VideoTracker.h"

namespace e
{
	VideoTracker::VideoTracker()
	{
		_enable = true;
	}

	VideoTracker::~VideoTracker()
	{

	}

	void VideoTracker::SetEnable(bool enable)
	{
		_enable = enable;
	}

	bool VideoTracker::IsEnable() const 
	{
		return _enable;
	}

	int VideoTracker::OnSampleProc(void* data)
	{
		//TODO:

		return 0;
	}
}
