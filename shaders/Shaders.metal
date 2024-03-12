#include <metal_math>
#include <metal_stdlib>
#include "SharedTypes.h"

using namespace metal;

constant uint width         [[function_constant(0)]];
constant uint height        [[function_constant(1)]];
constant uint spp           [[function_constant(2)]];
constant float aspectRatio  [[function_constant(3)]];
constant float fov          [[function_constant(4)]];

constant float2 screenQuadVerts[6] = {
    {-1, -1}, {-1, 1}, {1, 1},
    {-1, -1}, {1, 1}, {1, -1}
};

typedef struct VertexShaderOut {
    float4 position [[position]];
    float2 uv;
} VertexShaderOut;

float randUnif(uint2 position, uint n) {
    uint seed = position.x + position.y * 57 + n * 241;
    seed = (seed << 13) ^ seed;
    return (( 1.f - ( (seed * (seed * seed * 15731 + 789221) + 1376312589) & 2147483647) / 1073741824.0f) + 1.0f) / 2.0f;
}

//assume normal is UP(Y)
float schlickFresnel(float3 incidence, float ior1, float ior2) {
    float oneMinusCosTheta = 1 - incidence.y;
    float oneMinusCosThetaSquared = oneMinusCosTheta * oneMinusCosTheta;
    float r0 = (ior1 - ior2) / (ior1 + ior2);
    r0 = r0 * r0;
    return r0 + (1 - r0) * oneMinusCosThetaSquared * oneMinusCosThetaSquared * oneMinusCosTheta;
}

//assume normal is UP(Y)
float smithG1(float3 viewOut, float a2) {
    return 2 * viewOut.y / (sqrt(a2 + (1 - a2) * viewOut.y * viewOut.y) + viewOut.y);
}

//assume normal is UP(Y)
float smithG2(float3 lightIn, float3 viewOut, float a2) {
    return 2 * lightIn.y * viewOut.y / (viewOut.y * sqrt(a2 + (1 - a2) * lightIn.y * lightIn.y) + lightIn.y * sqrt(a2 + (1 - a2) * viewOut.y * viewOut.y));
}

//adapted from https://jcgt.org/published/0007/04/01/paper.pdf
//assume normal is UP(Y)
float3 sampleGgxVndf(float3 viewOut, float roughness, float rand1, float rand2) {
    float3 viewHemisphere = normalize(float3(roughness * viewOut.x, viewOut.y, roughness * viewOut.z));
    float lengthSquared = length_squared(viewHemisphere.xz);
    float3 b1 = lengthSquared > EPSILON ? float3(-viewHemisphere.z, 0, viewHemisphere.x) * rsqrt(lengthSquared) : float3(1, 0, 0);
    float3 b2 = cross(viewHemisphere, b1);
    float r = sqrt(rand1);
    float phi = 2 * M_PI_F * rand2;
    float t1 = r * cos(phi);
    float t2 = r * sin(phi);
    float s = 0.5f * (1 + viewHemisphere.y);
    t2 = (1 - s) * sqrt(1 - t1 * t1) + s * t2;
    float3 normHemisphere = t1 * b1 + t2 * b2 + sqrt(max(0.0f, 1 - t1 * t1 - t2 * t2)) * viewHemisphere;
    return normalize(float3(roughness * normHemisphere.x, max(0.0f, normHemisphere.y), roughness * normHemisphere.z));
}

float4 importanceSampleGgxVndf(uint2 position, uint32_t rand, float3 normal, float3 direction, float roughness) {
    float a2 = roughness * roughness;
    float projY = dot(normal, -direction);
    float3 b1 = normalize(cross(normal, -direction));
    float3 b2 = cross(b1, normal);
    float3 viewOut = float3(sqrt(1 - projY * projY), projY, 0);
    float3 ggxNorm = sampleGgxVndf(viewOut, roughness, randUnif(position, rand), randUnif(position, rand + 1));
    float3 lightIn = -reflect(viewOut, ggxNorm);
    return float4(lightIn.x * b2 + lightIn.y * normal + lightIn.z * b1, lightIn.y > 0 ? schlickFresnel(-viewOut, 1, 1) * smithG2(lightIn, viewOut, a2) / smithG1(viewOut, a2) : 0);
}

