#include "scenes/TestScene.hpp"

const uint16_t geoMats[1] = {
    0
};

const Material mats[1] = {
    Material{.color = {1.0f, 0.5f, 0.5f}}
};

TestScene::TestScene(MTL::Device *pDevice) {
    this->_pDescriptor = MTL::PrimitiveAccelerationStructureDescriptor::alloc()->init();
    this->_pFloorPlane = new FloorPlane(pDevice, 5);
    
    const NS::Object *pGeometryDescriptors[1] = {
        this->_pFloorPlane->getDescriptor()
    };

    this->_pDescriptor->setGeometryDescriptors(NS::Array::alloc()->init(pGeometryDescriptors, 1));
}

TestScene::~TestScene() {
    this->_pDescriptor->release();
}

Camera TestScene::getInitialCamera() {
    return Camera(
        simd::float3{1, 0, 0},
        simd::float3{0, 0.707106781, 0.707106781},
        simd::float3{0, -0.707106781, 0.707106781},
        simd::float3{0, 2, -2}
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