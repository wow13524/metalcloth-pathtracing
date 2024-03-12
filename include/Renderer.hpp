#pragma once

#include "EventDelegate.h"
#include "EventView.h"
#include "Metal.hpp"
#include "SVGFDenoiser.h"
#include "scenes/TestScene.hpp"

constexpr uint32_t BOUNCES = 8;
constexpr uint32_t SPP = 32;
constexpr float FOV = 90.0f * M_PI / 360;

class Renderer: public EventDelegate {
    public:
        Renderer(MTL::Device* pDevice, EventView *pView);
        ~Renderer();
        void initializeDenoiser();
        void draw(MTK::View* pView);
        void loadScene(Scene *pSscene);
        virtual void keyDown(unsigned int keyCode) override;
        virtual void keyUp(unsigned int keyCode) override;
        virtual void mouseDragged(float deltaX, float deltaY) override;
    private:
        MTL::Size _sceneTPG, _sceneTPT;
        MTL::Device *_pDevice;
        MTL::CommandQueue *_pCommandQueue;
        MTL::ComputePipelineState *_pComputeMotionPipelineState;
        MTL::ComputePipelineState *_pComputeScenePipelineState;
        MTL::RenderPipelineState *_pRenderPipelineState;
        SVGFDenoiser *_pDenoiser;
        MTL::Texture *_pHdriTexture;
        MTL::Texture *_pDepthNormalTextures[2];
        MTL::Texture *_pMotionTexture;
        MTL::Texture *_pOutputTexture;
        MTL::Buffer *_pGeometryMaterialBuffer;
        MTL::Buffer *_pMaterialBuffer;
        MTL::Buffer *_pScratchBuffer;
        MTL::AccelerationStructure *_pAccelerationStructure;
        simd::float4x4 _projectionMatrix;
        bool _wind = false;
        simd::float3 _clothDirection = {0, 0, 0};
        simd::float3 _moveDirection = {0, 0, 0};
        Camera _camera;
        Scene *_pScene = nullptr;
        
        std::chrono::system_clock::time_point _lastFrame = std::chrono::system_clock::now();
};