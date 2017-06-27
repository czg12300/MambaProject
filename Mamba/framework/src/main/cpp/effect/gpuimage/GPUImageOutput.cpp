//
// Created by liyongfa on 2016/4/9.
//

#include "include/GPUImageOutput.h"
#include <GLES2/gl2.h>
#include <algorithm>
#include <assert.h>

namespace e
{
    GPUImageOutput::GPUImageOutput(void)
    {
        _outputFramebuffer = NULL;
    	_overrideInputSize = false;

        _targets = new TargetList();
        assert(_targets);
    }
    
    GPUImageOutput::~GPUImageOutput(void)
    {
		_outputFramebuffer = NULL;
        RemoveAllTargets();
        SafeDelete(&_targets);
    }

    bool GPUImageOutput::Initialize(void)
    {
        //TODO:
        return true;
    }

    void GPUImageOutput::SetInputFramebuffer(GPUImageInput* target, int index)
    {
    	if (target){
            target->SetInputFramebuffer(GetOutputFramebuffer(), index);
        }
    }

    void GPUImageOutput::SetOutputTextureOptions(GPUImageTextureOptions & textureOptions)
    {
    	_outputTextureOptions = textureOptions;

    	if (_outputFramebuffer->GetTexture())
    	{
    		glBindTexture(GL_TEXTURE_2D, _outputFramebuffer->GetTexture());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _outputTextureOptions.minFilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _outputTextureOptions.magFilter);
	        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _outputTextureOptions.wrapS);
	        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _outputTextureOptions.wrapT);
	        glBindTexture(GL_TEXTURE_2D, 0);    		
    	}
    }

    // Size GPUImageOutput::GetOutputFrameSize() const 
    // {
    // }

    Ptr<GPUImageFramebuffer> & GPUImageOutput::GetOutputFramebuffer(void)
    {
    	return _outputFramebuffer;
    }

    void GPUImageOutput::RemoveOutputFramebuffer(void)
    {
    	_outputFramebuffer = NULL;
    }

    void GPUImageOutput::AddTarget(GPUImageInput* target)
    {
    	if(target){
            AddTarget(target, target->NextAvailableTextureIndex());
        }
    }

    void GPUImageOutput::AddTarget(GPUImageInput* target, int index)
    {
        TargetItem targetItem(target, index);
    	TargetList::iterator it = std::find(_targets->begin(), _targets->end(), targetItem);
    	if (it != _targets->end()) return;

    	SetInputFramebuffer(target, index);
    	_targets->push_back(targetItem);
    }

    void GPUImageOutput::RemoveTarget(GPUImageInput* target)
    {
        TargetItem targetItem(target, -1);
    	TargetList::iterator it = std::find(_targets->begin(), _targets->end(), targetItem);
    	if (it == _targets->end()) return;

    	int index = it->index;
    	target->SetInputSize(kSizeZero, index);
    	target->SetInputRotationMode(kGPUImageNoRotation, index);
        target->EndProcessing();
    	_targets->erase(it);
    }

    void GPUImageOutput::RemoveAllTargets(void)
    {
    	TargetList::iterator it = _targets->begin();
    	for (; it != _targets->end(); it++)
    	{
            //it->target->SetInputSize(kSizeZero, it->index);
            //it->target->SetInputRotationMode(kGPUImageNoRotation, it->index);
            it->target->Reset();
    	}
    	_targets->clear();
    }

   	TargetList* GPUImageOutput::GetTargets(void)
   	{
   		return _targets;
   	}
}
