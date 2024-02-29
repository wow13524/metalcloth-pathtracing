#include <metal_math>
#include <metal_stdlib>
#include "SharedTypes.h"

using namespace metal;

float2 worldToUv(constant float4x4 &vpMat, packed_float3 v) {
    float4 clipCoord = vpMat * float4(v, 1);
    return (clipCoord.xy / -clipCoord.w + 1) / 2;
}

kernel void motionVectorKernel(
    uint position                           [[thread_position_in_grid]],
    constant uint &triangleCount            [[buffer(0)]],
    constant packed_float3 *vertexBuffer    [[buffer(1)]],
    constant uint *indexBuffer              [[buffer(2)]],
    device PrimitiveData *dataBuffer        [[buffer(3)]],
    constant float4x4 &vpMat                [[buffer(4)]]
) {
    if (position >= triangleCount) return;
    device PrimitiveData &data = dataBuffer[position];
    data.v0PrevUV = data.v0CurrUV;
    data.v1PrevUV = data.v1CurrUV;
    data.v2PrevUV = data.v2CurrUV;
    data.v0CurrUV = worldToUv(vpMat, vertexBuffer[indexBuffer[3 * position]]);
    data.v1CurrUV = worldToUv(vpMat, vertexBuffer[indexBuffer[3 * position + 1]]);
    data.v2CurrUV = worldToUv(vpMat, vertexBuffer[indexBuffer[3 * position + 2]]);
}