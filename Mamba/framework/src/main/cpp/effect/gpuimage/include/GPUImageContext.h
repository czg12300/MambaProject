//
// Created by liyongfa on 2016/4/9.
//

#ifndef E_GLCONTEXT_H
#define E_GLCONTEXT_H

#include "GPUImageCommon.h"
#include "GPUImageRotationMode.h"
#include "GPUImageFramebuffer.h"

namespace e
{
    class GLContext : public RefObject
    {
    public:
        GLContext(void);
        virtual ~GLContext(void);
    };
    
    //filter input interface
    class GPUImageInput
    {
    public:
        GPUImageInput(void) {};
        virtual ~GPUImageInput(void){};
    public:
        virtual void NewFrameReady(Time frameTime, int index) = 0;
        virtual void SetInputSize(Size size, int index) = 0;
        virtual void SetInputFramebuffer(Ptr<GPUImageFramebuffer> & frameBuffer, int index) = 0;
        virtual void SetInputRotationMode(GPUImageRotationMode & rotationMode, int index) = 0;
        virtual int  NextAvailableTextureIndex(void) = 0;
        //virtual Size MaximumOutputSize(void)  = 0;
        virtual void EndProcessing(void) = 0;
        //virtual bool ShouldIgnoreUpdatesToThisTarget(void) = 0;
        virtual bool IsEnable(void) = 0;
        //virtual bool WantsMonochromeInput(void) = 0;
        //virtual bool SetCurrentlyReceivingMonochromeInput(bool value) = 0;

        virtual void Reset(void) = 0;
    };
}

#endif //E_GLCONTEXT_H
