#pragma once

#include "Metal.hpp"
#include "SVGFDenoiser.h"
#include "scenes/TestScene.hpp"

constexpr uint32_t BOUNCES = 8;
constexpr uint32_t SPP = 1;
constexpr float FOV = 90.0f * M_PI / 360;

class SVGFDenoiserAdapter;

class Renderer {
    public:
        Renderer(MTL::Device* pDevice, MTK::View *pView);
        ~Renderer();
        void initializeDenoiser();
        void draw(MTK::View* pView);
        void loadScene(Scene *pSscene);
    private:
        MTL::Size _resetTPG, _resetTPT;
        MTL::Size _rayTPG, _rayTPT;
        MTL::Size _sceneTPG, _sceneTPT;
        MTL::Device *_pDevice;
        MTL::CommandQueue *_pCommandQueue;
        MTL::ComputePipelineState *_pComputeMotionPipelineState;
        MTL::ComputePipelineState *_pComputeResetPipelineState;
        MTL::ComputePipelineState *_pComputeRayPipelineState;
        MTL::ComputePipelineState *_pComputeScenePipelineState;
        MTL::RenderPipelineState *_pRenderPipelineState;
        SVGFDenoiser *_pDenoiser;
        MTL::Texture *_pDepthNormalTextures[2];
        MTL::Texture *_pMotionTexture;
        MTL::Texture *_pOutputTexture;
        MTL::Buffer *_pRayBuffer;
        MTL::Buffer *_pGeometryMaterialBuffer;
        MTL::Buffer *_pMaterialBuffer;
        MTL::Buffer *_pCameraBuffer;
        MTL::Buffer *_pScratchBuffer;
        MTL::AccelerationStructure *_pAccelerationStructure;
        simd::float4x4 _projectionMatrix;
        Camera _camera;
        Scene *_pScene = nullptr;
        
        std::chrono::system_clock::time_point _lastFrame = std::chrono::system_clock::now();
};