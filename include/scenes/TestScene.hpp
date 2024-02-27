#pragma once

#include "sceneobjects/Cube.hpp"
#include "sceneobjects/FloorPlane.hpp"
#include "Scene.hpp"

class TestScene: public Scene {
    public:
        TestScene(MTL::Device *pDevice);
        virtual ~TestScene() override;

        virtual Camera getInitialCamera() override;
        virtual std::vector<uint16_t> getGeometryMaterials() override;
        virtual std::vector<Material> getMaterials() override;
        virtual void update(MTL::CommandBuffer *pCmd) override;
    private:
        Cube *_pCube;
        FloorPlane *_pFloorPlane;
};