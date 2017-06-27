#ifndef E_AUTOPTR_H
#define E_AUTOPTR_H
#include "Object.h"

namespace e
{
	template<typename T> class Ptr
	{
	public:
		Ptr(void)
		{
			this->p = 0;
		}
		
		Ptr(T * p)
		{
			this->p = p;

			if (this->p)
			{
				_Ptr()->AddRef();
			}
		}

		Ptr(const Ptr<T> & t)
		{
			this->p = t.p;

			if (this->p)
			{
				_Ptr()->AddRef();
			}
		}

		virtual ~Ptr(void)
		{
			if (this->p)
			{
				_Ptr()->Release();
			}
		}

		const Ptr<T> & operator=(const Ptr<T> & p)
		{
			if (this->p != p)
			{
				if (this->p)
				{
					_Ptr()->Release();
				}

				this->p = p;
				if (p)
				{
					_Ptr()->AddRef();
				}
			}

			return (*this);
		}

		const Ptr<T> & operator=(T * p)
		{
			if (this->p)
			{
				_Ptr()->Release();
			}

			this->p = p;

			if (this->p)
			{
				_Ptr()->AddRef();
			}

			return (*this);
		}

		void Clear(void)
		{
			if (this->p)
			{
				_Ptr()->Release();
			}

			this->p = 0;
		}
	public:
		T * operator->(void) const
		{
			return p;
		}

		T & operator*(void) const
		{
			return *p;
		}

		operator T * (void) const
		{
			return p;
		}

		bool operator==(const T* p)
		{
			return this->p == p;
		}

		bool operator!=(const T* p)
		{
			return !operator==(p);
		}

		bool operator==(const Ptr<T> & t)
		{
			return this->p == t.p;
		}

		bool operator!=(const Ptr<T> & t)
		{
			return !operator==(t);
		}

		T* Get(void) const
		{
			return p;
		}

		RefObject* _Ptr(void)
		{
			return (RefObject*)(p);
		}
	public:
		T * p;
	};
}

#endif
