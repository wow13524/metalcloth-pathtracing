#include "Scene.hpp"

Scene::~Scene() {
    for (SceneObject *pSceneObject: this->_sceneObjects) {
        delete pSceneObject;
    }
    this->_pDescriptor->release();
}

void Scene::addObject(SceneObject *pSceneObject) {
    this->_sceneObjects.push_back(pSceneObject);
    this->updateGeometry();
}

void Scene::loadHdri(MTL::Device *pDevice, const char *fileName) {
    this->_pHdri = new Hdri(pDevice, fileName);
}

Hdri* Scene::getHdri() {
    return this->_pHdri;
}

void Scene::update(MTL::CommandBuffer *pCmd, MTL::AccelerationStructure *pAccelerationStructure, float dt, simd::float3 moveDirection, bool enable) {
    for (SceneObject *pSceneObject: this->_sceneObjects) {
        pSceneObject->update(pCmd, pAccelerationStructure, dt, moveDirection, enable);
    }
}

void Scene::updateGeometry() {
    const NS::Object *pGeometryDescriptors[this->_sceneObjects.size()];
    for (int i = 0; i < this->_sceneObjects.size(); i++) {
        pGeometryDescriptors[i] = _sceneObjects.at(i)->getDescriptor();
    }

    this->_pDescriptor->setGeometryDescriptors(NS::Array::alloc()->init(pGeometryDescriptors, this->_sceneObjects.size()));
}

void Scene::updatePrimitiveMotion(MTL::ComputePipelineState *pComputeMotionPipelineState, MTL::CommandBuffer *pCmd, simd::float4x4 vpMat) {
    MTL::ComputeCommandEncoder *pCEnc = pCmd->computeCommandEncoder();
    pCEnc->setComputePipelineState(pComputeMotionPipelineState);
    for (SceneObject *sceneObject: this->_sceneObjects) {
        MTL::AccelerationStructureTriangleGeometryDescriptor *pDescriptor = sceneObject->getDescriptor();
        unsigned int triangleCount = pDescriptor->primitiveDataBuffer()->length() / sizeof(PrimitiveData);
        pCEnc->setBytes(&triangleCount, sizeof(unsigned int), 0);
        pCEnc->setBuffer(pDescriptor->vertexBuffer(), 0, 1);
        pCEnc->setBuffer(pDescriptor->indexBuffer(), 0, 2);
        pCEnc->setBuffer(pDescriptor->primitiveDataBuffer(), 0, 3);
        pCEnc->setBytes(&vpMat, sizeof(simd::float4x4), 4);
        
        unsigned int motionGroupWidth = pComputeMotionPipelineState->threadExecutionWidth();
        pCEnc->dispatchThreadgroups(
            MTL::Size::Make((triangleCount + motionGroupWidth - 1) / motionGroupWidth, 1, 1),
            MTL::Size::Make(motionGroupWidth, 1, 1)
        );
    }
    pCEnc->endEncoding();
}