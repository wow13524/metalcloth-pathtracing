#include "sceneobjects/Cube.hpp"

const uint32_t indices[36] = {
    0, 1, 2,
    2, 1, 3,
    0, 1, 4,
    4, 1, 5,
    2, 0, 4,
    2, 4, 6,
    3, 2, 6,
    3, 6, 7,
    1, 3, 7,
    1, 7, 5,
    4, 5, 6,
    6, 5, 7
};

const PrimitiveData primitiveData[12] = {
    PrimitiveData{{}, {}, {}, {}, {}, {}, {0, -1, 0}, {0, -1, 0}, {0, -1, 0}},
    PrimitiveData{{}, {}, {}, {}, {}, {}, {0, -1, 0}, {0,- 1, 0}, {0, -1, 0}},
    PrimitiveData{{}, {}, {}, {}, {}, {}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}},
    PrimitiveData{{}, {}, {}, {}, {}, {}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}},
    PrimitiveData{{}, {}, {}, {}, {}, {}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}},
    PrimitiveData{{}, {}, {}, {}, {}, {}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}},
    PrimitiveData{{}, {}, {}, {}, {}, {}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}},
    PrimitiveData{{}, {}, {}, {}, {}, {}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}},
    PrimitiveData{{}, {}, {}, {}, {}, {}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}},
    PrimitiveData{{}, {}, {}, {}, {}, {}, {0, 0, 1}, {0, 0, 1}, {0, 0, 1}},
    PrimitiveData{{}, {}, {}, {}, {}, {}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}},
    PrimitiveData{{}, {}, {}, {}, {}, {}, {0, 1, 0}, {0, 1, 0}, {0, 1, 0}},
};

Cube::Cube(MTL::Device *pDevice, float size) {
    float vertices[24] = {
        -size / 2,    0, -size / 2,
        -size / 2,    0,  size / 2,
         size / 2,    0, -size / 2,
         size / 2,    0,  size / 2,
        -size / 2, size, -size / 2,
        -size / 2, size,  size / 2,
         size / 2, size, -size / 2,
         size / 2, size,  size / 2
    };

    MTL::Buffer *pVertexBuffer = pDevice->newBuffer(24 * sizeof(float), MTL::ResourceStorageModeManaged);
    MTL::Buffer *pIndexBuffer = pDevice->newBuffer(36 * sizeof(unsigned int), MTL::ResourceStorageModeManaged);
    MTL::Buffer *pDataBuffer = pDevice->newBuffer(12 * sizeof(PrimitiveData), MTL::ResourceStorageModeManaged);
    memcpy(pVertexBuffer->contents(), vertices, pVertexBuffer->length());
    memcpy(pIndexBuffer->contents(), indices, pIndexBuffer->length());
    memcpy(pDataBuffer->contents(), primitiveData, pDataBuffer->length());
    pVertexBuffer->didModifyRange(NS::Range::Make(0, pVertexBuffer->length()));
    pIndexBuffer->didModifyRange(NS::Range::Make(0, pIndexBuffer->length()));
    pDataBuffer->didModifyRange(NS::Range::Make(0, pDataBuffer->length()));

    this->getDescriptor()->setTriangleCount(12);
    this->getDescriptor()->setVertexBuffer(pVertexBuffer);
    this->getDescriptor()->setIndexBuffer(pIndexBuffer);
    this->getDescriptor()->setPrimitiveDataBuffer(pDataBuffer);
    this->getDescriptor()->setPrimitiveDataStride(sizeof(PrimitiveData));
    this->getDescriptor()->setPrimitiveDataElementSize(sizeof(PrimitiveData));

    pVertexBuffer->release();
    pIndexBuffer->release();
    pDataBuffer->release();
}