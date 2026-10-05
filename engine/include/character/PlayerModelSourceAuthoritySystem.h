#pragma once

#include "world/WorldScaleAuthoritySystem.h"

#include <cmath>
#include <string>
#include <vector>

namespace subspace {

struct PlayerModelSourceProfile {
    std::string provider;
    std::string packId;
    std::string displayName;
    std::string officialUrl;
    std::string license;
    std::vector<std::string> acceptedRuntimeFormats;
    bool humanoidRigRequired = true;
    bool skinnedMeshRequired = true;
    bool naturalHeightIsScaleAuthority = true;
};

struct PlayerModelRuntimeEvidence {
    bool sourceHydrated = false;
    bool skinnedMeshLoaded = false;
    bool humanoidRigResolved = false;
    bool animationCompatible = false;
    float sourceHeightUnits = 0.0f;
    float sourceMetersPerUnit = 0.0f;
};

struct PlayerModelCertification {
    bool ready = false;
    float measuredHeightMeters = 0.0f;
    float importScaleToWorld = 1.0f;
    WorldScaleProfile worldScale{};
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

/// Governs the actual runtime player source and makes the measured imported
/// humanoid the human-scale authority.  The historical 1.80 m profile is a
/// bootstrap default only; a certified adult player source updates doors,
/// corridors, eye height, decks, interaction reach and authoring guides.
class PlayerModelSourceAuthoritySystem {
public:
    static PlayerModelSourceProfile QuaterniusUniversalBaseCharacters() {
        PlayerModelSourceProfile p;
        p.provider = "Quaternius";
        p.packId = "quaternius.universal_base_characters.standard";
        p.displayName = "Quaternius Universal Base Characters [Standard]";
        p.officialUrl = "https://quaternius.com/packs/universalbasecharacters.html";
        p.license = "CC0-1.0";
        p.acceptedRuntimeFormats = {".gltf", ".glb", ".fbx"};
        return p;
    }

    static PlayerModelCertification Certify(const PlayerModelRuntimeEvidence& evidence,
                                             const WorldScaleProfile& bootstrap = WorldScaleAuthoritySystem::DefaultProfile()) {
        PlayerModelCertification out;
        out.worldScale = bootstrap;
        if (!evidence.sourceHydrated) out.errors.push_back("Player source pack is not hydrated through governed asset intake");
        if (!evidence.skinnedMeshLoaded) out.errors.push_back("Player source has no loaded skinned mesh");
        if (!evidence.humanoidRigResolved) out.errors.push_back("Player source humanoid rig has not been resolved");
        if (!evidence.animationCompatible) out.errors.push_back("Player source has not passed animation-retarget compatibility");
        if (!(evidence.sourceHeightUnits > 0.0f) || !(evidence.sourceMetersPerUnit > 0.0f)) {
            out.errors.push_back("Player source height/unit calibration is missing");
            return out;
        }
        out.measuredHeightMeters = evidence.sourceHeightUnits * evidence.sourceMetersPerUnit;
        if (!std::isfinite(out.measuredHeightMeters) || out.measuredHeightMeters < 1.20f || out.measuredHeightMeters > 2.40f) {
            out.errors.push_back("Measured player height is outside the supported human-scale calibration envelope");
            return out;
        }

        // Preserve natural character proportions: convert source units to the
        // one-metre Subspace world, then derive the rest of the human-scale
        // profile from the measured model instead of scaling it to 1.80 m.
        out.importScaleToWorld = evidence.sourceMetersPerUnit / std::max(0.0001f, bootstrap.metersPerWorldUnit);
        out.worldScale = WorldScaleAuthoritySystem::WithPlayerHeight(out.measuredHeightMeters, bootstrap);
        if (std::fabs(out.measuredHeightMeters - bootstrap.referencePlayerHeightMeters) > 0.30f)
            out.warnings.push_back("Player model materially changes the bootstrap human-scale profile; review existing authored interiors");
        out.ready = out.errors.empty();
        return out;
    }
};

} // namespace subspace
