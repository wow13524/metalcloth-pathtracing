#pragma once

#include "Metal.hpp"
#include "SharedTypes.h"

class SceneObject {
    public:
        virtual ~SceneObject() = default;

        inline MTL::AccelerationStructureGeometryDescriptor* getDescriptor() {return this->_pDescriptor;};
        virtual void update(MTL::CommandBuffer *pCmd, MTL::AccelerationStructure *pAccelerationStructure, float dt) {};
        virtual void updateGeometry() {};
    protected:
        MTL::AccelerationStructureGeometryDescriptor *_pDescriptor;
};