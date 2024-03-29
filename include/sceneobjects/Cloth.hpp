#pragma once

#include "SceneObject.hpp"

class Cloth: public SceneObject {
    public:
        Cloth(MTL::Device *pDevice, float size, uint32_t particleCount, float unitMass, float springConstant, float dampingConstant);
        ~Cloth();

        virtual void update(MTL::CommandBuffer *pCmd, MTL::AccelerationStructure *pAccelerationStructure, float dt, simd::float3 moveDirection, bool enable) override;
        virtual void updateGeometry() override;
    private:
        float _springConstant;
        float _size;
        uint32_t _particleCount;
        MTL::Size _clothTPG, _clothTPT;
        MTL::ComputePipelineState *_pComputeClothPipelineState;
        MTL::IntersectionFunctionTable *_pIntersectionFunctionTable;
        MTL::Buffer *_pVertexBuffer;
        MTL::Buffer *_pDataBuffer;
        MTL::Buffer *_pParticleBuffer;
};