#pragma once

#include "Metal.hpp"
#include "../shaders/SharedTypes.h"

class Scene {
    public:
        virtual ~Scene() = default;

        inline MTL::AccelerationStructureDescriptor* getDescriptor() {return this->_pDescriptor;};
        virtual Camera getInitialCamera() = 0;
        virtual std::vector<uint16_t> getGeometryMaterials() = 0;
        virtual std::vector<Material> getMaterials() = 0;
        virtual void update(MTL::CommandBuffer *pCmd) = 0;
    protected:
        MTL::PrimitiveAccelerationStructureDescriptor *_pDescriptor;
};