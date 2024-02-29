#import <Metal/Metal.h>
#import <MetalPerformanceShaders/MetalPerformanceShaders.h>
#import <MetalKit/MetalKit.h>

#include "SVGFDenoiser.h"

SVGFDenoiser* SVGFDenoiser::alloc() {
    SVGFDenoiser *self = new SVGFDenoiser();
    self->_pDenoiser = [MPSSVGFDenoiser alloc];
    return self;
}

SVGFDenoiser* SVGFDenoiser::init(MTL::Device *pDevice) {
    MPSSVGFDenoiser* pDenoiser = [(id)this->_pDenoiser initWithDevice:(id)pDevice];
    pDenoiser.bilateralFilterIterations = 5;
    return this;
}

void SVGFDenoiser::release() {
    [(id)this->_pDenoiser release];
}

MTL::Texture* SVGFDenoiser::encodeToCommandBuffer(MTL::CommandBuffer *pCommandBuffer, MTL::Texture *pSourceTexture, MTL::Texture *pMotionVectorTexture, MTL::Texture *pDepthNormalTexture, MTL::Texture *pPreviousDepthNormalTexture) {
    MPSSVGFDenoiser* pDenoiser = (id)this->_pDenoiser;
    return (MTL::Texture*)[pDenoiser encodeToCommandBuffer:(id)pCommandBuffer
        sourceTexture:(id)pSourceTexture
        motionVectorTexture:(id)pMotionVectorTexture
        depthNormalTexture:(id)pDepthNormalTexture
        previousDepthNormalTexture:(id)pPreviousDepthNormalTexture
    ];
}