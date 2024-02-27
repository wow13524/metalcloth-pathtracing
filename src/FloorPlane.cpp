#include "sceneobjects/FloorPlane.hpp"

const uint32_t indices[6] = {
    0, 1, 2,
    1, 2, 3
};

const PrimitiveData primitiveData[2] = {
    PrimitiveData{{0, 1, 0}, {0, 1, 0}, {0, 1, 0}},
    PrimitiveData{{0, 1, 0}, {0, 1, 0}, {0, 1, 0}},
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
    MTL::Buffer *pDataBuffer = pDevice->newBuffer(2 * sizeof(PrimitiveData), MTL::ResourceStorageModeManaged);
    memcpy(pVertexBuffer->contents(), vertices, pVertexBuffer->length());
    memcpy(pIndexBuffer->contents(), indices, pIndexBuffer->length());
    memcpy(pDataBuffer->contents(), primitiveData, pDataBuffer->length());
    pVertexBuffer->didModifyRange(NS::Range::Make(0, pVertexBuffer->length()));
    pIndexBuffer->didModifyRange(NS::Range::Make(0, pIndexBuffer->length()));
    pDataBuffer->didModifyRange(NS::Range::Make(0, pDataBuffer->length()));

    this->_pDescriptor = this->_pTriangleDescriptor = MTL::AccelerationStructureTriangleGeometryDescriptor::alloc()->init();
    this->_pTriangleDescriptor->setTriangleCount(2);
    this->_pTriangleDescriptor->setVertexBuffer(pVertexBuffer);
    this->_pTriangleDescriptor->setIndexBuffer(pIndexBuffer);
    this->_pTriangleDescriptor->setPrimitiveDataBuffer(pDataBuffer);
    this->_pTriangleDescriptor->setPrimitiveDataStride(sizeof(PrimitiveData));
    this->_pTriangleDescriptor->setPrimitiveDataElementSize(sizeof(PrimitiveData));

    pVertexBuffer->release();
    pIndexBuffer->release();
    pDataBuffer->release();
}

FloorPlane::~FloorPlane() {
    this->_pTriangleDescriptor->release();
}

void FloorPlane::update() {

}