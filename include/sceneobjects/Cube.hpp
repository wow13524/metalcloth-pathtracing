#pragma once

#include "SceneObject.hpp"

class Cube: public SceneObject {
    public:
        Cube(MTL::Device *pDevice, float size);
        virtual ~Cube() override;

        virtual void update() override;
    private:
        MTL::AccelerationStructureTriangleGeometryDescriptor *_pTriangleDescriptor;
};