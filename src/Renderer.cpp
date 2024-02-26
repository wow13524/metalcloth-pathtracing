#include "Renderer.hpp"

Renderer::Renderer(MTL::Device *pDevice, MTK::View *pView) {
    unsigned int width = pView->drawableSize().width, height = pView->drawableSize().height;
    float fovh = FOV, fovv = FOV * height / width;
    NS::Error *err = nullptr;
    
    this->_pDevice = pDevice->retain();
    this->_pCommandQueue = this->_pDevice->newCommandQueue();
    
    MTL::FunctionConstantValues *pFunctionConstants = MTL::FunctionConstantValues::alloc()->init();
    pFunctionConstants->setConstantValue(&width, MTL::DataTypeUInt, NS::UInteger(0));
    pFunctionConstants->setConstantValue(&height, MTL::DataTypeUInt, NS::UInteger(1));
    pFunctionConstants->setConstantValue(&fovh, MTL::DataTypeFloat, NS::UInteger(2));
    pFunctionConstants->setConstantValue(&fovv, MTL::DataTypeFloat, NS::UInteger(3));

    MTL::Library *pLibrary = this->_pDevice->newLibrary(NS::String::string("Shaders.metallib", NS::UTF8StringEncoding), &err);
    MTL::Function *pRenderFunction = pLibrary->newFunction(NS::String::string("renderMain", NS::UTF8StringEncoding), pFunctionConstants, &err);
    MTL::Function *pVertexFunction = pLibrary->newFunction(NS::String::string("vertexMain", NS::UTF8StringEncoding));
    MTL::Function *pFragmentFunction = pLibrary->newFunction(NS::String::string("fragmentMain", NS::UTF8StringEncoding));

    this->_pComputePipelineState = this->_pDevice->newComputePipelineState(pRenderFunction, &err);
    unsigned int groupWidth = this->_pComputePipelineState->threadExecutionWidth();
    unsigned int groupHeight = this->_pComputePipelineState->maxTotalThreadsPerThreadgroup() / groupWidth;
    this->_threadgroupsPerGrid = MTL::Size::Make((width + groupWidth - 1) / groupWidth, (height + groupHeight - 1) / groupHeight, 1);
    this->_threadsPerThreadgroup = MTL::Size::Make(groupWidth, groupHeight, 1);

    MTL::RenderPipelineDescriptor *pRenderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    pRenderPipelineDescriptor->setVertexFunction(pVertexFunction);
    pRenderPipelineDescriptor->setFragmentFunction(pFragmentFunction);
    pRenderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm_sRGB);

    this->_pRenderPipelineState = this->_pDevice->newRenderPipelineState(pRenderPipelineDescriptor, &err);
    this->_pCameraBuffer = this->_pDevice->newBuffer(sizeof(Camera), MTL::ResourceStorageModeManaged);
    this->_pOutputTexture = this->_pDevice->newTexture(MTL::TextureDescriptor::texture2DDescriptor(
        MTL::PixelFormatRGBA32Float,
        width,
        height,
        false
    ));
    
    this->loadScene(new TestScene(this->_pDevice));

    pLibrary->release();
    pRenderFunction->release();
    pVertexFunction->release();
    pFragmentFunction->release();
    pRenderPipelineDescriptor->release();
}

Renderer::~Renderer() {
    this->_pDevice->release();
    this->_pCommandQueue->release();
    this->_pComputePipelineState->release();
    this->_pRenderPipelineState->release();
    this->_pOutputTexture->release();
    this->_pCameraBuffer->release();
    this->_pGeometryMaterialBuffer->release();
    this->_pMaterialBuffer->release();
    this->_pScratchBuffer->release();
    this->_pAccelerationStructure->release();
    delete this->_pScene;
}

