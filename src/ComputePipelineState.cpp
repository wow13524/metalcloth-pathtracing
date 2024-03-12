#include "ComputePipelineState.hpp"

ComputePipelineState::ComputePipelineState(MTL::Device *pDevice, MTL::Function *pFunction, MTL::Size contextSize) {
    NS::Error *pErr;
    this->_pComputePipelineState = pDevice->newComputePipelineState(pFunction, &pErr);
    assertNSError(pErr);

    unsigned int threadgroupWidth = this->_pComputePipelineState->threadExecutionWidth();
    unsigned int threadgroupHeight = this->_pComputePipelineState->maxTotalThreadsPerThreadgroup() / threadgroupWidth;
    unsigned int threadgroupDepth = 1;
    this->_threadgroupsPerGrid = MTL::Size::Make(
        (contextSize.width + threadgroupWidth - 1) / threadgroupWidth,
        (contextSize.height + threadgroupHeight - 1) / threadgroupHeight,
        (contextSize.depth + threadgroupDepth - 1) / threadgroupDepth
    );
    this->_threadsPerThreadgroup = MTL::Size::Make(threadgroupWidth, threadgroupHeight, threadgroupDepth);
}

ComputePipelineState::~ComputePipelineState() {
    this->_pComputePipelineState->release();
}

void ComputePipelineState::dispatch(MTL::ComputeCommandEncoder *pCEnc) {
    pCEnc->setComputePipelineState(this->_pComputePipelineState);
    pCEnc->dispatchThreadgroups(this->_threadgroupsPerGrid, this->_threadsPerThreadgroup);
}