#pragma once

#include "Metal.hpp"

class SVGFDenoiser {
    public:
        static SVGFDenoiser* alloc();
        SVGFDenoiser* init(MTL::Device *pDevice);
        void release();
        MTL::Texture* encodeToCommandBuffer(MTL::CommandBuffer *pCommandBuffer, MTL::Texture *pSourceTexture, MTL::Texture *pMotionVectorTexture, MTL::Texture *pDepthNormalTexture, MTL::Texture *pPreviousDepthNormalTexture);
    private:
        SVGFDenoiser() = default;
        void *_pDenoiser;
};