void Renderer::loadScene(Scene *pScene) {
    if (this->_pScene != nullptr) {
        this->_pGeometryMaterialBuffer->release();
        this->_pMaterialBuffer->release();
        this->_pScratchBuffer->release();
        this->_pAccelerationStructure->release();
        delete this->_pScene;
    }

    this->_pScene = pScene;
    this->_camera = this->_pScene->getInitialCamera();

    std::vector<uint16_t> geometryMaterials = this->_pScene->getGeometryMaterials();
    std::vector<Material> materials = this->_pScene->getMaterials();
    MTL::AccelerationStructureSizes sizes = this->_pDevice->accelerationStructureSizes(this->_pScene->getDescriptor());

    this->_pGeometryMaterialBuffer = this->_pDevice->newBuffer(sizes.buildScratchBufferSize, MTL::ResourceStorageModeManaged);
    this->_pMaterialBuffer = this->_pDevice->newBuffer(sizes.buildScratchBufferSize, MTL::ResourceStorageModeManaged);
    this->_pScratchBuffer = this->_pDevice->newBuffer(sizes.buildScratchBufferSize, MTL::ResourceStorageModePrivate);
    this->_pAccelerationStructure = this->_pDevice->newAccelerationStructure(sizes.accelerationStructureSize);

    memcpy(this->_pCameraBuffer->contents(), &this->_camera, sizeof(Camera));
    memcpy(this->_pGeometryMaterialBuffer->contents(), geometryMaterials.data(), geometryMaterials.size() * sizeof(uint16_t));
    memcpy(this->_pMaterialBuffer->contents(), materials.data(), materials.size() * sizeof(Material));
    this->_pCameraBuffer->didModifyRange(NS::Range::Make(0, sizeof(Camera)));
    this->_pGeometryMaterialBuffer->didModifyRange(NS::Range::Make(0, geometryMaterials.size() * sizeof(uint16_t)));
    this->_pMaterialBuffer->didModifyRange(NS::Range::Make(0, materials.size() * sizeof(Material)));
}

void Renderer::draw(MTK::View *pView) {
    auto start = std::chrono::system_clock::now();

    NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();

    MTL::CommandBuffer *pCmd = this->_pCommandQueue->commandBuffer();

    this->_pScene->update(pCmd);

    //rebuild acceleration structure
    MTL::AccelerationStructureCommandEncoder *pASEnc = pCmd->accelerationStructureCommandEncoder();
    pASEnc->buildAccelerationStructure(
        this->_pAccelerationStructure,
        this->_pScene->getDescriptor(),
        this->_pScratchBuffer,
        0
    );
    pASEnc->endEncoding();

    //render output to texture
    MTL::ComputeCommandEncoder *pCEnc = pCmd->computeCommandEncoder();
    pCEnc->setComputePipelineState(this->_pComputePipelineState);
    pCEnc->setAccelerationStructure(this->_pAccelerationStructure, 0);
    pCEnc->setBuffer(this->_pCameraBuffer, 0, 1);
    pCEnc->setBuffer(this->_pGeometryMaterialBuffer, 0, 2);
    pCEnc->setBuffer(this->_pMaterialBuffer, 0, 3);
    pCEnc->setTexture(this->_pOutputTexture, 0);
    pCEnc->dispatchThreadgroups(this->_threadgroupsPerGrid, this->_threadsPerThreadgroup);
    pCEnc->endEncoding();

    //draw texture to screen
    MTL::RenderPassDescriptor *pRpd = pView->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder *pREnc = pCmd->renderCommandEncoder(pRpd);
    pREnc->setRenderPipelineState(this->_pRenderPipelineState);
    pREnc->setFragmentTexture(this->_pOutputTexture, NS::UInteger(0));
    pREnc->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(6));
    pREnc->endEncoding();

    pCmd->presentDrawable(pView->currentDrawable());

    pCmd->commit();

    pPool->release();

    auto stop = std::chrono::system_clock::now();
    printf("FPS: %f\n", 1e6 / (stop - start).count());
}