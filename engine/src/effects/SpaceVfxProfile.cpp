#include "effects/SpaceVfxProfile.h"

namespace subspace {

const char* SpaceVfxKindName(SpaceVfxKind kind) {
    switch (kind) {
    case SpaceVfxKind::MiningBeam: return "Mining Beam";
    case SpaceVfxKind::SalvageBeam: return "Salvage Beam";
    case SpaceVfxKind::ShieldHit: return "Shield Hit";
    case SpaceVfxKind::CargoPickup: return "Cargo Pickup";
    case SpaceVfxKind::AsteroidImpact: return "Asteroid Impact";
    case SpaceVfxKind::RailTravelSpark: return "Rail Travel Spark";
    case SpaceVfxKind::HomeFactoryPulse: return "Home Factory Pulse";
    }
    return "Unknown";
}

SpaceVfxProfile CreateSpaceVfxProfile(SpaceVfxKind kind) {
    SpaceVfxProfile profile;
    profile.kind = kind;
    profile.label = SpaceVfxKindName(kind);
    switch (kind) {
    case SpaceVfxKind::MiningBeam: profile.coreColor = 0xFFF0A0u; profile.glowColor = 0xFFAA33u; profile.coreWidth = 2.0f; profile.glowWidth = 7.0f; profile.durationSeconds = 0.15f; profile.particleRate = 18.0f; break;
    case SpaceVfxKind::SalvageBeam: profile.coreColor = 0xB6F3FFu; profile.glowColor = 0x41C8FFu; profile.coreWidth = 1.5f; profile.glowWidth = 6.0f; profile.durationSeconds = 0.20f; profile.particleRate = 14.0f; break;
    case SpaceVfxKind::ShieldHit: profile.coreColor = 0xFFFFFFu; profile.glowColor = 0x6688FFu; profile.coreWidth = 3.0f; profile.glowWidth = 12.0f; profile.durationSeconds = 0.35f; profile.particleRate = 20.0f; break;
    case SpaceVfxKind::CargoPickup: profile.coreColor = 0xD8FFC0u; profile.glowColor = 0x80FF80u; profile.coreWidth = 2.0f; profile.glowWidth = 10.0f; profile.durationSeconds = 0.45f; profile.particleRate = 12.0f; break;
    case SpaceVfxKind::AsteroidImpact: profile.coreColor = 0xFFD0A0u; profile.glowColor = 0xAA7040u; profile.coreWidth = 3.0f; profile.glowWidth = 8.0f; profile.durationSeconds = 0.25f; profile.particleRate = 25.0f; break;
    case SpaceVfxKind::RailTravelSpark: profile.coreColor = 0xCCEEFFu; profile.glowColor = 0x6AAEFFu; profile.coreWidth = 1.0f; profile.glowWidth = 5.0f; profile.durationSeconds = 0.18f; profile.particleRate = 30.0f; break;
    case SpaceVfxKind::HomeFactoryPulse: profile.coreColor = 0xC9FFE8u; profile.glowColor = 0x4BE4B0u; profile.coreWidth = 1.0f; profile.glowWidth = 4.0f; profile.durationSeconds = 0.50f; profile.particleRate = 6.0f; break;
    }
    return profile;
}

} // namespace subspace
