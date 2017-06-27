#ifndef _E_THREAD_H_
#define _E_THREAD_H_
#include <pthread.h>

namespace e
{
	class Mutex
	{
	public:
		Mutex(void)
		{
			pthread_mutex_init(&mutex, NULL);
		}
		virtual ~Mutex(void)
		{
			pthread_mutex_destroy(&mutex);
		}
		void Lock(void)
		{
			pthread_mutex_lock(&mutex);
		}
		void Unlock(void)
		{
			pthread_mutex_unlock(&mutex);
		}
	protected:
		pthread_mutex_t mutex;
	};

	class AutoLock
	{
	public:
		AutoLock(Mutex* _lock)
		{
			lock = _lock;
			if (lock){
				lock->Lock();
			}
		}
		virtual ~AutoLock(void)
		{
			if (lock){
				lock->Unlock();
			}
		}
	private:
		Mutex* lock;
	};
}

#endif