float3 sampleHdri(texture2d<float, access::sample> hdri, float3 direction) {
    float2 xz = normalize(direction.xz);
    float2 uv = float2(atan2(xz.y, xz.x) / 2, asin(direction.y)) / M_PI_F + 0.5;
    constexpr sampler sam(min_filter::linear, mag_filter::linear, mip_filter::none);
    return hdri.sample(sam, uv).xyz;
}

float3 acesTonemap(float3 x) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

kernel void resetImageKernel(
    uint2 position                                                      [[thread_position_in_grid]],
    texture2d<float, access::write> output                              [[texture(2)]]
) {
    output.write(float4(), position);
}

kernel void finalizeImageKernel(
    uint2 position                                                      [[thread_position_in_grid]],
    texture2d<float, access::read_write> output                              [[texture(2)]]
) {
    output.write(float4(acesTonemap(output.read(position).xyz / spp), 1), position);
}

kernel void sampleSceneKernel(
    uint2 position                                                      [[thread_position_in_grid]],
    constant uint32_t &rand                                             [[buffer(0)]],
    raytracing::primitive_acceleration_structure accelerationStructure  [[buffer(1)]],
    constant uint16_t *geometryMaterials                                [[buffer(2)]],
    constant Material *materials                                        [[buffer(3)]],
    constant float3 &origin                                             [[buffer(4)]],
    constant float4x4 &pvMatInv                                         [[buffer(5)]],
    texture2d<float, access::write> depthNormal                         [[texture(0)]],
    texture2d<float, access::read_write> motion                         [[texture(1)]],
    texture2d<float, access::read_write> output                         [[texture(2)]],
    texture2d<float, access::sample> hdri                               [[texture(3)]]
) {
    if (position.x >= width || position.y >= height) return;
    float2 normalizedCoords = 2 * (float2)position / float2(width, height) - 1;
    float3 direction = normalize((pvMatInv * float4(normalizedCoords, 1, -1)).xyz);
    raytracing::ray ray{origin, direction, EPSILON};
    float3 color = 1;
    
    raytracing::intersector<raytracing::triangle_data> primitiveIntersector;

    for (uint i = 0; i < 8; i++) {
        raytracing::intersection_result<raytracing::triangle_data> intersection = primitiveIntersector.intersect(ray, accelerationStructure);

        bool hit = intersection.type != raytracing::intersection_type::none;
        Material mat = materials[geometryMaterials[intersection.geometry_id]];
        PrimitiveData data = *(const device PrimitiveData*)intersection.primitive_data;

        float2 bCoords = intersection.triangle_barycentric_coord;
        float2 currUv = bCoords.x * data.v1CurrUV + bCoords.y * data.v2CurrUV + (1 - bCoords.x - bCoords.y) * data.v0CurrUV;
        float2 prevUv = bCoords.x * data.v1PrevUV + bCoords.y * data.v2PrevUV + (1 - bCoords.x - bCoords.y) * data.v0PrevUV;
        float2 dUv = currUv - prevUv;
        float3 surfaceNormal = normalize(bCoords.x * data.v1Normal + bCoords.y * data.v2Normal + (1 - bCoords.x - bCoords.y) * data.v0Normal);
        float3 intersectedPosition = ray.origin + intersection.distance * ray.direction;
        surfaceNormal = faceforward(surfaceNormal, ray.direction, surfaceNormal);

        float4 ggxSample = importanceSampleGgxVndf(position, rand, surfaceNormal, ray.direction, mat.roughness);

        if (i == 0) {
            depthNormal.write(float4(1 - 1 / (hit ? intersection.distance : INFINITY), hit ? surfaceNormal : float3(0)), position);
            motion.write(0, position);
            if (length(dUv) > 0 || (length(dUv) == 0 && randUnif(position, rand) < 0.1)) {
                motion.write(1, position);
            }
        }

        color *= hit ? mat.color : sampleHdri(hdri, ray.direction);
        ray.origin = intersectedPosition;
        ray.direction = ggxSample.xyz;

        if (!hit) {
            output.write(output.read(position) + float4(color, 1), position);
            break;
        }
    }
}

vertex VertexShaderOut vertexMain(
    unsigned short vertexId [[vertex_id]]
) {
    float2 position = screenQuadVerts[vertexId];
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