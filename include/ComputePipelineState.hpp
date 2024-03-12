#pragma once

#include "Metal.hpp"
#include "Utils.hpp"

class ComputePipelineState {
    public:
        ComputePipelineState(MTL::Device *pDevice, MTL::Function *pFunction, MTL::Size contextSize);
        ~ComputePipelineState();
        void dispatch(MTL::ComputeCommandEncoder *pCEnc);
    private:
        MTL::ComputePipelineState *_pComputePipelineState;
        MTL::Size _threadgroupsPerGrid, _threadsPerThreadgroup;
};