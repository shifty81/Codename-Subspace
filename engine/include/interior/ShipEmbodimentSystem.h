#pragma once

#include "core/Math.h"
#include <cstdint>
#include <string>

namespace subspace {

enum class ShipEmbodimentMode { CockpitControl, InteriorOnFoot, CutawayInspection, DockedHangar };

struct InteriorTraversalBounds {
    Vector3 minimum{-1.75f,-2.45f,0.0f};
    Vector3 maximum{1.75f,2.15f,3.0f};
    bool enabled = true;
};

struct InteriorAvatarState {
    std::uint64_t shipId = 0;
    Vector3 localPosition{0.0f, 1.45f, 0.0f};
    int deck = 0;
    float facingRadians = 0.0f; // compatibility alias for lookYawRadians
    float lookYawRadians = 0.0f;
    float lookPitchRadians = 0.0f;
    float moveSpeed = 3.2f;
    float eyeHeightMeters = 1.68f;
    float capsuleHeightMeters = 1.80f;
    float capsuleRadiusMeters = 0.32f;
};

/// Embodied first-person authority for a persistent ship interior. Generated
/// interior collision/nav may replace the default legacy bounds at runtime;
/// camera look and locomotion remain player-scale and ship-local.
class ShipEmbodimentSystem {
public:
    ShipEmbodimentMode Mode() const { return mode_; }
    const InteriorAvatarState& Avatar() const { return avatar_; }
    bool ExitCockpit(std::uint64_t shipId);
    bool TakeControls();
    bool EnterDockedHangar(std::uint64_t shipId);
    bool BoardInterior(std::uint64_t shipId);
    void SetInspection(bool enabled);
    void Move(float forward, float strafe, double seconds);
    void Look(float yawDeltaRadians,float pitchDeltaRadians);
    void SetTraversalBounds(const InteriorTraversalBounds& bounds) { traversalBounds_=bounds; }
    const InteriorTraversalBounds& TraversalBounds() const { return traversalBounds_; }
    Vector3 EyeLocalPosition() const;
    bool ZoomMayRevealInterior(float) const { return false; }
    bool IsPiloting() const { return mode_ == ShipEmbodimentMode::CockpitControl; }
    bool IsOnFoot() const { return mode_ == ShipEmbodimentMode::InteriorOnFoot; }
private:
    ShipEmbodimentMode mode_ = ShipEmbodimentMode::CockpitControl;
    InteriorAvatarState avatar_{};
    InteriorTraversalBounds traversalBounds_{};
};

} // namespace subspace
