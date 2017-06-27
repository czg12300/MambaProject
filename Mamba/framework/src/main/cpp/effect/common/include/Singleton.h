//
// Created by liyongfa on 2016/11/16.
//

#ifndef E_SINGLETON_H
#define E_SINGLETON_H

namespace e
{
    template<class T>
    class Singleton {
    public:
        static T* GetInstance()
        {
            if (instance == 0)
            {
                instance = new T();
            }
            return instance;
        }
    private:
        Singleton()
        {

        }
        ~Singleton()
        {
			instance = 0;
        }
    protected:
        static T* instance;
    };

    template <class T>
    T* Singleton<T>::instance = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    template<class T>
    class SingletonHolder {
    public:
        static void SetInstance(T* object)
        {
            instance = object;
        }

        static T* GetInstance()
        {
            return instance;
        }
    private:
        SingletonHolder()
        {

        }
        ~SingletonHolder()
        {
			instance = 0;
        }
    protected:
        static T* instance;
    };

    template <class T>
    T* SingletonHolder<T>::instance = 0;
}

#endif //E_SINGLETON_H
