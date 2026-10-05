#pragma once

#include "core/Math.h"
#include <cstdint>
#include <string>

namespace subspace {

enum class ShipEmbodimentMode { CockpitControl, InteriorOnFoot, CutawayInspection, DockedHangar };
enum class InteriorAvatarStance { Standing, Crouched };

struct InteriorTraversalBounds {
    Vector3 minimum{-1.75f,-2.45f,0.0f};
    Vector3 maximum{1.75f,2.15f,3.0f};
    bool enabled = true;
};

struct InteriorAvatarState {
    std::uint64_t shipId = 0;
    Vector3 localPosition{0.0f, 1.45f, 0.0f};
    Vector3 planarVelocity{};
    int deck = 0;
    float facingRadians = 0.0f; // compatibility alias for lookYawRadians
    float lookYawRadians = 0.0f;
    float lookPitchRadians = 0.0f;
    float headLookYawOffsetRadians = 0.0f;
    float headLookPitchOffsetRadians = 0.0f;
    // Compatibility/current values consumed by shell traversal and camera code.
    float moveSpeed = 3.2f;
    float eyeHeightMeters = 1.68f;
    float capsuleHeightMeters = 1.80f;
    float capsuleRadiusMeters = 0.32f;

    // Normalized FPS movement profile. Movement acceleration happens here;
    // collision/occupancy remains owned by ShipInteriorShellTraversalSystem.
    float walkSpeedMetersPerSecond = 3.2f;
    float sprintSpeedMetersPerSecond = 5.6f;
    float crouchSpeedMetersPerSecond = 1.8f;
    float accelerationMetersPerSecond2 = 18.0f;
    float decelerationMetersPerSecond2 = 24.0f;
    float standingEyeHeightMeters = 1.68f;
    float crouchedEyeHeightMeters = 1.05f;
    float standingCapsuleHeightMeters = 1.80f;
    float crouchedCapsuleHeightMeters = 1.18f;
    float stamina01 = 1.0f;
    InteriorAvatarStance stance = InteriorAvatarStance::Standing;
    bool sprinting = false;
    bool grounded = true;
};

/// Embodied first-person authority for a persistent ship interior. Generated
/// interior collision/nav may replace the default legacy bounds at runtime;
/// camera look and locomotion remain player-scale and ship-local.
class ShipEmbodimentSystem {
public:
    ShipEmbodimentMode Mode() const { return mode_; }
    const InteriorAvatarState& Avatar() const { return avatar_; }
    bool ExitCockpit(std::uint64_t shipId);
    bool CanTakeControls() const;
    bool TakeControls();
    bool TakeControlsAt(Vector3 seatFeet,float interactionRadiusMeters=1.45f);
    void SetCommandSeatLocalPosition(Vector3 seatFeet) { commandSeatLocalPosition_=seatFeet; RefreshCommandSeatInteraction(); }
    Vector3 CommandSeatLocalPosition() const { return commandSeatLocalPosition_; }
    bool EnterDockedHangar(std::uint64_t shipId);
    bool BoardInterior(std::uint64_t shipId);
    void SetInspection(bool enabled);

    // Compatibility API used by the live runtime. It now uses accelerated
    // locomotion instead of teleport-style constant displacement.
    void Move(float forward, float strafe, double seconds);
    void ConfigureLocomotion(bool sprintRequested, bool crouchRequested);
    void StopLocomotion();

    // Runtime shell traversal sets only a certified ship-local foot position.
    // Seat interaction eligibility is updated from the certified location so
    // accelerated locomotion cannot accidentally leave the cockpit interaction
    // armed after the player has begun walking away.
    void SetCertifiedFootPosition(Vector3 position);
    void Look(float yawDeltaRadians,float pitchDeltaRadians);
    void HeadLook(float yawDeltaRadians,float pitchDeltaRadians);
    void UpdateHeadLook(bool active,double seconds);
    void ResetHeadLook();
    void SetTraversalBounds(const InteriorTraversalBounds& bounds) { traversalBounds_=bounds; }
    const InteriorTraversalBounds& TraversalBounds() const { return traversalBounds_; }
    Vector3 EyeLocalPosition() const;
    bool ZoomMayRevealInterior(float) const { return false; }
    bool IsPiloting() const { return mode_ == ShipEmbodimentMode::CockpitControl; }
    bool IsOnFoot() const { return mode_ == ShipEmbodimentMode::InteriorOnFoot; }
private:
    void RefreshCommandSeatInteraction();
    ShipEmbodimentMode mode_ = ShipEmbodimentMode::CockpitControl;
    InteriorAvatarState avatar_{};
    InteriorTraversalBounds traversalBounds_{};
    bool commandSeatInteractionReady_ = true;
    bool commandSeatDeparted_ = false;
    Vector3 commandSeatLocalPosition_{0.0f,1.45f,0.0f};
};

} // namespace subspace
