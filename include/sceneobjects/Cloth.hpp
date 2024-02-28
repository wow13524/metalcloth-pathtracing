#pragma once

#include "SceneObject.hpp"

constexpr float TARGET_TIMESTEP = 1.f / 960;

class Cloth: public SceneObject {
    public:
        Cloth(MTL::Device *pDevice, float size, uint32_t particleCount, float unitMass, float springConstant, float dampingConstant);
        virtual ~Cloth() override;

        virtual void update(MTL::CommandBuffer *pCmd, MTL::AccelerationStructure *pAccelerationStructure, float dt) override;
        virtual void updateGeometry() override;
    private:
        MTL::Size _clothTPG, _clothTPT;
        MTL::AccelerationStructureTriangleGeometryDescriptor *_pTriangleDescriptor;
        MTL::ComputePipelineState *_pComputeClothPipelineState;
        MTL::Buffer *_pVertexBuffer;
        MTL::Buffer *_pDataBuffer;
        MTL::Buffer *_pParticleBuffer;
};