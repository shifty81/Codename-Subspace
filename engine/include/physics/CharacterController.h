#pragma once

#include "core/Math.h"
#include "core/ecs/IComponent.h"
#include "core/persistence/SaveGameManager.h"

namespace subspace {

/// Parameters that define the character capsule and movement properties.
struct CharacterControllerConfig {
    float height     = 1.8f;   ///< Full capsule height in metres.
    float radius     = 0.4f;   ///< Capsule radius in metres.
    float stepHeight = 0.35f;  ///< Maximum step the character can climb.
    float gravity    = 9.81f;  ///< Gravity acceleration (positive = downward).
    float maxSlope   = 45.0f;  ///< Maximum walkable slope in degrees.
    float skinWidth  = 0.01f;  ///< Anti-penetration skin width.
};

/// Component that stores a character controller's runtime state.
///
/// Attach to a player-controlled or NPC entity to enable capsule-based
/// character movement on ship interiors, stations, and planetary surfaces.
///
/// The CharacterController does NOT call physics; callers use Move() on the
/// system and receive a collision-resolved displacement.
struct CharacterControllerComponent : public IComponent {
    CharacterControllerConfig config;

    Vector3 position{};      ///< World-space foot position.
    Vector3 velocity{};      ///< Current velocity in m/s.

    bool isGrounded   = false;
    bool isOnStep     = false;
    bool isEnabled    = true;

    ComponentData Serialize()                      const;
    void          Deserialize(const ComponentData& data);
};

/// Manages character movement with capsule AABB collision and gravity.
///
/// Usage:
/// @code
///   CharacterController cc;
///
///   // Each frame:
///   Vector3 desiredVelocity = { moveX, 0, moveZ };
///   Vector3 newPos = cc.Move(comp, desiredVelocity, dt);
///   comp.position = newPos;
/// @endcode
class CharacterController {
public:
    /// Apply @p desiredVelocity to the character for @p dt seconds.
    ///
    /// Gravity is applied automatically based on the component's
    /// config.gravity when the character is not grounded.
    ///
    /// @return New world-space position after collision resolution.
    static Vector3 Move(CharacterControllerComponent& comp,
                        const Vector3& desiredVelocity,
                        float dt);

    /// Returns the AABB half-extents for the character capsule.
    static Vector3 GetHalfExtents(const CharacterControllerComponent& comp) noexcept;

    /// Teleport the character to a new position without any collision checks.
    static void Teleport(CharacterControllerComponent& comp,
                         const Vector3& newPosition) noexcept;
};

} // namespace subspace
