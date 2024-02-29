#include <metal_math>
#include <metal_stdlib>
#include "SharedTypes.h"

using namespace metal;

constant uint32_t particleCount [[function_constant(0)]];
constant float particleMass [[function_constant(1)]];
constant float springConstant [[function_constant(2)]];
constant float dampingConstant [[function_constant(3)]];
constant float sideSpringLength [[function_constant(4)]];
constant float diagonalSpringLength [[function_constant(5)]];

void applyGravity(device Particle &particle) {
    particle.acceleration += float3(0, -1, 0);
}

void applySpring(device Particle &particleA, device Particle &particleB, float springLength) {
    float3 displacement = particleB.position - particleA.position;
    float difference = springLength - length(displacement);
    float3 da = -springConstant * difference * normalize(displacement) / particleMass;
    particleA.acceleration += da / 2;
}

void applyDamper(device Particle &particleA, device Particle &particleB) {
    float3 direction = normalize(particleB.position - particleA.position);
    float3 da = -dampingConstant * dot(particleA.velocity - particleB.velocity, direction) * direction / particleMass;
    particleA.acceleration += da / 2;
}

void applySpringDamper(device Particle &particleA, device Particle &particleB, float springLength) {
    applySpring(particleA, particleB, springLength);
    applyDamper(particleA, particleB);
}

void applyClothSpringDampers(uint2 position, device Particle *particles, uint index, uint distance) {
    device Particle &particle = particles[index];
    if (position.x >= distance) applySpringDamper(particle, particles[index - distance], distance * sideSpringLength);
    if (position.y >= distance) applySpringDamper(particle, particles[index - distance * particleCount], distance * sideSpringLength);
    if (position.x < particleCount - distance) applySpringDamper(particle, particles[index + distance], distance * sideSpringLength);
    if (position.y < particleCount - distance) applySpringDamper(particle, particles[index + distance * particleCount], distance * sideSpringLength);
    if (position.x >= distance && position.y >= distance) applySpringDamper(particle, particles[index - distance * particleCount - distance], distance * diagonalSpringLength);
    if (position.x >= distance && position.y < particleCount - distance) applySpringDamper(particle, particles[index + distance * particleCount - distance], distance * diagonalSpringLength);
    if (position.x < particleCount - distance && position.y >= distance) applySpringDamper(particle, particles[index - distance * particleCount + distance], distance * diagonalSpringLength);
    if (position.x < particleCount - distance && position.y < particleCount - distance) applySpringDamper(particle, particles[index + distance * particleCount + distance], distance * diagonalSpringLength);
}

void applyDrag(device Particle &particleA, device Particle &particleB, device Particle &particleC) {
    float3 surfaceVelocity = (particleA.velocity + particleB.velocity + particleC.velocity) / 3;
    float3 longNormal = cross(particleB.position - particleA.position, particleC.position - particleA.position);
    float3 normal = normalize(longNormal);
    float3 dv = surfaceVelocity - float3(0, 0, 1);
    float3 crossArea = length(longNormal) / 2 * dot(normalize(dv), normal);
    float3 da = -1.225 * length_squared(dv) * 1.28 * crossArea * normal / 2 / particleMass;
    particleA.acceleration += da / 2;
}

void applyClothDrag(uint2 position, device Particle *particles) {
    uint index = position.y * particleCount + position.x;
    device Particle &particle = particles[index];
    if (position.x > 0 && position.y > 0) {
        applyDrag(particle, particles[index - 1], particles[index - particleCount]);
    }
    if (position.x < particleCount - 1 && position.y > 0) {
        applyDrag(particle, particles[index - particleCount], particles[index - particleCount + 1]);
        applyDrag(particle, particles[index - particleCount + 1], particles[index + 1]);
    }
    if (position.x < particleCount - 1 && position.y < particleCount - 1) {
        applyDrag(particle, particles[index + 1], particles[index + particleCount]);
    }
    if (position.x > 0 && position.y < particleCount - 1) {
        applyDrag(particle, particles[index + particleCount], particles[index + particleCount - 1]);
        applyDrag(particle, particles[index + particleCount - 1], particles[index - 1]);
    }
}

void simulateMotion(device Particle &particle, float dt) {
    if (!particle.alive) return;
    particle.velocity += dt * particle.acceleration;
    particle.position += dt * particle.velocity;
    particle.acceleration = 0;
}

