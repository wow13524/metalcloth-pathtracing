#pragma once

#include <simd/simd.h>

#define EPSILON 0.0001f

enum RayState {
    RAY_DEAD,
    RAY_PRIMARY,
    RAY_ALIVE
};

typedef struct pfloat3 {
    float x, y, z;
} pfloat3;

typedef struct Material {
    simd::float3 color;
    float roughness;
} Material;

typedef struct Particle {
    bool alive;
    simd::float3 normal, position, velocity, acceleration;
} Particle;

typedef struct PrimitiveData {
    simd::float2 v0PrevUV, v1PrevUV, v2PrevUV, v0CurrUV, v1CurrUV, v2CurrUV;
    simd::float3 v0Normal, v1Normal, v2Normal;
} PrimitiveData;