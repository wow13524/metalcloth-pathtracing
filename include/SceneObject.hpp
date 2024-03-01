#pragma once

#include "Metal.hpp"
#include "SharedTypes.h"

class SceneObject {
    public:
        SceneObject();
        virtual ~SceneObject();

        inline MTL::AccelerationStructureTriangleGeometryDescriptor* getDescriptor() {return this->_pDescriptor;};
        virtual void update(MTL::CommandBuffer *pCmd, MTL::AccelerationStructure *pAccelerationStructure, float dt, simd::float3 moveDirection, bool enable) {};
        virtual void updateGeometry() {};
    private:
        MTL::AccelerationStructureTriangleGeometryDescriptor *_pDescriptor;
};