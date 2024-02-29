#include "sceneobjects/FloorPlane.hpp"

const uint32_t indices[6] = {
    0, 1, 2,
    2, 1, 3
};

const PrimitiveData primitiveData[2] = {
    PrimitiveData{{}, {}, {}, {}, {}, {}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}},
    PrimitiveData{{}, {}, {}, {}, {}, {}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}},
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

    this->getDescriptor()->setTriangleCount(2);
    this->getDescriptor()->setVertexBuffer(pVertexBuffer);
    this->getDescriptor()->setIndexBuffer(pIndexBuffer);
    this->getDescriptor()->setPrimitiveDataBuffer(pDataBuffer);
    this->getDescriptor()->setPrimitiveDataStride(sizeof(PrimitiveData));
    this->getDescriptor()->setPrimitiveDataElementSize(sizeof(PrimitiveData));

    pVertexBuffer->release();
    pIndexBuffer->release();
    pDataBuffer->release();
}