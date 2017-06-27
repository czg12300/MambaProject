#ifndef E_RECT_H
#define E_RECT_H
namespace e
{
	template <class T> class _Rect
	{
	public:
		_Rect(void)
		{
			x = y = width = height = 0;
		}

		_Rect(const _Rect<T> & r)
		{
			x = r.x;
			y = r.y;
			width = r.width;
			height = r.height;
		}

		_Rect(T x, T y, T width, T height)
		{
			this->x = x;
			this->y = y;
			this->width = width;
			this->height = height;
		}

		T X(void) const 
		{
			return x;
		}

		T Y(void) const 
		{
			return y;
		}

		T Width(void) const 
		{
			return width;
		}

		T Height(void) const 
		{
			return height;
		}

		bool operator==(const _Rect<T> & r)
		{
			return x==r.x && y==r.y && width==r.width && height==r.height;
		}
		
		bool operator!=(const _Rect<T> & r)
		{
			return !operator==(r);
		}
	public:
		T x;
		T y;
		T width;
		T height;
	};

	typedef _Rect<float> Rect;	
}
#endif