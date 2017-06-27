#ifndef _E_CVFACETRACKER_H_
#define _E_CVFACETRACKER_H_
#include "st_mobile.h"
#include "IFaceTracker.h"
#include <map>

namespace e
{
    class CVFaceTracker 
        : public IFaceTracker
    {
    public:
        CVFaceTracker(void);
        virtual ~CVFaceTracker(void);
        virtual int SampleProc(void* data);
    protected:
        void AdapterPoints(const st_mobile_106_t* face);
        virtual int ResetResult(TrackResult* trackResult);
    protected:
        st_handle_t _trackHandle;
    }; 
}

#endif