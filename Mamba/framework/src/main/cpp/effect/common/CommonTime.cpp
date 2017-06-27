#include "include/CommonTime.h"
#include <sys/time.h>

namespace e
{
	Time::Time(void)
	{
		value = 0;
	}
	
	Time::Time(uint64_t time)
	{
	    this->value = time;
	}

	Time::Time(const Time & r)
	{
	    this->value = r.value;
	}

	Time::~Time(void)
	{
	    
	}

	void Time::Reset()
	{
		value = 0;
	}

	uint64_t Time::GetValue(void) const
	{
	    return value;
	}

	uint64_t Time::GetTime(void)
	{
	    timeval t;
	    gettimeofday(&t, NULL);
	    return t.tv_sec * 1000 + t.tv_usec / 1000;
	}

	bool Time::operator==(const Time & r)
	{
	    return this->value == r.value;
	}

	bool Time::operator!=(const Time & r)
	{
	    return !operator==(r);
	}	
}