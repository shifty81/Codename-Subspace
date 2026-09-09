#pragma once

#include "core/Math.h"
#include <cstdint>
#include <string>

namespace subspace {

enum class ShipEmbodimentMode { CockpitControl, InteriorOnFoot, CutawayInspection, DockedHangar };

struct InteriorAvatarState {
    std::uint64_t shipId = 0;
    Vector3 localPosition{0.0f, 1.45f, 0.0f};
    int deck = 0;
    float facingRadians = 0.0f;
    float moveSpeed = 3.2f;
};

/// Pass306-310 authority separating piloting, exterior inspection and actual
/// top-down on-foot interior play. Camera zoom can never enter the interior.
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
    bool ZoomMayRevealInterior(float) const { return false; }
    bool IsPiloting() const { return mode_ == ShipEmbodimentMode::CockpitControl; }
    bool IsOnFoot() const { return mode_ == ShipEmbodimentMode::InteriorOnFoot; }
private:
    ShipEmbodimentMode mode_ = ShipEmbodimentMode::CockpitControl;
    InteriorAvatarState avatar_{};
};

} // namespace subspace
