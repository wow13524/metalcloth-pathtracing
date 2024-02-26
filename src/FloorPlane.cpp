#include "sceneobjects/FloorPlane.hpp"

const uint32_t indices[6] = {
    0, 1, 2,
    1, 2, 3
};

FloorPlane::FloorPlane(MTL::Device *pDevice, float size) {
    float vertices[12] = {
        -size / 2, 0, -size / 2,
        -size / 2, 0,  size / 2,
         size / 2, 0, -size / 2,
         size / 2, 0,  size / 2
    };

    MTL::Buffer *pVertexBuffer = pDevice->newBuffer(12 * sizeof(float), MTL::ResourceStorageModeManaged);
    MTL::Buffer *pIndexBuffer = pDevice->newBuffer(6 * sizeof(unsigned int), MTL::ResourceStorageModeManaged);
    memcpy(pVertexBuffer->contents(), vertices, 12 * sizeof(float));
    memcpy(pIndexBuffer->contents(), indices, 6 * sizeof(unsigned int));
    pVertexBuffer->didModifyRange(NS::Range::Make(0, 12 * sizeof(float)));
    pIndexBuffer->didModifyRange(NS::Range::Make(0, 6 * sizeof(unsigned int)));

    this->_pDescriptor = this->_pTriangleDescriptor = MTL::AccelerationStructureTriangleGeometryDescriptor::alloc()->init();
    this->_pTriangleDescriptor->setTriangleCount(2);
    this->_pTriangleDescriptor->setVertexBuffer(pVertexBuffer);
    this->_pTriangleDescriptor->setIndexBuffer(pIndexBuffer);

    pVertexBuffer->release();
    pIndexBuffer->release();
}

FloorPlane::~FloorPlane() {
    this->_pTriangleDescriptor->release();
}

void FloorPlane::update() {

}