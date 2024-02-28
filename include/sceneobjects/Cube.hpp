#pragma once

#include "SceneObject.hpp"

class Cube: public SceneObject {
    public:
        Cube(MTL::Device *pDevice, float size);
        virtual ~Cube() override;
    private:
        MTL::AccelerationStructureTriangleGeometryDescriptor *_pTriangleDescriptor;
};