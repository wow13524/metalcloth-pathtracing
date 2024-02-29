#include "SceneObject.hpp"

SceneObject::SceneObject() {
    this->_pDescriptor = MTL::AccelerationStructureTriangleGeometryDescriptor::alloc()->init();
}

SceneObject::~SceneObject() {
    this->_pDescriptor->release();
}