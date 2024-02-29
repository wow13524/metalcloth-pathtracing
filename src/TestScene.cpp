#include "scenes/TestScene.hpp"

const uint16_t geoMats[3] = {
    0, 1, 2
};

const Material mats[3] = {
    Material{.color = {0.5f, 1.0f, 0.5f}, .roughness = 1},
    Material{.color = {0.5f, 0.5f, 1.0f}, .roughness = 0.25},
    Material{.color = {1.0f, 0.5f, 0.5f}, .roughness = 0.05}
};

TestScene::TestScene(MTL::Device *pDevice) {
    this->_pDescriptor = MTL::PrimitiveAccelerationStructureDescriptor::alloc()->init();
    this->addObject(new Cloth(pDevice, 2, 20, 1, 20, 1));
    this->addObject(new Cube(pDevice, 1));
    this->addObject(new FloorPlane(pDevice, 5));
}

Camera TestScene::getInitialCamera() {
    return Camera(
        simd::float3{1, 0, 0},
        simd::float3{0, 0.707106781, 0.707106781},
        simd::float3{0, -0.707106781, 0.707106781},
        simd::float3{0, 5, -5}
    );
}

std::vector<uint16_t> TestScene::getGeometryMaterials() {
    return std::vector<uint16_t>(std::begin(geoMats), std::end(geoMats));
}

std::vector<Material> TestScene::getMaterials() {
    return std::vector<Material>(std::begin(mats), std::end(mats));
}