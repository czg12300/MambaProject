//
// Created by liyongfa on 2016/11/7.
//

#ifndef E_OBJECT_H
#define E_OBJECT_H

namespace e
{
    class Object
    {
    public:
        virtual ~Object(void) = 0;
    };

    class RefObject : public Object
    {
    public:
        void AddRef(void)
        {
            refCount++;
        }
        void Release(void)
        {
            refCount--;
            if (refCount <= 0)
            {
                DeleteObject();
            }
        }
        int RefCount(void) const
        {
            return refCount;
        }
    protected:
        RefObject(void) : refCount(0)
        {}
        virtual ~RefObject(void)
        {}
        virtual void DeleteObject(void)
        { delete this; }
    private:
        int refCount;
    };
}

#endif //KUGOUEFFECT_OBJECT_H
