#pragma once

#include "Metal.hpp"
#include "scenes/TestScene.hpp"

constexpr float FOV = 90.0f * M_PI / 360;

class Renderer {
    public:
        Renderer(MTL::Device* pDevice, MTK::View *pView);
        ~Renderer();
        void draw(MTK::View* pView);
        void loadScene(Scene *pSscene);
    private:
        MTL::Size _threadgroupsPerGrid;
        MTL::Size _threadsPerThreadgroup;
        MTL::Device *_pDevice;
        MTL::CommandQueue *_pCommandQueue;
        MTL::ComputePipelineState *_pComputePipelineState;
        MTL::RenderPipelineState *_pRenderPipelineState;
        MTL::Texture *_pOutputTexture;
        MTL::Buffer *_pCameraBuffer;
        MTL::Buffer *_pGeometryMaterialBuffer;
        MTL::Buffer *_pMaterialBuffer;
        MTL::Buffer *_pScratchBuffer;
        MTL::AccelerationStructure *_pAccelerationStructure;
        Camera _camera;
        Scene *_pScene = nullptr;
};