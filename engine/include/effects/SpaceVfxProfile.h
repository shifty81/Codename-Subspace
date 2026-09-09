#pragma once

#include <cstdint>
#include <string>

namespace subspace {

enum class SpaceVfxKind {
    MiningBeam,
    SalvageBeam,
    ShieldHit,
    CargoPickup,
    AsteroidImpact,
    RailTravelSpark,
    HomeFactoryPulse
};

struct SpaceVfxProfile {
    SpaceVfxKind kind = SpaceVfxKind::MiningBeam;
    std::string label;
    std::uint32_t coreColor = 0xFFFFFFu;
    std::uint32_t glowColor = 0x66D9FFu;
    float coreWidth = 1.0f;
    float glowWidth = 3.0f;
    float durationSeconds = 0.25f;
    float particleRate = 8.0f;
};

SpaceVfxProfile CreateSpaceVfxProfile(SpaceVfxKind kind);
const char* SpaceVfxKindName(SpaceVfxKind kind);

} // namespace subspace
