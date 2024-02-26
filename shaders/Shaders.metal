#include <metal_math>
#include <metal_stdlib>
#include "SharedTypes.h"

using namespace metal;

constant uint width [[function_constant(0)]];
constant uint height [[function_constant(1)]];
constant float fovh [[function_constant(2)]];
constant float fovv [[function_constant(3)]];

kernel void renderMain(
    uint2 position [[thread_position_in_grid]],
    raytracing::primitive_acceleration_structure accelerationStructure [[buffer(0)]],
    constant Camera &camera[[buffer(1)]],
    constant uint16_t *geometryMaterials [[buffer(2)]],
    constant Material *materials [[buffer(3)]],
    texture2d<float, access::write> output [[texture(0)]]
) {
    if (position.x < width && position.y < height) {
        float2 uv = ((float2)position / float2(width, height) - 0.5f) * 2;

        raytracing::ray ray{camera.position, normalize(tan(uv.x * fovh) * camera.right + tan(uv.y * fovv) * camera.up + camera.forward)};
        raytracing::intersector<> primitiveIntersector;
        raytracing::intersection_result<> intersection = primitiveIntersector.intersect(ray, accelerationStructure);

        bool hit = intersection.type != raytracing::intersection_type::none;
        Material mat = materials[geometryMaterials[intersection.geometry_id]];

        output.write(float4(1 / intersection.distance / intersection.distance * hit * (float3)mat.color, 1), position);
    }
}

constant float2 screenQuadVerts[6] = {
    {-1, -1}, {-1, 1}, {1, 1},
    {-1, -1}, {1, 1}, {1, -1}
};

typedef struct VertexShaderOut {
    float4 position [[position]];
    float2 uv;
} VertexShaderOut;

vertex VertexShaderOut vertexMain(
    unsigned short vertexId [[vertex_id]]
) {
    constant float2 &position = screenQuadVerts[vertexId];
    return {
        .position = float4(position, 0, 1),
        .uv = position * 0.5f + 0.5f
    };
}

fragment float4 fragmentMain(
    VertexShaderOut in [[stage_in]],
    texture2d<float, access::sample> output [[texture(0)]]
) {
    constexpr sampler sam(min_filter::nearest, mag_filter::nearest, mip_filter::none);
    float3 color = output.sample(sam, in.uv).xyz;
    return float4(color, 1.0f);
}