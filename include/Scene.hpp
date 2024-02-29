#pragma once

#include "Metal.hpp"
#include "SceneObject.hpp"
#include "SharedTypes.h"

typedef struct Camera {
    float pitch, yaw;
    simd::float3 position;
} Camera;

class Scene {
    public:
        virtual ~Scene();

        inline MTL::AccelerationStructureDescriptor* getDescriptor() {return this->_pDescriptor;};
        virtual Camera getInitialCamera() = 0;
        virtual std::vector<uint16_t> getGeometryMaterials() = 0;
        virtual std::vector<Material> getMaterials() = 0;
        void update(MTL::CommandBuffer *pCmd, MTL::AccelerationStructure *pAccelerationStructure, float dt);
        void updateGeometry();
        void updatePrimitiveMotion(MTL::ComputePipelineState *pComputeMotionPipelineState, MTL::CommandBuffer *pCmd, simd::float4x4 vpMat);
    protected:
        MTL::PrimitiveAccelerationStructureDescriptor *_pDescriptor;

        void addObject(SceneObject *pSceneObject);
    private:
        std::vector<SceneObject*> _sceneObjects; 
};