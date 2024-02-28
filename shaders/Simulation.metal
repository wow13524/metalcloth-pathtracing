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

void simulateMotion(device Particle &particle, float dt) {
    if (!particle.alive) return;
    particle.velocity += dt * particle.acceleration;
    particle.position += dt * particle.velocity;
    particle.acceleration = 0;
}

void constrainCollision(device Particle &particle, float dt, raytracing::primitive_acceleration_structure accelerationStructure, float3 previousPosition) {
    float3 displacement = particle.position - previousPosition;
    float distance = length(displacement);
    raytracing::ray ray{previousPosition, normalize(displacement), EPSILON, distance + EPSILON};
    raytracing::intersector<raytracing::triangle_data> intersector;
    raytracing::intersection_result<raytracing::triangle_data> intersection = intersector.intersect(ray, accelerationStructure);
    
    if (intersection.type != raytracing::intersection_type::none) {
        PrimitiveData data = *(const device PrimitiveData*)intersection.primitive_data;
        float2 bCoords = intersection.triangle_barycentric_coord;
        float3 surfaceNormal = bCoords.x * data.v0Normal + bCoords.y * data.v1Normal + (1 - bCoords.x - bCoords.y) * data.v2Normal;

        particle.velocity += dot(surfaceNormal, -particle.velocity) * surfaceNormal;
        particle.position = previousPosition + (intersection.distance - 2 * EPSILON) * ray.direction;
    }
}

kernel void simulateClothKernel(
    uint2 position [[thread_position_in_grid]],
    constant float &dt [[buffer(0)]],
    raytracing::primitive_acceleration_structure accelerationStructure [[buffer(1)]],
    device Particle *particles [[buffer(2)]],
    device packed_float3 *vertices [[buffer(3)]]
) {
    if (position.x >= particleCount || position.y >= particleCount) return;
    uint index = position.y * particleCount + position.x;
    device Particle &particle = particles[index];
    float3 previousPosition = particle.position;

    //neighbor spring dampers
    if (position.x > 0) applySpringDamper(particle, particles[index - 1], sideSpringLength);
    if (position.y > 0) applySpringDamper(particle, particles[index - particleCount], sideSpringLength);
    if (position.x < particleCount - 1) applySpringDamper(particle, particles[index + 1], sideSpringLength);
    if (position.y < particleCount - 1) applySpringDamper(particle, particles[index + particleCount], sideSpringLength);
    if (position.x > 0 && position.y > 0) applySpringDamper(particle, particles[index - particleCount - 1], diagonalSpringLength);
    if (position.x > 0 && position.y < particleCount - 1) applySpringDamper(particle, particles[index + particleCount - 1], diagonalSpringLength);
    if (position.x < particleCount - 1 && position.y > 0) applySpringDamper(particle, particles[index - particleCount + 1], diagonalSpringLength);
    if (position.x < particleCount - 1 && position.y < particleCount - 1) applySpringDamper(particle, particles[index + particleCount + 1], diagonalSpringLength);

    //bending spring dampers
    if (position.x > 1) applySpringDamper(particle, particles[index - 2], 2 * sideSpringLength);
    if (position.y > 1) applySpringDamper(particle, particles[index - 2 * particleCount], 2 * sideSpringLength);
    if (position.x < particleCount - 2) applySpringDamper(particle, particles[index + 2], 2 * sideSpringLength);
    if (position.y < particleCount - 2) applySpringDamper(particle, particles[index + 2 * particleCount], 2 * sideSpringLength);
    if (position.x > 1 && position.y > 1) applySpringDamper(particle, particles[index - 2 * particleCount - 2], 2 * diagonalSpringLength);
    if (position.x > 1 && position.y < particleCount - 2) applySpringDamper(particle, particles[index + 2 * particleCount - 2], 2 * diagonalSpringLength);
    if (position.x < particleCount - 2 && position.y > 1) applySpringDamper(particle, particles[index - 2 * particleCount + 2], 2 * diagonalSpringLength);
    if (position.x < particleCount - 2 && position.y < particleCount - 2) applySpringDamper(particle, particles[index + 2 * particleCount + 2], 2 * diagonalSpringLength);

    applyGravity(particle);
    simulateMotion(particle, dt);
    constrainCollision(particle, dt, accelerationStructure, previousPosition);

    vertices[index] = particle.position;
}