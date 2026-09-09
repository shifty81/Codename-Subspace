#include "world/WorldScaleAuthoritySystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {

WorldScaleProfile WorldScaleAuthoritySystem::DefaultProfile() { return {}; }

WorldScaleProfile WorldScaleAuthoritySystem::WithPlayerHeight(float playerHeightMeters,
                                                              const WorldScaleProfile& base) {
    WorldScaleProfile p = base;
    const float oldHeight = std::max(0.25f, base.referencePlayerHeightMeters);
    const float newHeight = std::clamp(playerHeightMeters, 0.50f, 4.0f);
    const float ratio = newHeight / oldHeight;
    p.referencePlayerHeightMeters = newHeight;
    p.referenceEyeHeightMeters = base.referenceEyeHeightMeters * ratio;
    p.referenceShoulderWidthMeters = base.referenceShoulderWidthMeters * ratio;
    p.referenceReachMeters = base.referenceReachMeters * ratio;
    p.referenceDoorHeightMeters = base.referenceDoorHeightMeters * ratio;
    p.referenceDoorWidthMeters = base.referenceDoorWidthMeters * ratio;
    p.referenceCorridorWidthMeters = base.referenceCorridorWidthMeters * ratio;
    p.referenceInteriorCellMeters = base.referenceInteriorCellMeters * ratio;
    p.referenceDeckHeightMeters = base.referenceDeckHeightMeters * ratio;
    // Authoring snap sizes are physical project conventions. Fine/detail snaps
    // follow the player reference while the 4 m ship foundation remains the
    // canonical construction grid used throughout the existing project.
    p.fineSnapMeters = base.fineSnapMeters * ratio;
    p.detailSnapMeters = base.detailSnapMeters * ratio;
    p.structuralSnapMeters = base.structuralSnapMeters * ratio;
    return p;
}

float WorldScaleAuthoritySystem::Resolve(ScaleReferenceKind kind, const WorldScaleProfile& p) {
    switch (kind) {
        case ScaleReferenceKind::PlayerHeight: return p.referencePlayerHeightMeters;
        case ScaleReferenceKind::EyeHeight: return p.referenceEyeHeightMeters;
        case ScaleReferenceKind::DoorHeight: return p.referenceDoorHeightMeters;
        case ScaleReferenceKind::DoorWidth: return p.referenceDoorWidthMeters;
        case ScaleReferenceKind::CorridorWidth: return p.referenceCorridorWidthMeters;
        case ScaleReferenceKind::InteriorCell: return p.referenceInteriorCellMeters;
        case ScaleReferenceKind::DeckHeight: return p.referenceDeckHeightMeters;
        case ScaleReferenceKind::FineSnap: return p.fineSnapMeters;
        case ScaleReferenceKind::DetailSnap: return p.detailSnapMeters;
        case ScaleReferenceKind::StructuralSnap: return p.structuralSnapMeters;
        case ScaleReferenceKind::ShipFoundation: return p.shipFoundationMeters;
    }
    return 1.0f;
}

HumanScaleGuide WorldScaleAuthoritySystem::BuildHumanGuide(const WorldScaleProfile& p) {
    return {p.referencePlayerHeightMeters, p.referenceEyeHeightMeters,
            p.referenceDoorHeightMeters, p.referenceDoorWidthMeters,
            p.referenceCorridorWidthMeters, p.referenceDeckHeightMeters,
            p.referenceInteriorCellMeters};
}

ScaleCalibrationResult WorldScaleAuthoritySystem::CalibrateHumanoid(float sourceHeightUnits,
                                                                    float sourceMetersPerUnit,
                                                                    const WorldScaleProfile& p) {
    ScaleCalibrationResult out;
    if (sourceHeightUnits <= 0.0f || sourceMetersPerUnit <= 0.0f) {
        out.warnings.push_back("Humanoid source height and source unit scale must both be positive");
        return out;
    }
    out.measuredHeightMeters = sourceHeightUnits * sourceMetersPerUnit;
    out.targetHeightMeters = p.referencePlayerHeightMeters;
    out.uniformScale = out.targetHeightMeters / out.measuredHeightMeters;
    out.valid = std::isfinite(out.uniformScale) && out.uniformScale > 0.0f;
    if (out.uniformScale < 0.05f || out.uniformScale > 20.0f)
        out.warnings.push_back("Humanoid import scale differs greatly from Subspace player reference");
    return out;
}

bool WorldScaleAuthoritySystem::IsHumanScalePlausible(float sizeMeters,
                                                       ScaleReferenceKind reference,
                                                       const WorldScaleProfile& p,
                                                       float toleranceFraction) {
    const float expected = std::max(0.001f, Resolve(reference, p));
    const float tolerance = expected * std::max(0.01f, toleranceFraction);
    return std::fabs(sizeMeters - expected) <= tolerance;
}

} // namespace subspace
