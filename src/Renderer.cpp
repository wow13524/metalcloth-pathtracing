#include "Renderer.hpp"

Renderer::Renderer(MTL::Device *pDevice, MTK::View *pView) {
    unsigned int width = pView->drawableSize().width, height = pView->drawableSize().height;
    float aspectRatio = (float)width / height;
    NS::Error *err = nullptr;
    
    this->_pDevice = pDevice->retain();
    this->_pCommandQueue = this->_pDevice->newCommandQueue();
    
    MTL::FunctionConstantValues *pFunctionConstants = MTL::FunctionConstantValues::alloc()->init();
    pFunctionConstants->setConstantValue(&width, MTL::DataTypeUInt, NS::UInteger(0));
    pFunctionConstants->setConstantValue(&height, MTL::DataTypeUInt, NS::UInteger(1));
    pFunctionConstants->setConstantValue(&SPP, MTL::DataTypeUInt, NS::UInteger(2));
    pFunctionConstants->setConstantValue(&aspectRatio, MTL::DataTypeFloat, NS::UInteger(3));
    pFunctionConstants->setConstantValue(&FOV, MTL::DataTypeFloat, NS::UInteger(4));

    MTL::Library *pDenoisingLibrary = this->_pDevice->newLibrary(NS::String::string("Denoising.metallib", NS::UTF8StringEncoding), &err);
    MTL::Library *pShaderLibrary = this->_pDevice->newLibrary(NS::String::string("Shaders.metallib", NS::UTF8StringEncoding), &err);
    MTL::Function *pResetFunction = pShaderLibrary->newFunction(NS::String::string("resetImageKernel", NS::UTF8StringEncoding));
    MTL::Function *pMotionFunction = pDenoisingLibrary->newFunction(NS::String::string("motionVectorKernel", NS::UTF8StringEncoding), pFunctionConstants, &err);
    MTL::Function *pRayFunction = pShaderLibrary->newFunction(NS::String::string("generateRayKernel", NS::UTF8StringEncoding), pFunctionConstants, &err);
    MTL::Function *pSceneFunction = pShaderLibrary->newFunction(NS::String::string("sampleSceneKernel", NS::UTF8StringEncoding), pFunctionConstants, &err);
    MTL::Function *pVertexFunction = pShaderLibrary->newFunction(NS::String::string("vertexMain", NS::UTF8StringEncoding));
    MTL::Function *pFragmentFunction = pShaderLibrary->newFunction(NS::String::string("fragmentMain", NS::UTF8StringEncoding));

    this->_pComputeMotionPipelineState = this->_pDevice->newComputePipelineState(pMotionFunction, &err);

    this->_pComputeResetPipelineState = this->_pDevice->newComputePipelineState(pResetFunction, &err);
    unsigned int resetGroupWidth = this->_pComputeResetPipelineState->threadExecutionWidth();
    unsigned int resetGroupHeight = this->_pComputeResetPipelineState->maxTotalThreadsPerThreadgroup() / resetGroupWidth;
    this->_resetTPG = MTL::Size::Make((width + resetGroupWidth - 1) / resetGroupWidth, (height + resetGroupHeight - 1) / resetGroupHeight, 1);
    this->_resetTPT = MTL::Size::Make(resetGroupWidth, resetGroupHeight, 1);

    this->_pComputeRayPipelineState = this->_pDevice->newComputePipelineState(pRayFunction, &err);
    unsigned int rayGroupWidth = this->_pComputeRayPipelineState->threadExecutionWidth();
    unsigned int rayGroupHeight = this->_pComputeRayPipelineState->maxTotalThreadsPerThreadgroup() / rayGroupWidth;
    this->_rayTPG = MTL::Size::Make((width + rayGroupWidth - 1) / rayGroupWidth, (height + rayGroupHeight - 1) / rayGroupHeight, 1);
    this->_rayTPT = MTL::Size::Make(rayGroupWidth, rayGroupHeight, 1);

    this->_pComputeScenePipelineState = this->_pDevice->newComputePipelineState(pSceneFunction, &err);
    unsigned int sceneGroupWidth = this->_pComputeScenePipelineState->threadExecutionWidth();
    unsigned int sceneGroupHeight = this->_pComputeScenePipelineState->maxTotalThreadsPerThreadgroup() / sceneGroupWidth;
    this->_sceneTPG = MTL::Size::Make((width + sceneGroupWidth - 1) / sceneGroupWidth, (height + sceneGroupHeight - 1) / sceneGroupHeight, 1);
    this->_sceneTPT = MTL::Size::Make(sceneGroupWidth, sceneGroupHeight, 1);

    MTL::RenderPipelineDescriptor *pRenderPipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    pRenderPipelineDescriptor->setVertexFunction(pVertexFunction);
    pRenderPipelineDescriptor->setFragmentFunction(pFragmentFunction);
    pRenderPipelineDescriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm_sRGB);

    this->_pDenoiser = SVGFDenoiser::alloc()->init(pDevice);

    this->_pRenderPipelineState = this->_pDevice->newRenderPipelineState(pRenderPipelineDescriptor, &err);
    this->_pDepthNormalTextures[0] = this->_pDevice->newTexture(MTL::TextureDescriptor::texture2DDescriptor(
        MTL::PixelFormatRGBA32Float,
        width,
        height,
        false
    ));
    this->_pDepthNormalTextures[1] = this->_pDevice->newTexture(MTL::TextureDescriptor::texture2DDescriptor(
        MTL::PixelFormatRGBA32Float,
        width,
        height,
        false
    ));

    this->_pMotionTexture = this->_pDevice->newTexture(MTL::TextureDescriptor::texture2DDescriptor(
        MTL::PixelFormatRGBA32Float,
        width,
        height,
        false
    ));

    this->_pOutputTexture = this->_pDevice->newTexture(MTL::TextureDescriptor::texture2DDescriptor(
        MTL::PixelFormatRGBA32Float,
        width,
        height,
        false
    ));
    
    this->_pRayBuffer = this->_pDevice->newBuffer(width * height * sizeof(Ray), MTL::ResourceStorageModePrivate);
    this->_pCameraBuffer = this->_pDevice->newBuffer(sizeof(Camera), MTL::ResourceStorageModeManaged);

    this->_projectionMatrix = simd::float4x4{
        simd::float4{1 / tan(FOV), 0, 0, 0},
        simd::float4{0, aspectRatio / tan(FOV), 0, 0},
        simd::float4{0, 0, 1 ,-1},
        simd::float4{0, 0, 2, 0}
    };
    
    this->loadScene(new TestScene(this->_pDevice));

    pFunctionConstants->release();
    pDenoisingLibrary->release();
    pShaderLibrary->release();
    pMotionFunction->release();
    pRayFunction->release();
    pSceneFunction->release();
    pVertexFunction->release();
    pFragmentFunction->release();
    pRenderPipelineDescriptor->release();
}

