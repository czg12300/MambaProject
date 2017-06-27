//
// Created by liyongfa on 2016/4/9.
//

#ifndef E_GLBASEOUTPUT_H
#define E_GLBASEOUTPUT_H

#include "GPUImageBase.h"
#include "GPUImageContext.h"
#include "GPUImageFramebuffer.h"

namespace e
{
    struct TargetItem
    {
        int index;
        GPUImageInput* target;

        TargetItem(void)
        {
            index = -1;
            target = 0;
        };

        TargetItem(GPUImageInput* target, int index)
        {
            this->target = target;
            this->index = index;
        }

        bool operator==(const TargetItem & item)
        {
            return target == item.target;
        }

        bool operator!=(const TargetItem & item)
        {
            return !operator==(item);
        }
    };

    typedef std::list<TargetItem> TargetList;


    ////////////////////////////////////////////////////////////////////////////////////////////////
    class GPUImageOutput : public RefObject
    {
    public:
        GPUImageOutput(void);
        virtual ~GPUImageOutput(void);    
    public:
        virtual bool Initialize(void);
        virtual void SetInputFramebuffer(GPUImageInput* target, int index);
        virtual void SetOutputTextureOptions(GPUImageTextureOptions & textureOptions); 
        //virtual Size GetOutputFrameSize() const;
        virtual Ptr<GPUImageFramebuffer> & GetOutputFramebuffer(void);
        virtual void RemoveOutputFramebuffer(void);
        
        void AddTarget(GPUImageInput* target);
        void AddTarget(GPUImageInput* target, int index);
        void RemoveTarget(GPUImageInput* target);
        void RemoveAllTargets(void);
        TargetList* GetTargets(void);
    protected:
        Ptr<GPUImageFramebuffer> _outputFramebuffer;
        GPUImageTextureOptions _outputTextureOptions;
        Size _inputTextureSize;
        bool _overrideInputSize;
        TargetList* _targets;
    };

    typedef Ptr<GPUImageOutput> GPUImageOutputPtr;
}

#endif //_E_GLBASEOUTPUT_H_
