#pragma once

#include "Metal.hpp"

class SceneObject {
    public:
        virtual ~SceneObject() = default;

        inline MTL::AccelerationStructureGeometryDescriptor* getDescriptor() {return this->_pDescriptor;};
        virtual void update() = 0;
    protected:
        MTL::AccelerationStructureGeometryDescriptor *_pDescriptor;
};