Renderer::~Renderer() {
    this->_pDevice->release();
    this->_pCommandQueue->release();
    this->_pComputeMotionPipelineState->release();
    this->_pComputeResetPipelineState->release();
    this->_pComputeRayPipelineState->release();
    this->_pComputeScenePipelineState->release();
    this->_pRenderPipelineState->release();
    this->_pDenoiser->release();
    this->_pDepthNormalTextures[0]->release();
    this->_pDepthNormalTextures[1]->release();
    this->_pMotionTexture->release();
    this->_pOutputTexture->release();
    this->_pRayBuffer->release();
    this->_pGeometryMaterialBuffer->release();
    this->_pMaterialBuffer->release();
    this->_pCameraBuffer->release();
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

    this->_pGeometryMaterialBuffer = this->_pDevice->newBuffer(geometryMaterials.size() * sizeof(uint16_t), MTL::ResourceStorageModeManaged);
    this->_pMaterialBuffer = this->_pDevice->newBuffer(materials.size() * sizeof(Material), MTL::ResourceStorageModeManaged);
    this->_pScratchBuffer = this->_pDevice->newBuffer(sizes.buildScratchBufferSize, MTL::ResourceStorageModePrivate);
    this->_pAccelerationStructure = this->_pDevice->newAccelerationStructure(sizes.accelerationStructureSize);

    memcpy(this->_pCameraBuffer->contents(), &this->_camera, this->_pCameraBuffer->length());
    memcpy(this->_pGeometryMaterialBuffer->contents(), geometryMaterials.data(), this->_pGeometryMaterialBuffer->length());
    memcpy(this->_pMaterialBuffer->contents(), materials.data(), this->_pMaterialBuffer->length());
    this->_pCameraBuffer->didModifyRange(NS::Range::Make(0, this->_pCameraBuffer->length()));
    this->_pGeometryMaterialBuffer->didModifyRange(NS::Range::Make(0, this->_pGeometryMaterialBuffer->length()));
    this->_pMaterialBuffer->didModifyRange(NS::Range::Make(0, this->_pMaterialBuffer->length()));
}

