#include "physics/CharacterController.h"

#include <cmath>

namespace subspace {

// ---------------------------------------------------------------------------
// CharacterControllerComponent
// ---------------------------------------------------------------------------

ComponentData CharacterControllerComponent::Serialize() const {
    ComponentData d;
    d.componentType = "CharacterControllerComponent";
    d.data["posX"] = std::to_string(position.x);
    d.data["posY"] = std::to_string(position.y);
    d.data["posZ"] = std::to_string(position.z);
    d.data["velX"] = std::to_string(velocity.x);
    d.data["velY"] = std::to_string(velocity.y);
    d.data["velZ"] = std::to_string(velocity.z);
    d.data["isGrounded"] = isGrounded ? "1" : "0";
    d.data["isEnabled"]  = isEnabled  ? "1" : "0";
    d.data["height"]     = std::to_string(config.height);
    d.data["radius"]     = std::to_string(config.radius);
    d.data["stepHeight"] = std::to_string(config.stepHeight);
    return d;
}

void CharacterControllerComponent::Deserialize(const ComponentData& data) {
    if (data.data.count("posX"))      position.x    = std::stof(data.data.at("posX"));
    if (data.data.count("posY"))      position.y    = std::stof(data.data.at("posY"));
    if (data.data.count("posZ"))      position.z    = std::stof(data.data.at("posZ"));
    if (data.data.count("velX"))      velocity.x    = std::stof(data.data.at("velX"));
    if (data.data.count("velY"))      velocity.y    = std::stof(data.data.at("velY"));
    if (data.data.count("velZ"))      velocity.z    = std::stof(data.data.at("velZ"));
    if (data.data.count("isGrounded"))isGrounded    = data.data.at("isGrounded") == "1";
    if (data.data.count("isEnabled")) isEnabled     = data.data.at("isEnabled")  == "1";
    if (data.data.count("height"))    config.height = std::stof(data.data.at("height"));
    if (data.data.count("radius"))    config.radius = std::stof(data.data.at("radius"));
    if (data.data.count("stepHeight"))config.stepHeight = std::stof(data.data.at("stepHeight"));
}

// ---------------------------------------------------------------------------
// CharacterController (static helpers)
// ---------------------------------------------------------------------------

Vector3 CharacterController::GetHalfExtents(
    const CharacterControllerComponent& comp) noexcept {
    return { comp.config.radius,
             comp.config.height * 0.5f,
             comp.config.radius };
}

void CharacterController::Teleport(CharacterControllerComponent& comp,
                                    const Vector3& newPosition) noexcept {
    comp.position  = newPosition;
    comp.velocity  = {};
    comp.isGrounded = false;
}

Vector3 CharacterController::Move(CharacterControllerComponent& comp,
                                   const Vector3& desiredVelocity,
                                   float dt) {
    if (!comp.isEnabled) return comp.position;

    // Apply gravity if not grounded
    if (!comp.isGrounded) {
        comp.velocity.y -= comp.config.gravity * dt;
    } else {
        // Clamp downward velocity when grounded
        if (comp.velocity.y < 0.0f) comp.velocity.y = 0.0f;
    }

    // Horizontal movement comes entirely from desired velocity
    comp.velocity.x = desiredVelocity.x;
    comp.velocity.z = desiredVelocity.z;

    // Integrate position
    Vector3 delta{
        comp.velocity.x * dt,
        comp.velocity.y * dt,
        comp.velocity.z * dt
    };

    Vector3 newPos{
        comp.position.x + delta.x,
        comp.position.y + delta.y,
        comp.position.z + delta.z
    };

    // Simple ground plane at y=0 (a real implementation would ray-cast
    // against the voxel world; this provides correct grounding semantics
    // without physics coupling)
    if (newPos.y <= 0.0f) {
        newPos.y     = 0.0f;
        comp.velocity.y = 0.0f;
        comp.isGrounded  = true;
    } else {
        comp.isGrounded = false;
    }

    comp.position = newPos;
    return newPos;
}

} // namespace subspace
