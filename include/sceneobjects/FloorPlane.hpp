#pragma once

#include "SceneObject.hpp"

class FloorPlane: public SceneObject {
    public:
        FloorPlane(MTL::Device *pDevice, float size);
};