void constrainCollision(device Particle &particle, raytracing::primitive_acceleration_structure accelerationStructure, raytracing::intersection_function_table<raytracing::triangle_data> intersectionFunctionTable, float3 previousPosition) {
    float3 displacement = particle.position - previousPosition;
    raytracing::ray ray{previousPosition, normalize(displacement), 0, length(displacement) + diagonalSpringLength};
    raytracing::intersector<raytracing::triangle_data> intersector;
    raytracing::intersection_result<raytracing::triangle_data> intersection = intersector.intersect(ray, accelerationStructure, intersectionFunctionTable, previousPosition);
    
    if (intersection.type != raytracing::intersection_type::none) {
        PrimitiveData data = *(const device PrimitiveData*)intersection.primitive_data;
        float2 bCoords = intersection.triangle_barycentric_coord;
        float3 surfaceNormal = bCoords.x * data.v0Normal + bCoords.y * data.v1Normal + (1 - bCoords.x - bCoords.y) * data.v2Normal;

        particle.velocity += dot(surfaceNormal, -particle.velocity) * surfaceNormal;
        particle.position = previousPosition + (intersection.distance - diagonalSpringLength) * ray.direction;
    }
}

float3 getTriangleNormal(device Particle &particleA, device Particle &particleB, device Particle &particleC) {
    return normalize(cross(particleB.position - particleA.position, particleC.position - particleA.position));
}

void recalculateClothNormals(uint2 position, device Particle *particles, device PrimitiveData *primitiveData) {
    uint index = position.y * particleCount + position.x;
    uint triangleIndex = position.y * (particleCount - 1) + position.x;
    device Particle &particle = particles[index];
    float3 averageNormal = float3();
    if (position.x > 0 && position.y > 0) {
        averageNormal += getTriangleNormal(particle, particles[index - 1], particles[index - particleCount]);
    }
    if (position.x < particleCount - 1 && position.y > 0) {
        averageNormal += getTriangleNormal(particle, particles[index - particleCount], particles[index - particleCount + 1]);
        averageNormal += getTriangleNormal(particle, particles[index - particleCount + 1], particles[index + 1]);
    }
    if (position.x < particleCount - 1 && position.y < particleCount - 1) {
        averageNormal += getTriangleNormal(particle, particles[index + 1], particles[index + particleCount]);
    }
    if (position.x > 0 && position.y < particleCount - 1) {
        averageNormal += getTriangleNormal(particle, particles[index + particleCount], particles[index + particleCount - 1]);
        averageNormal += getTriangleNormal(particle, particles[index + particleCount - 1], particles[index - 1]);
    }
    
    averageNormal = normalize(averageNormal);
    if (position.x > 0 && position.y > 0) {
        primitiveData[2 * (triangleIndex - (particleCount - 1) - 1) + 1].v1Normal = averageNormal;
    }
    if (position.x < particleCount - 1 && position.y > 0) {
        primitiveData[2 * (triangleIndex - (particleCount - 1))].v1Normal = averageNormal;
        primitiveData[2 * (triangleIndex - (particleCount - 1)) + 1].v0Normal = averageNormal;
    }
    if (position.x < particleCount - 1 && position.y < particleCount - 1) {
        primitiveData[2 * triangleIndex + 0].v2Normal = averageNormal;
    }
    if (position.x > 0 && position.y < particleCount - 1) {
        primitiveData[2 * (triangleIndex - 1) + 1].v2Normal = averageNormal;
        primitiveData[2 * (triangleIndex - 1)].v0Normal = averageNormal;
    }
}

[[intersection(triangle, raytracing::triangle_data)]]
bool intersectIgnoreClothTriangles(
    uint geometryId             [[geometry_id]],
    ray_data float3 &position   [[payload]],
    float3 origin               [[origin]],
    float3 direction            [[direction]],
    float distance              [[distance]]
) {
    return geometryId != 0 || length_squared(origin + direction * distance - position) > sideSpringLength;
}

/*
+ - + - +
| /1|2/3|
+ - @ - +
|6/5|4/ |
+ - + - +
*/

kernel void simulateClothKernel(
    uint2 position                                                                                  [[thread_position_in_grid]],
    constant float &dt                                                                              [[buffer(0)]],
    raytracing::primitive_acceleration_structure accelerationStructure                              [[buffer(1)]],
    raytracing::intersection_function_table<raytracing::triangle_data> intersectionFunctionTable    [[buffer(2)]],
    device Particle *particles                                                                      [[buffer(3)]],
    device PrimitiveData *primitiveData                                                             [[buffer(4)]],
    device packed_float3 *vertices                                                                  [[buffer(5)]]
) {
    if (position.x >= particleCount || position.y >= particleCount) return;
    uint index = position.y * particleCount + position.x;
    device Particle &particle = particles[index];
    float3 previousPosition = particle.position;

    applyGravity(particle);
    applyClothSpringDampers(position, particles, index, 1);
    applyClothSpringDampers(position, particles, index, 2);
    applyClothDrag(position, particles);
    simulateMotion(particle, dt);
    constrainCollision(particle, accelerationStructure, intersectionFunctionTable, previousPosition);
    recalculateClothNormals(position, particles, primitiveData);

    vertices[index] = particle.position;
}