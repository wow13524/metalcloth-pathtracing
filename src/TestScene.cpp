#include "scenes/TestScene.hpp"

const uint16_t geoMats[2] = {
    0, 1
};

const Material mats[2] = {
    Material{.color = {0.5f, 0.5f, 1.0f}, .roughness = 0.25},
    Material{.color = {1.0f, 0.5f, 0.5f}, .roughness = 0.05}
};

TestScene::TestScene(MTL::Device *pDevice) {
    this->_pDescriptor = MTL::PrimitiveAccelerationStructureDescriptor::alloc()->init();
    this->_pCube = new Cube(pDevice, 1);
    this->_pFloorPlane = new FloorPlane(pDevice, 5);
    
    const NS::Object *pGeometryDescriptors[2] = {
        this->_pCube->getDescriptor(),
        this->_pFloorPlane->getDescriptor()
    };

    this->_pDescriptor->setGeometryDescriptors(NS::Array::alloc()->init(pGeometryDescriptors, 2));
}

TestScene::~TestScene() {
    this->_pDescriptor->release();
}

Camera TestScene::getInitialCamera() {
    return Camera(
        simd::float3{1, 0, 0},
        simd::float3{0, 0.707106781, 0.707106781},
        simd::float3{0, -0.707106781, 0.707106781},
        simd::float3{0, 3, -3}
    );
}

std::vector<uint16_t> TestScene::getGeometryMaterials() {
    return std::vector<uint16_t>(std::begin(geoMats), std::end(geoMats));
}

std::vector<Material> TestScene::getMaterials() {
    return std::vector<Material>(std::begin(mats), std::end(mats));
}

void TestScene::update(MTL::CommandBuffer *pCmd) {

}