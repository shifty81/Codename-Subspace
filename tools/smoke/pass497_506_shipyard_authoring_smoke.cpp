#include "ship_editor/ShipyardAuthoringAuthority.h"
#include "ship_editor/ShipyardAuthoringBridge.h"

#include <cassert>
#include <iostream>

using namespace subspace;

int main() {
    ShipyardAuthoringAuthority authority;
    authority.RegisterBuiltInCertifiedOverrides();
    auto wing2 = authority.InferDefinition("shipyard_a_wing_156_shipyard_wing_010_miscwing2");
    wing2.certification = ShipyardCertificationState::Reviewed;
    wing2.compatibleSocketTypes = {"lateral_mount", "wing_root", "hull_lateral"};
    authority.RegisterDefinition(wing2);
    auto curve = authority.InferDefinition("shipyard_a_wing_148_shipyard_wing_002_misccurvewing");
    curve.certification = ShipyardCertificationState::Reviewed;
    curve.compatibleSocketTypes = {"lateral_mount", "wing_root", "hull_lateral"};
    authority.RegisterDefinition(curve);

    const auto* wing149 = authority.FindDefinition("shipyard_a_wing_149_shipyard_wing_003_miscfinhanger");
    assert(wing149);
    assert(wing149->subtype == ShipyardAuthoringSubtype::LateralWing);
    assert(wing149->certification == ShipyardCertificationState::Certified);

    ShipyardPlacementEvidence bad;
    bad.rootFacesParent = 0.2f;
    bad.outwardFromCenterline = 0.8f;
    bad.forwardAlignment = 0.9f;
    bad.upAlignment = 0.1f;
    auto badScore = authority.ScorePlacement(*wing149, bad);
    assert(badScore.hardInvalid);
    assert(authority.RecommendRepair(badScore, true, true, true, true) == ShipyardRepairAction::CorrectOrientation);

    ShipyardPlacementEvidence good;
    good.rootFacesParent = 0.95f;
    good.outwardFromCenterline = 0.95f;
    good.forwardAlignment = 0.95f;
    good.upAlignment = 0.95f;
    auto goodScore = authority.ScorePlacement(*wing149, good);
    assert(!goodScore.hardInvalid);
    assert(authority.RecommendRepair(goodScore, true, true, true, true) == ShipyardRepairAction::Keep);

    ShipyardResolvedBlueprint ship;
    ship.shipSeed = 881452;
    ship.blueprintId = "cert.fighter.001";
    ShipyardSlot hull;
    hull.slotId = "hull.mid.01";
    hull.role = "Hull";
    hull.moduleDefinitionId = "shipyard_hull_mid";
    ship.slots.push_back(hull);
    ShipyardSlot left;
    left.slotId = "surface.port.01";
    left.role = "LateralWing";
    left.moduleDefinitionId = wing149->definitionId;
    left.parentSlotId = "hull.mid.01";
    left.parentSocketId = "lateral_port_02";
    ShipyardSlot right = left;
    right.slotId = "surface.starboard.01";
    right.parentSocketId = "lateral_starboard_02";
    ship.slots.push_back(left);
    ship.slots.push_back(right);
    authority.RegisterDefinition(authority.InferDefinition("shipyard_hull_mid"));
    assert(authority.PairSlots(ship, "surface.port.01", "surface.starboard.01", ShipyardPairMode::MirroredIdentical));

    ShipyardSelectionAuthority selection;
    assert(selection.SelectSlot(ship, "surface.port.01"));
    assert(selection.SelectedDefinitionId() == wing149->definitionId);

    auto preview = authority.PreviewReroll(ship, "surface.port.01", ShipyardRerollMode::Similar);
    assert(preview.valid);
    assert(preview.candidateDefinitionId != wing149->definitionId);
    const auto beforeCount = ship.slots.size();
    assert(authority.CommitReroll(ship, preview));
    assert(ship.slots.size() == beforeCount);
    assert(ship.slots[1].slotId == "surface.port.01");
    assert(ship.slots[1].parentSocketId == "lateral_port_02");
    assert(authority.RestorePreviousReroll(ship, "surface.port.01"));
    assert(ship.slots[1].moduleDefinitionId == wing149->definitionId);

    auto pairPreviews = authority.PreviewRerollOperation(ship, "surface.port.01", ShipyardRerollMode::Pair);
    assert(pairPreviews.size() == 2);
    assert(pairPreviews[0].candidateDefinitionId == pairPreviews[1].candidateDefinitionId);
    assert(authority.CommitRerollOperation(ship, pairPreviews));
    assert(ship.slots[1].moduleDefinitionId == ship.slots[2].moduleDefinitionId);
    assert(authority.RestorePreviousReroll(ship, "surface.port.01"));
    assert(authority.RestorePreviousReroll(ship, "surface.starboard.01"));

    ShipyardSlotLocks lock;
    lock.module = true;
    assert(authority.LockSlot(ship, "surface.port.01", lock));
    assert(!authority.PreviewReroll(ship, "surface.port.01", ShipyardRerollMode::Similar).valid);

    assert(ShipyardAuthoringAuthority::InferMaterialZone("Mat_Main") == ShipyardMaterialZone::PrimaryPaint);
    assert(ShipyardAuthoringAuthority::InferMaterialZone("Mat_Seco") == ShipyardMaterialZone::SecondaryPaint);
    assert(ShipyardAuthoringAuthority::InferMaterialZone("Glow_Blu") == ShipyardMaterialZone::EmissionPrimary);
    assert(ShipyardAuthoringAuthority::InferMaterialZone("Glass") == ShipyardMaterialZone::Glass);

    // Existing recipe bridge: module choice can change without touching placement.
    ShipyardModuleRecord record149;
    record149.source.moduleId = wing149->definitionId;
    record149.moduleClass = ShipyardModuleClass::Wing;
    record149.semantic = ShipyardModuleSemantic::Wing;
    record149.size = ShipyardModuleSize::M;
    record149.sockets.push_back({"mount", "lateral_mount", 0,0,0, 1,0,0, 0});
    ShipyardModuleRecord record156 = record149;
    record156.source.moduleId = wing2.definitionId;
    std::vector<ShipyardModuleRecord> existingCatalog{record149, record156};
    ProceduralShipVisualRecipe recipe;
    recipe.recipeId = "bridge.test";
    recipe.seed = 991;
    VisualModulePlacement pLeft; pLeft.moduleId = wing149->definitionId; pLeft.x = -2.0f; pLeft.y = 1.0f; pLeft.rollDegrees = 90.0f;
    VisualModulePlacement pRight = pLeft; pRight.x = 2.0f;
    recipe.modules = {pLeft, pRight};
    auto projected = ShipyardAuthoringBridge::BuildPreservedBlueprint(authority, recipe, existingCatalog);
    assert(projected.slots.size() == 2);
    const float originalX = recipe.modules[0].x;
    auto explicitSwap = authority.PreviewReplacement(projected, projected.slots[0].slotId, wing2.definitionId);
    assert(explicitSwap.valid);
    assert(authority.CommitReroll(projected, explicitSwap));
    assert(ShipyardAuthoringBridge::ApplyModuleChoicesPreservingPlacement(projected, recipe));
    assert(recipe.modules[0].moduleId == wing2.definitionId);
    assert(recipe.modules[0].x == originalX);
    assert(recipe.modules[0].rollDegrees == 90.0f); // bridge did not silently move/rotate it

    auto issues = authority.ValidateBlueprint(ship);
    bool hasErrors = false;
    for (const auto& issue : issues) if (issue.severity == ShipyardValidationSeverity::Error) hasErrors = true;
    assert(!hasErrors);

    std::cout << "PASS497-506 ShipyardAuthoringAuthority smoke: PASS\n";
    return 0;
}
