#pragma once

#include "core/Math.h"

#include <string>
#include <vector>

namespace subspace {

enum class ScaleReferenceKind {
    PlayerHeight,
    EyeHeight,
    DoorHeight,
    DoorWidth,
    CorridorWidth,
    InteriorCell,
    DeckHeight,
    FineSnap,
    DetailSnap,
    StructuralSnap,
    ShipFoundation
};

struct WorldScaleProfile {
    // One Subspace world unit is one meter. The player is the visible scale
    // reference from which generated doors, corridors, decks and authoring
    // guides derive; changing the reference player height therefore changes
    // generated human-scale content coherently instead of scaling physics units.
    float metersPerWorldUnit = 1.0f;
    float referencePlayerHeightMeters = 1.80f;
    float referenceEyeHeightMeters = 1.68f;
    float referenceShoulderWidthMeters = 0.46f;
    float referenceReachMeters = 0.82f;
    float referenceDoorHeightMeters = 2.25f;
    float referenceDoorWidthMeters = 1.05f;
    float referenceCorridorWidthMeters = 1.60f;
    float referenceInteriorCellMeters = 2.0f;
    float referenceDeckHeightMeters = 3.0f;
    float fineSnapMeters = 0.25f;
    float detailSnapMeters = 0.50f;
    float structuralSnapMeters = 2.0f;
    float shipFoundationMeters = 4.0f;
};

struct ScaleCalibrationResult {
    bool valid = false;
    float uniformScale = 1.0f;
    float measuredHeightMeters = 0.0f;
    float targetHeightMeters = 0.0f;
    std::vector<std::string> warnings;
};

struct HumanScaleGuide {
    float playerHeightMeters = 1.8f;
    float eyeHeightMeters = 1.68f;
    float doorHeightMeters = 2.25f;
    float doorWidthMeters = 1.05f;
    float corridorWidthMeters = 1.6f;
    float deckHeightMeters = 3.0f;
    float interiorCellMeters = 2.0f;
};

class WorldScaleAuthoritySystem {
public:
    static WorldScaleProfile DefaultProfile();
    static WorldScaleProfile WithPlayerHeight(float playerHeightMeters,
                                               const WorldScaleProfile& base = DefaultProfile());
    static float Resolve(ScaleReferenceKind kind, const WorldScaleProfile& profile);
    static HumanScaleGuide BuildHumanGuide(const WorldScaleProfile& profile);
    static ScaleCalibrationResult CalibrateHumanoid(float sourceHeightUnits,
                                                     float sourceMetersPerUnit,
                                                     const WorldScaleProfile& profile);
    static bool IsHumanScalePlausible(float sizeMeters,
                                      ScaleReferenceKind reference,
                                      const WorldScaleProfile& profile,
                                      float toleranceFraction = 0.50f);
};

} // namespace subspace
