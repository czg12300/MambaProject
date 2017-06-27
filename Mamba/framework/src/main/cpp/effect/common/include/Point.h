//
//BaseCalss.h - create by yongfali
//
#ifndef E_POINT_H
#define E_POINT_H

namespace e
{
	template<class T> class _Point
	{
	public:
		_Point(void)
		{
			x = y = 0;
		}

		_Point(T x, T y)
		{
			this->x = x;
			this->y = y;
		}

		_Point(const _Point<T> & r)
		{
			x = r.x;
			y = r.y;
		}

		T X(void) const 
		{
			return x;
		}

		T Y(void) const 
		{
			return y;
		}

	    bool operator==(const _Point<T> & r) 
	    {
	        return x==r.x && y==r.y;
	    }

	    bool operator!=(const _Point<T> &other) 
	    {
	        return !operator==(other);
	    }
	public:
		T x, y;
	};
	
	typedef _Point<float> Point;

	static Point PointZero;	
}
#endif