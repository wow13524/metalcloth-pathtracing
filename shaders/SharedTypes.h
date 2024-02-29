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

typedef struct Camera {
    Camera(simd::float3 right = {1, 0, 0}, simd::float3 up = {0, 1, 0}, simd::float3 forward = {0, 0, 1}, simd::float3 position = {0, 0, 0}): right(right), up(up), forward(forward), position(position) {};
    simd::float3 right, up, forward, position;
} Camera;

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

typedef struct Ray {
    RayState state;
    simd::float3 origin, direction, color;
} Ray;