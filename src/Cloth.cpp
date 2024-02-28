#include "sceneobjects/Cloth.hpp"

Cloth::Cloth(MTL::Device *pDevice, float size, uint32_t particleCount, float unitMass, float springConstant, float dampingConstant) {
    NS::Error *err = nullptr;

    float vertices[3 * particleCount * particleCount];
    unsigned int indices[6 * (particleCount - 1) * (particleCount - 1)];
    PrimitiveData primitiveData[2 * (particleCount - 1) * (particleCount - 1)];
    Particle particleData[particleCount * particleCount];
    
    //generate indices
    for (int i = 0; i < particleCount - 1; i++) {
        for (int j = 0; j < particleCount - 1; j++) {
            unsigned int index = 6 * (i * (particleCount - 1) + j);
            indices[index    ] = i * particleCount + j;
            indices[index + 1] = i * particleCount + j + 1;
            indices[index + 2] = (i + 1) * particleCount + j;
            indices[index + 3] = i * particleCount + j + 1;
            indices[index + 4] = (i + 1) * particleCount + j;
            indices[index + 5] = (i + 1) * particleCount + j + 1;
        }
    }
    
    //generate primitive data
    for (int i = 0; i < 2 * (particleCount - 1) * (particleCount - 1); i++) {
        primitiveData[i] = PrimitiveData{{0, 0, -1}, {0, 0, -1}, {0, 0, -1}};
    }

    //generate particle data
    for (int i = 0; i < particleCount; i++) {
        for (int j = 0; j < particleCount; j++) {
            particleData[i * particleCount + j] = {
                .alive = i > 0,
                .position = simd::float3{
                    size * (j - (particleCount - 1.0f) / 2) / (particleCount - 1),
                    1 + size * (1 - (float)i / (particleCount - 1)),
                    0.01f * (float)rand() / RAND_MAX
                },
                .velocity = simd::float3{},
                .acceleration = simd::float3{}
            };
        }
    }

    float particleMass = size * size * unitMass / particleCount / particleCount;
    float sideSpringLength = size / (particleCount - 1);
    float diagonalSpringLength = sqrt(2) * sideSpringLength;

    MTL::FunctionConstantValues *pFunctionConstants = MTL::FunctionConstantValues::alloc()->init();
    pFunctionConstants->setConstantValue(&particleCount, MTL::DataTypeUInt, NS::UInteger(0));
    pFunctionConstants->setConstantValue(&particleMass, MTL::DataTypeFloat, NS::UInteger(1));
    pFunctionConstants->setConstantValue(&springConstant, MTL::DataTypeFloat, NS::UInteger(2));
    pFunctionConstants->setConstantValue(&dampingConstant, MTL::DataTypeFloat, NS::UInteger(3));
    pFunctionConstants->setConstantValue(&sideSpringLength, MTL::DataTypeFloat, NS::UInteger(4));
    pFunctionConstants->setConstantValue(&diagonalSpringLength, MTL::DataTypeFloat, NS::UInteger(5));

    MTL::Library *pLibrary = pDevice->newLibrary(NS::String::string("Simulation.metallib", NS::UTF8StringEncoding), &err);
    MTL::Function *pClothFunction = pLibrary->newFunction(NS::String::string("simulateClothKernel", NS::UTF8StringEncoding), pFunctionConstants, &err);

    this->_pComputeClothPipelineState = pDevice->newComputePipelineState(pClothFunction, &err);
    unsigned int clothGroupWidth = this->_pComputeClothPipelineState->threadExecutionWidth();
    unsigned int clothGroupHeight = this->_pComputeClothPipelineState->maxTotalThreadsPerThreadgroup() / clothGroupWidth;
    this->_clothTPG = MTL::Size::Make((particleCount + clothGroupWidth - 1) / clothGroupWidth, (particleCount + clothGroupHeight - 1) / clothGroupHeight, 1);
    this->_clothTPT = MTL::Size::Make(clothGroupWidth, clothGroupHeight, 1);
    
    this->_pVertexBuffer = pDevice->newBuffer(3 * particleCount * particleCount * sizeof(float), MTL::ResourceStorageModeManaged);
    MTL::Buffer *pIndexBuffer = pDevice->newBuffer(6 * (particleCount - 1) * (particleCount - 1) * sizeof(unsigned int), MTL::ResourceStorageModeManaged);
    this->_pDataBuffer = pDevice->newBuffer(2 * (particleCount - 1) * (particleCount - 1) * sizeof(PrimitiveData), MTL::ResourceStorageModeManaged);
    this->_pParticleBuffer = pDevice->newBuffer(particleCount * particleCount * sizeof(Particle), MTL::ResourceStorageModeManaged);
    memcpy(this->_pVertexBuffer->contents(), vertices, this->_pVertexBuffer->length());
    memcpy(pIndexBuffer->contents(), indices, pIndexBuffer->length());
    memcpy(this->_pDataBuffer->contents(), primitiveData, this->_pDataBuffer->length());
    memcpy(this->_pParticleBuffer->contents(), particleData, this->_pParticleBuffer->length());
    this->_pVertexBuffer->didModifyRange(NS::Range::Make(0, this->_pVertexBuffer->length()));
    pIndexBuffer->didModifyRange(NS::Range::Make(0, pIndexBuffer->length()));
    this->_pDataBuffer->didModifyRange(NS::Range::Make(0, this->_pDataBuffer->length()));
    this->_pParticleBuffer->didModifyRange(NS::Range::Make(0, this->_pParticleBuffer->length()));

    this->_pDescriptor = this->_pTriangleDescriptor = MTL::AccelerationStructureTriangleGeometryDescriptor::alloc()->init();
    this->_pTriangleDescriptor->setTriangleCount(2 * (particleCount - 1) * (particleCount - 1));
    this->_pTriangleDescriptor->setVertexBuffer(this->_pVertexBuffer);
    this->_pTriangleDescriptor->setIndexBuffer(pIndexBuffer);
    this->_pTriangleDescriptor->setPrimitiveDataBuffer(this->_pDataBuffer);
    this->_pTriangleDescriptor->setPrimitiveDataStride(sizeof(PrimitiveData));
    this->_pTriangleDescriptor->setPrimitiveDataElementSize(sizeof(PrimitiveData));

    pFunctionConstants->release();
    pLibrary->release();
    pIndexBuffer->release();
}

Cloth::~Cloth() {
    this->_pTriangleDescriptor->release();
    this->_pVertexBuffer->release();
    this->_pDataBuffer->release();
    this->_pParticleBuffer->release();
}

void Cloth::update(MTL::CommandBuffer *pCmd, MTL::AccelerationStructure *pAccelerationStructure, float dt) {
    unsigned int iterations = dt / TARGET_TIMESTEP;
    printf("dt: %f iterations: %d\n", dt, iterations);
    float fdt = dt / iterations;
    MTL::ComputeCommandEncoder *pCEnc = pCmd->computeCommandEncoder();
    pCEnc->setBytes(&fdt, sizeof(float), 0);
    pCEnc->setAccelerationStructure(pAccelerationStructure, 1);
    pCEnc->setBuffer(this->_pParticleBuffer, 0, 2);
    pCEnc->setBuffer(this->_pVertexBuffer, 0, 3);
    pCEnc->setComputePipelineState(this->_pComputeClothPipelineState);
    for (int i = 0; i < iterations; i++)
        pCEnc->dispatchThreadgroups(this->_clothTPG, this->_clothTPT);
    pCEnc->endEncoding();
}

void Cloth::updateGeometry() {
    this->_pTriangleDescriptor->setVertexBuffer(this->_pVertexBuffer);
}