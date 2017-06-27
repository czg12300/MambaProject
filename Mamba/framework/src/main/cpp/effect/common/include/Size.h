#ifndef E_SIZE_H
#define E_SIZE_H

namespace e
{
	struct Size 
	{
		Size(void)
		{	
			width = height = 0;
		}

		Size(int w, int h)
		{
			width = w;
			height = h;
		}

		Size(const Size & r)
		{
			width = r.width;
			height = r.height;
		}

		int Width(void) const 
		{
			return width;
		}

		int Height(void) const 
		{
			return height;
		}

		int Area(void) const
		{
			return width * height;
		}

	    bool operator==(const Size &other) 
	    {
	        return width == other.width && height == other.height;
	    }

	    bool operator!=(const Size &other) 
	    {
	        return !operator==(other);
	    }

	public:
		int width, height;
	};

	static Size kSizeZero;
}

#endif