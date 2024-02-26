#pragma once

#include "SceneObject.hpp"

class FloorPlane: public SceneObject {
    public:
        FloorPlane(MTL::Device *pDevice, float size);
        virtual ~FloorPlane() override;

        virtual void update() override;
    private:
        MTL::AccelerationStructureTriangleGeometryDescriptor *_pTriangleDescriptor;
};