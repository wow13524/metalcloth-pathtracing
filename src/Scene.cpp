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

void Scene::updateGeometry() {
    const NS::Object *pGeometryDescriptors[this->_sceneObjects.size()];
    for (int i = 0; i < this->_sceneObjects.size(); i++) {
        pGeometryDescriptors[i] = _sceneObjects.at(i)->getDescriptor();
    }

    this->_pDescriptor->setGeometryDescriptors(NS::Array::alloc()->init(pGeometryDescriptors, this->_sceneObjects.size()));
}

void Scene::update(MTL::CommandBuffer *pCmd, MTL::AccelerationStructure *pAccelerationStructure, float dt) {
    for (SceneObject *pSceneObject: this->_sceneObjects) {
        pSceneObject->update(pCmd, pAccelerationStructure, dt);
    }
}