void Renderer::draw(MTK::View *pView) {
    auto thisFrame = std::chrono::system_clock::now();
    float dt = (thisFrame - this->_lastFrame).count() / 1e6;
    printf("FPS: %f\n", 1 / dt);
    this->_lastFrame = thisFrame;

    NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();

    //Simulate scene and update geometry
    MTL::CommandBuffer *pCmd = this->_pCommandQueue->commandBuffer();
    this->_pScene->update(pCmd, this->_pAccelerationStructure, dt);
    pCmd->commit();
    pCmd->waitUntilCompleted();
    this->_pScene->updateGeometry();

    simd::float4x4 viewMat = simd_inverse(simd::float4x4{
        simd::float4{this->_camera.right[0], this->_camera.right[1], this->_camera.right[2], 0},
        simd::float4{this->_camera.up[0], this->_camera.up[1], this->_camera.up[2], 0},
        simd::float4{this->_camera.forward[0], this->_camera.forward[1], this->_camera.forward[2], 0},
        simd::float4{this->_camera.position[0], this->_camera.position[1], this->_camera.position[2], 1}
    });
    simd::float4x4 pvMat = this->_projectionMatrix * viewMat;
    simd::float4x4 pvMatInv = simd_inverse(pvMat);

    pCmd = this->_pCommandQueue->commandBuffer();

    //update primitive motion data
    this->_pScene->updatePrimitiveMotion(this->_pComputeMotionPipelineState, pCmd, pvMat);

    //rebuild acceleration structure
    MTL::AccelerationStructureCommandEncoder *pASEnc = pCmd->accelerationStructureCommandEncoder();
    pASEnc->buildAccelerationStructure(
        this->_pAccelerationStructure,
        this->_pScene->getDescriptor(),
        this->_pScratchBuffer,
        0
    );
    pASEnc->endEncoding();

    //reset output texture
    MTL::ComputeCommandEncoder *pCEnc = pCmd->computeCommandEncoder();
    pCEnc->setAccelerationStructure(this->_pAccelerationStructure, 1);
    pCEnc->setBuffer(this->_pRayBuffer, 0, 2);
    pCEnc->setBuffer(this->_pGeometryMaterialBuffer, 0, 3);
    pCEnc->setBuffer(this->_pMaterialBuffer, 0, 4);
    pCEnc->setBuffer(this->_pCameraBuffer, 0, 5);
    pCEnc->setBytes(&pvMatInv, sizeof(simd::float4x4), 6);
    pCEnc->setTexture(this->_pDepthNormalTextures[0], 0);
    pCEnc->setTexture(this->_pMotionTexture, 1);
    pCEnc->setTexture(this->_pOutputTexture, 2);
    pCEnc->setComputePipelineState(this->_pComputeResetPipelineState);
    pCEnc->dispatchThreadgroups(this->_resetTPG, this->_resetTPT);
    //generate primary rays
    uint32_t r;
    for (int i = 0; i < SPP; i++) {
        r = rand();
        pCEnc->setBytes(&r, sizeof(uint32_t), 0);
        pCEnc->setComputePipelineState(this->_pComputeRayPipelineState);
        pCEnc->dispatchThreadgroups(this->_rayTPG, this->_rayTPT);
        //sample scene
        pCEnc->setComputePipelineState(this->_pComputeScenePipelineState);
        for (int j = 0; j < BOUNCES; j++) {
            pCEnc->setBytes(&r, sizeof(uint32_t), 0);
            pCEnc->dispatchThreadgroups(this->_sceneTPG, this->_sceneTPT);
        }
    }
    pCEnc->endEncoding();

    //denoise
    MTL::Texture* denoisedOutput = this->_pDenoiser->encodeToCommandBuffer(
        pCmd,
        this->_pOutputTexture,
        this->_pMotionTexture,
        this->_pDepthNormalTextures[0],
        this->_pDepthNormalTextures[1]
    );

    //draw texture to screen
    MTL::RenderPassDescriptor *pRpd = pView->currentRenderPassDescriptor();
    MTL::RenderCommandEncoder *pREnc = pCmd->renderCommandEncoder(pRpd);
    pREnc->setRenderPipelineState(this->_pRenderPipelineState);
    pREnc->setFragmentTexture(denoisedOutput, NS::UInteger(0));
    pREnc->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0), NS::UInteger(6));
    pREnc->endEncoding();

    pCmd->presentDrawable(pView->currentDrawable());

    pCmd->commit();

    std::swap(this->_pDepthNormalTextures[0], this->_pDepthNormalTextures[1]);

    pPool->release();
}