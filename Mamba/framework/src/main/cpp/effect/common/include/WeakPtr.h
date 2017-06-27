#ifndef _E_WEAKPTR_H_
#define _E_WEAKPTR_H_

template<class T> class WeakPtr
{
public:
	WeakPtr(void)
	{

	}

	virtual ~WeakPtr(void)
	{

	}
private:
	T * p;
};