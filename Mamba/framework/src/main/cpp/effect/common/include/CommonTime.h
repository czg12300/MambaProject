//
//Point.h
//
#ifndef E_TIME_H
#define E_TIME_H
#include <stdint.h>

namespace e
{
	class Time
	{
	public:
		Time();
	    Time(uint64_t time);
	    Time(const Time & r);
	    virtual ~Time(void);
	public:
		void Reset();
		uint64_t GetValue(void) const;
	    bool operator==(const Time & r);
	    bool operator!=(const Time & r);
	    static uint64_t GetTime(void);
	public:
		uint64_t value;
	};

	static Time kTimeInvalid(-1);

#ifndef GLTIME_IS_INDEFINITE
#	define GLTIME_IS_INDEFINITE(time) ((time)==kTimeInvalid)
#endif
}

#endif