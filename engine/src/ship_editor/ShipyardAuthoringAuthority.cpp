#include "ship_editor/ShipyardAuthoringAuthority.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <set>
#include <sstream>
#include <unordered_set>

namespace subspace {
namespace {

bool HasToken(const std::string& value, const std::string& token) {
    return value.find(token) != std::string::npos;
}

float SimilarityScore(const ShipyardAuthoringDefinition& current,
                      const ShipyardAuthoringDefinition& candidate,
                      ShipyardRerollMode mode) {
    if (!candidate.generationEnabled || candidate.certification == ShipyardCertificationState::Quarantined) return -1.0f;
    if (candidate.definitionId == current.definitionId) return -1.0f;

    float score = 0.0f;
    if (candidate.primaryClass == current.primaryClass) score += 0.25f;
    if (candidate.subtype == current.subtype) score += 0.30f;
    if (!current.family.empty() && candidate.family == current.family) score += 0.15f;
    if (!current.sizeClass.empty() && candidate.sizeClass == current.sizeClass) score += 0.10f;
    if (!current.functionalRole.empty() && candidate.functionalRole == current.functionalRole) score += 0.10f;
    if (!current.rerollGroup.empty() && candidate.rerollGroup == current.rerollGroup) score += 0.10f;

    switch (mode) {
        case ShipyardRerollMode::SameFamily:
            if (current.family.empty() || candidate.family != current.family) return -1.0f;
            break;
        case ShipyardRerollMode::SameRole:
            if (candidate.subtype != current.subtype && candidate.functionalRole != current.functionalRole) return -1.0f;
            break;
        case ShipyardRerollMode::Similar:
            if (candidate.primaryClass != current.primaryClass) return -1.0f;
            if (candidate.subtype != current.subtype && candidate.rerollGroup != current.rerollGroup) score -= 0.20f;
            break;
        case ShipyardRerollMode::AnyCompatible:
        case ShipyardRerollMode::Pair:
        case ShipyardRerollMode::Branch:
        case ShipyardRerollMode::AllUnlocked:
            if (candidate.primaryClass != current.primaryClass && candidate.functionalRole != current.functionalRole) return -1.0f;
            break;
    }

    if (candidate.certification == ShipyardCertificationState::Certified) score += 0.08f;
    else if (candidate.certification == ShipyardCertificationState::Reviewed) score += 0.04f;
    else if (candidate.certification == ShipyardCertificationState::Unreviewed) score -= 0.08f;

    score += std::max(0.0f, std::min(candidate.generationWeight, 2.0f)) * 0.01f;
    return score;
}

} // namespace

bool ShipyardSelectionAuthority::SelectSlot(const ShipyardResolvedBlueprint& blueprint, const std::string& slotId) {
    auto it = std::find_if(blueprint.slots.begin(), blueprint.slots.end(), [&](const ShipyardSlot& slot) { return slot.slotId == slotId; });
    if (it == blueprint.slots.end()) return false;
    m_selectedSlotId = it->slotId;
    m_selectedDefinitionId = it->moduleDefinitionId;
    ++m_generation;
    return true;
}

void ShipyardSelectionAuthority::Clear() {
    m_selectedSlotId.clear();
    m_selectedDefinitionId.clear();
    ++m_generation;
}

void ShipyardAuthoringAuthority::RegisterDefinition(ShipyardAuthoringDefinition definition) {
    if (definition.definitionId.empty()) return;
    m_definitions[definition.definitionId] = std::move(definition);
}

bool ShipyardAuthoringAuthority::HasDefinition(const std::string& definitionId) const {
    return m_definitions.find(definitionId) != m_definitions.end();
}

const ShipyardAuthoringDefinition* ShipyardAuthoringAuthority::FindDefinition(const std::string& definitionId) const {
    const auto it = m_definitions.find(definitionId);
    return it == m_definitions.end() ? nullptr : &it->second;
}

ShipyardAuthoringDefinition ShipyardAuthoringAuthority::InferDefinition(const std::string& definitionId, const std::string& displayName) const {
    ShipyardAuthoringDefinition out;
    out.definitionId = definitionId;
    out.displayName = displayName.empty() ? definitionId : displayName;
    out.certification = ShipyardCertificationState::Inferred;
    out.frame.source = ShipyardOrientationSource::Inferred;
    out.generationWeight = 1.0f;
    out.generationEnabled = true;

    const std::string n = Lower(definitionId + " " + displayName);

    if (HasToken(n, "bridge") || HasToken(n, "cockpit")) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Command;
        out.subtype = HasToken(n, "cockpit") ? ShipyardAuthoringSubtype::Cockpit : ShipyardAuthoringSubtype::Bridge;
        out.functionalRole = "Command";
        out.placementZone = "DorsalForward";
        out.importance = ShipyardStructuralImportance::RequiredFunctional;
        out.capabilities = {"Command"};
        if (HasToken(n, "antenna") || HasToken(n, "radar") || HasToken(n, "dish")) out.capabilities.push_back("Sensor");
    } else if (HasToken(n, "wing") || HasToken(n, "fin") || HasToken(n, "keel")) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Surface;
        out.functionalRole = "StructuralSurface";
        out.importance = ShipyardStructuralImportance::Structural;
        out.frame.lateralSurface = true;
        out.frame.rootNormal = {-1.0f, 0.0f, 0.0f};
        out.frame.outward = {1.0f, 0.0f, 0.0f};
        out.frame.forward = {0.0f, 1.0f, 0.0f};
        out.frame.up = {0.0f, 0.0f, 1.0f};
        if (HasToken(n, "topwing") || HasToken(n, "vertical") || HasToken(n, "dorsal")) {
            out.subtype = ShipyardAuthoringSubtype::VerticalStabilizer;
            out.placementZone = "DorsalAft";
            out.frame.lateralSurface = false;
        } else if (HasToken(n, "keel") || HasToken(n, "ventral")) {
            out.subtype = ShipyardAuthoringSubtype::Keel;
            out.placementZone = "VentralAft";
            out.frame.lateralSurface = false;
        } else {
            out.subtype = ShipyardAuthoringSubtype::LateralWing;
            out.placementZone = "LateralExterior";
        }
        out.family = "surface." + SubtypeName(out.subtype);
        out.rerollGroup = "surface." + Lower(SubtypeName(out.subtype)) + "." + Lower(out.sizeClass);
        out.capabilities = {"StructuralSurface", "Mirrorable"};
    } else if (HasToken(n, "engine") || HasToken(n, "thruster") || HasToken(n, "nozzle") || HasToken(n, "rcs")) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Propulsion;
        out.placementZone = "AftExterior";
        out.importance = ShipyardStructuralImportance::RequiredFunctional;
        if (HasToken(n, "strut")) out.subtype = ShipyardAuthoringSubtype::EngineStrut;
        else if (HasToken(n, "support") || HasToken(n, "bracket")) out.subtype = ShipyardAuthoringSubtype::EngineSupport;
        else if (HasToken(n, "housing") || HasToken(n, "body")) out.subtype = ShipyardAuthoringSubtype::EngineHousing;
        else if (HasToken(n, "nozzle") || HasToken(n, "trumpet")) out.subtype = ShipyardAuthoringSubtype::EngineNozzle;
        else if (HasToken(n, "rcs") || HasToken(n, "thruster")) out.subtype = ShipyardAuthoringSubtype::ManeuverThruster;
        else out.subtype = ShipyardAuthoringSubtype::MainEngine;
        out.functionalRole = (out.subtype == ShipyardAuthoringSubtype::MainEngine) ? "MainEngine" : "PropulsionStructure";
        if (out.subtype == ShipyardAuthoringSubtype::MainEngine || out.subtype == ShipyardAuthoringSubtype::ManeuverThruster)
            out.capabilities = {"Thrust"};
        else out.capabilities = {"StructuralSupport"};
    } else if (HasToken(n, "gun") || HasToken(n, "turret") || HasToken(n, "hardpoint") || HasToken(n, "launcher")) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Weapon;
        out.importance = ShipyardStructuralImportance::OptionalFunctional;
        if (HasToken(n, "base") || HasToken(n, "hardpoint") || HasToken(n, "platform")) out.subtype = ShipyardAuthoringSubtype::WeaponMount;
        else if (HasToken(n, "launcher")) out.subtype = ShipyardAuthoringSubtype::Launcher;
        else out.subtype = ShipyardAuthoringSubtype::Turret;
        out.functionalRole = (out.subtype == ShipyardAuthoringSubtype::WeaponMount) ? "WeaponMount" : "Weapon";
        out.capabilities = {out.functionalRole};
    } else if (HasToken(n, "mast") || HasToken(n, "radar") || HasToken(n, "dish") || HasToken(n, "antenna") || HasToken(n, "telescope")) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Electronics;
        out.importance = ShipyardStructuralImportance::OptionalFunctional;
        if (HasToken(n, "radar")) out.subtype = ShipyardAuthoringSubtype::Radar;
        else if (HasToken(n, "dish")) out.subtype = ShipyardAuthoringSubtype::Dish;
        else if (HasToken(n, "antenna")) out.subtype = ShipyardAuthoringSubtype::Antenna;
        else out.subtype = ShipyardAuthoringSubtype::SensorMast;
        out.functionalRole = "Sensor";
        out.placementZone = "ExteriorExposed";
        out.capabilities = {"Sensor"};
    } else if (HasToken(n, "cargo") || HasToken(n, "tank") || HasToken(n, "hangar") || HasToken(n, "hanger") || HasToken(n, "mining") || HasToken(n, "salvage")) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Utility;
        out.importance = ShipyardStructuralImportance::OptionalFunctional;
        if (HasToken(n, "cargo")) out.subtype = ShipyardAuthoringSubtype::Cargo;
        else if (HasToken(n, "tank")) out.subtype = ShipyardAuthoringSubtype::Tank;
        else if (HasToken(n, "hangar") || HasToken(n, "hanger")) out.subtype = ShipyardAuthoringSubtype::Hangar;
        else if (HasToken(n, "mining")) out.subtype = ShipyardAuthoringSubtype::Mining;
        else if (HasToken(n, "salvage")) out.subtype = ShipyardAuthoringSubtype::Salvage;
        out.functionalRole = SubtypeName(out.subtype);
        out.capabilities = {out.functionalRole};
    } else if (HasToken(n, "adapter") || HasToken(n, "joined") || HasToken(n, "coupler")) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Adapter;
        out.subtype = ShipyardAuthoringSubtype::HullAdapter;
        out.functionalRole = "StructuralAdapter";
        out.importance = ShipyardStructuralImportance::Structural;
        out.capabilities = {"Structural"};
    } else if (HasToken(n, "vent") || HasToken(n, "panel") || HasToken(n, "grille") || HasToken(n, "detail") || HasToken(n, "window")) {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Detail;
        out.importance = ShipyardStructuralImportance::Decorative;
        if (HasToken(n, "vent")) out.subtype = ShipyardAuthoringSubtype::Vent;
        else if (HasToken(n, "window")) out.subtype = ShipyardAuthoringSubtype::Window;
        else out.subtype = ShipyardAuthoringSubtype::Panel;
        out.functionalRole = "ExteriorDetail";
        out.capabilities = {"Decorative"};
    } else {
        out.primaryClass = ShipyardAuthoringPrimaryClass::Hull;
        out.importance = ShipyardStructuralImportance::Structural;
        out.functionalRole = "HullStructure";
        out.capabilities = {"Structural"};
        if (HasToken(n, "bow") || HasToken(n, "nose")) out.subtype = ShipyardAuthoringSubtype::HullBow;
        else if (HasToken(n, "stern") || HasToken(n, "aft")) out.subtype = ShipyardAuthoringSubtype::HullStern;
        else if (HasToken(n, "side")) out.subtype = ShipyardAuthoringSubtype::HullSide;
        else out.subtype = ShipyardAuthoringSubtype::HullMid;
    }

    if (out.family.empty()) out.family = Lower(PrimaryClassName(out.primaryClass)) + "." + Lower(SubtypeName(out.subtype));
    if (out.rerollGroup.empty()) out.rerollGroup = out.family + "." + Lower(out.sizeClass);
    return out;
}

void ShipyardAuthoringAuthority::RegisterBuiltInCertifiedOverrides() {
    // Certification fixture derived from the known wing_149 screenshot. All runtime logic remains generic;
    // the asset-specific correction lives only in metadata/registry authority.
    ShipyardAuthoringDefinition wing = InferDefinition("shipyard_a_wing_149_shipyard_wing_003_miscfinhanger");
    wing.displayName = "Fin Hanger Wing";
    wing.primaryClass = ShipyardAuthoringPrimaryClass::Surface;
    wing.subtype = ShipyardAuthoringSubtype::LateralWing;
    wing.family = "greyoxide.surface.fin_hanger";
    wing.functionalRole = "StructuralSurface";
    wing.placementZone = "LateralExterior";
    wing.rerollGroup = "greyoxide.surface.lateral.m";
    wing.certification = ShipyardCertificationState::Certified;
    wing.frame.source = ShipyardOrientationSource::Certified;
    // The preserved source mesh is thin on local X and broad across Y/Z. Treat local
    // -Z as the authored root edge, +Z as the tip/outward axis, +Y as ship-forward,
    // and +X as source-up. This encodes the required 90-degree lay-flat correction
    // as metadata rather than a renderer special case.
    wing.frame.rootNormal = {0.0f, 0.0f, -1.0f};
    wing.frame.outward = {0.0f, 0.0f, 1.0f};
    wing.frame.forward = {0.0f, 1.0f, 0.0f};
    wing.frame.up = {1.0f, 0.0f, 0.0f};
    wing.frame.lateralSurface = true;
    wing.frame.preferredMaxRollDegrees = 25.0f;
    wing.compatibleSocketTypes = {"lateral_mount", "wing_root", "hull_lateral"};
    wing.capabilities = {"StructuralSurface", "Mirrorable"};
    wing.materialZones["mat_main"] = ShipyardMaterialZone::PrimaryPaint;
    wing.materialZones["mat_seco"] = ShipyardMaterialZone::SecondaryPaint;
    wing.materialZones["mat_dark"] = ShipyardMaterialZone::StructuralDark;
    wing.materialZones["mat_metall"] = ShipyardMaterialZone::MetalLight;
    wing.materialZones["mat_metald"] = ShipyardMaterialZone::MetalDark;
    RegisterDefinition(std::move(wing));
}

ShipyardPlacementScore ShipyardAuthoringAuthority::ScorePlacement(const ShipyardAuthoringDefinition& definition,
                                                                  const ShipyardPlacementEvidence& e) const {
    ShipyardPlacementScore out;
    out.socket = Clamp01(e.socketCompatibility);
    const float root = Clamp01(e.rootFacesParent);
    const float outward = Clamp01(e.outwardFromCenterline);
    const float forward = Clamp01(e.forwardAlignment);
    const float up = Clamp01(e.upAlignment);
    out.orientation = (root * 0.35f) + (outward * 0.30f) + (forward * 0.20f) + (up * 0.15f);
    out.role = Clamp01(e.roleLocation);
    out.clearance = Clamp01(e.clearance);
    out.pairing = Clamp01(e.pairing);

    if (e.disconnected) { out.hardInvalid = true; out.reasons.push_back("disconnected structural module"); }
    if (out.socket <= 0.01f) { out.hardInvalid = true; out.reasons.push_back("illegal socket pairing"); }
    if (e.structuralIntersectionRatio >= 0.60f) { out.hardInvalid = true; out.reasons.push_back("severe structural intersection"); }
    if (e.exhaustFullyBlocked && ContainsCapability(definition, "Thrust")) { out.hardInvalid = true; out.reasons.push_back("propulsion exhaust fully blocked"); }
    if (e.hangarMouthFullyBlocked && definition.subtype == ShipyardAuthoringSubtype::Hangar) { out.hardInvalid = true; out.reasons.push_back("hangar mouth fully blocked"); }
    if (e.bridgeBuried && definition.primaryClass == ShipyardAuthoringPrimaryClass::Command) { out.hardInvalid = true; out.reasons.push_back("command module buried in geometry"); }

    if (definition.frame.lateralSurface) {
        if (root < 0.50f) { out.hardInvalid = true; out.reasons.push_back("lateral surface root is not hull-facing"); }
        if (outward < 0.50f) { out.hardInvalid = true; out.reasons.push_back("lateral surface tip does not point away from centerline"); }
        if (up < 0.55f) { out.hardInvalid = true; out.reasons.push_back("lateral surface is outside allowed roll/orientation envelope"); }
    }

    if (!out.hardInvalid) {
        if (out.role < 0.45f) out.reasons.push_back("unusual role location");
        if (out.clearance < 0.45f) out.reasons.push_back("tight but non-fatal clearance");
        if (out.pairing < 0.45f) out.reasons.push_back("pair relationship is unusual or incomplete");
    }

    out.confidence = (out.socket * 0.25f) + (out.orientation * 0.30f) + (out.role * 0.15f) +
                     (out.clearance * 0.20f) + (out.pairing * 0.10f);
    if (out.hardInvalid) out.confidence = std::min(out.confidence, 0.49f);
    return out;
}

ShipyardRepairAction ShipyardAuthoringAuthority::RecommendRepair(const ShipyardPlacementScore& score,
                                                                 bool sameModuleCanRotate,
                                                                 bool sameParentHasAlternateSocket,
                                                                 bool nearbyParentAvailable,
                                                                 bool sameRoleReplacementAvailable) const {
    if (!score.hardInvalid) return ShipyardRepairAction::Keep;
    if (sameModuleCanRotate) return ShipyardRepairAction::CorrectOrientation;
    if (sameParentHasAlternateSocket) return ShipyardRepairAction::AlternateSameParentSocket;
    if (nearbyParentAvailable) return ShipyardRepairAction::AlternateNearbyParent;
    if (sameRoleReplacementAvailable) return ShipyardRepairAction::SameRoleReplacement;
    return ShipyardRepairAction::Unresolved;
}

std::vector<ShipyardRerollCandidate> ShipyardAuthoringAuthority::BuildRerollPool(const ShipyardResolvedBlueprint& /*blueprint*/,
                                                                                 const ShipyardSlot& slot,
                                                                                 ShipyardRerollMode mode) const {
    std::vector<ShipyardRerollCandidate> out;
    const auto* current = FindDefinition(slot.moduleDefinitionId);
    if (!current || slot.locks.module) return out;

    std::unordered_set<std::string> recent(slot.rerollHistory.begin(), slot.rerollHistory.end());
    for (const auto& pair : m_definitions) {
        const auto& candidate = pair.second;
        float score = SimilarityScore(*current, candidate, mode);
        if (score < 0.0f) continue;
        if (recent.find(candidate.definitionId) != recent.end()) score -= 0.20f;

        if (!current->compatibleSocketTypes.empty() && !candidate.compatibleSocketTypes.empty()) {
            bool overlap = false;
            for (const auto& a : current->compatibleSocketTypes) {
                if (std::find(candidate.compatibleSocketTypes.begin(), candidate.compatibleSocketTypes.end(), a) != candidate.compatibleSocketTypes.end()) {
                    overlap = true; break;
                }
            }
            if (!overlap && slot.locks.socket) continue;
            if (overlap) score += 0.10f;
        }
        out.push_back({candidate.definitionId, score});
    }

    std::sort(out.begin(), out.end(), [](const ShipyardRerollCandidate& a, const ShipyardRerollCandidate& b) {
        if (std::fabs(a.score - b.score) > 0.0001f) return a.score > b.score;
        return a.definitionId < b.definitionId;
    });
    return out;
}

ShipyardRerollPreview ShipyardAuthoringAuthority::PreviewReroll(const ShipyardResolvedBlueprint& blueprint,
                                                                const std::string& slotId,
                                                                ShipyardRerollMode mode) const {
    ShipyardRerollPreview out;
    const auto* slot = FindSlot(blueprint, slotId);
    if (!slot || slot->locks.module) {
        out.explanation = slot ? "slot module is locked" : "slot not found";
        return out;
    }
    auto pool = BuildRerollPool(blueprint, *slot, mode);
    if (pool.empty()) { out.explanation = "no compatible reroll candidates"; return out; }

    const uint64_t key = Mix(blueprint.shipSeed ^ StableHash(slot->slotId) ^ (static_cast<uint64_t>(slot->rerollIndex + 1) * 0x9E3779B97F4A7C15ULL));
    const size_t topBand = std::min<size_t>(pool.size(), std::max<size_t>(1, std::min<size_t>(6, pool.size())));
    const size_t index = static_cast<size_t>(key % topBand);
    const auto& candidate = pool[index];

    out.valid = true;
    out.slotId = slot->slotId;
    out.previousDefinitionId = slot->moduleDefinitionId;
    out.candidateDefinitionId = candidate.definitionId;
    out.rerollIndex = slot->rerollIndex + 1;
    out.candidateScore = candidate.score;
    std::ostringstream ss;
    ss << "compatible replacement selected without moving slot '" << slot->slotId << "'";
    if (slot->locks.socket) ss << "; socket pinned";
    if (slot->locks.children) ss << "; child branch protected";
    out.explanation = ss.str();
    return out;
}

ShipyardRerollPreview ShipyardAuthoringAuthority::PreviewReplacement(const ShipyardResolvedBlueprint& blueprint,
                                                                     const std::string& slotId,
                                                                     const std::string& candidateDefinitionId) const {
    ShipyardRerollPreview out;
    const auto* slot = FindSlot(blueprint, slotId);
    const auto* current = slot ? FindDefinition(slot->moduleDefinitionId) : nullptr;
    const auto* candidate = FindDefinition(candidateDefinitionId);
    if (!slot || !current || !candidate) {
        out.explanation = "slot/current/candidate definition not found";
        return out;
    }
    if (slot->locks.module) { out.explanation = "slot module is locked"; return out; }
    const float score = SimilarityScore(*current, *candidate, ShipyardRerollMode::AnyCompatible);
    if (score < 0.0f) { out.explanation = "requested replacement is incompatible with the slot role"; return out; }
    if (slot->locks.socket && !current->compatibleSocketTypes.empty() && !candidate->compatibleSocketTypes.empty()) {
        bool compatible = false;
        for (const auto& type : current->compatibleSocketTypes) {
            if (std::find(candidate->compatibleSocketTypes.begin(), candidate->compatibleSocketTypes.end(), type) != candidate->compatibleSocketTypes.end()) {
                compatible = true;
                break;
            }
        }
        if (!compatible) { out.explanation = "requested replacement does not satisfy pinned socket compatibility"; return out; }
    }
    out.valid = true;
    out.slotId = slot->slotId;
    out.previousDefinitionId = slot->moduleDefinitionId;
    out.candidateDefinitionId = candidateDefinitionId;
    out.rerollIndex = slot->rerollIndex + 1;
    out.candidateScore = score;
    out.explanation = "explicit compatible replacement preview; slot identity and parent are preserved";
    return out;
}

std::vector<ShipyardRerollPreview> ShipyardAuthoringAuthority::PreviewRerollOperation(const ShipyardResolvedBlueprint& blueprint,
                                                                                      const std::string& slotId,
                                                                                      ShipyardRerollMode mode) const {
    std::vector<ShipyardRerollPreview> out;
    const auto* root = FindSlot(blueprint, slotId);
    if (!root) return out;

    if (mode != ShipyardRerollMode::Pair && mode != ShipyardRerollMode::Branch && mode != ShipyardRerollMode::AllUnlocked) {
        auto preview = PreviewReroll(blueprint, slotId, mode);
        if (preview.valid) out.push_back(std::move(preview));
        return out;
    }

    if (mode == ShipyardRerollMode::Pair) {
        auto first = PreviewReroll(blueprint, slotId, ShipyardRerollMode::Similar);
        if (!first.valid) return out;
        out.push_back(first);
        if (root->mirrorPartnerSlotId.empty()) return out;
        const auto* partner = FindSlot(blueprint, root->mirrorPartnerSlotId);
        if (!partner || partner->locks.module) return out;
        // Prefer the exact same candidate for a mirrored-identical pair. Fall back to a
        // independently ranked compatible candidate only when the pair is explicitly compatible.
        auto second = PreviewReplacement(blueprint, partner->slotId, first.candidateDefinitionId);
        if (!second.valid && root->pairMode == ShipyardPairMode::MirroredCompatible)
            second = PreviewReroll(blueprint, partner->slotId, ShipyardRerollMode::Similar);
        if (second.valid) out.push_back(std::move(second));
        return out;
    }

    std::vector<const ShipyardSlot*> targets;
    if (mode == ShipyardRerollMode::AllUnlocked) {
        for (const auto& slot : blueprint.slots) if (!slot.locks.module) targets.push_back(&slot);
    } else {
        std::vector<std::string> frontier{slotId};
        std::unordered_set<std::string> visited;
        while (!frontier.empty()) {
            const std::string currentId = frontier.back();
            frontier.pop_back();
            if (!visited.insert(currentId).second) continue;
            const auto* current = FindSlot(blueprint, currentId);
            if (!current) continue;
            if (!current->locks.module) targets.push_back(current);
            if (current->locks.children && currentId != slotId) continue;
            for (const auto& child : blueprint.slots) if (child.parentSlotId == currentId) frontier.push_back(child.slotId);
        }
    }

    for (const auto* target : targets) {
        auto preview = PreviewReroll(blueprint, target->slotId, ShipyardRerollMode::Similar);
        if (preview.valid) out.push_back(std::move(preview));
    }
    return out;
}

bool ShipyardAuthoringAuthority::CommitReroll(ShipyardResolvedBlueprint& blueprint, const ShipyardRerollPreview& preview) const {
    if (!preview.valid) return false;
    auto* slot = FindSlot(blueprint, preview.slotId);
    if (!slot || slot->locks.module || slot->moduleDefinitionId != preview.previousDefinitionId) return false;
    slot->rerollHistory.push_back(slot->moduleDefinitionId);
    if (slot->rerollHistory.size() > 16) slot->rerollHistory.erase(slot->rerollHistory.begin());
    slot->moduleDefinitionId = preview.candidateDefinitionId;
    slot->rerollIndex = preview.rerollIndex;
    return true;
}

bool ShipyardAuthoringAuthority::CommitRerollOperation(ShipyardResolvedBlueprint& blueprint,
                                                               const std::vector<ShipyardRerollPreview>& previews) const {
    if (previews.empty()) return false;
    ShipyardResolvedBlueprint candidate = blueprint;
    std::unordered_set<std::string> touched;
    for (const auto& preview : previews) {
        if (!preview.valid || !touched.insert(preview.slotId).second || !CommitReroll(candidate, preview)) return false;
    }
    blueprint = std::move(candidate);
    return true;
}

bool ShipyardAuthoringAuthority::RestorePreviousReroll(ShipyardResolvedBlueprint& blueprint, const std::string& slotId) const {
    auto* slot = FindSlot(blueprint, slotId);
    if (!slot || slot->locks.module || slot->rerollHistory.empty()) return false;
    const std::string previous = slot->rerollHistory.back();
    slot->rerollHistory.pop_back();
    slot->moduleDefinitionId = previous;
    if (slot->rerollIndex > 0) --slot->rerollIndex;
    return true;
}

bool ShipyardAuthoringAuthority::LockSlot(ShipyardResolvedBlueprint& blueprint, const std::string& slotId, ShipyardSlotLocks locks) const {
    auto* slot = FindSlot(blueprint, slotId);
    if (!slot) return false;
    slot->locks = locks;
    return true;
}

bool ShipyardAuthoringAuthority::PairSlots(ShipyardResolvedBlueprint& blueprint,
                                           const std::string& firstSlotId,
                                           const std::string& secondSlotId,
                                           ShipyardPairMode mode) const {
    auto* a = FindSlot(blueprint, firstSlotId);
    auto* b = FindSlot(blueprint, secondSlotId);
    if (!a || !b || a == b) return false;
    a->mirrorPartnerSlotId = b->slotId;
    b->mirrorPartnerSlotId = a->slotId;
    a->pairMode = mode;
    b->pairMode = mode;
    return true;
}

std::vector<ShipyardValidationIssue> ShipyardAuthoringAuthority::ValidateBlueprint(const ShipyardResolvedBlueprint& blueprint) const {
    std::vector<ShipyardValidationIssue> out;
    std::unordered_set<std::string> ids;
    for (const auto& slot : blueprint.slots) {
        if (slot.slotId.empty()) {
            out.push_back({ShipyardValidationSeverity::Error, {}, slot.moduleDefinitionId, "EMPTY_SLOT_ID", "ship slot has no stable identity"});
            continue;
        }
        if (!ids.insert(slot.slotId).second)
            out.push_back({ShipyardValidationSeverity::Error, slot.slotId, slot.moduleDefinitionId, "DUPLICATE_SLOT_ID", "duplicate stable ship slot identity"});
        if (!HasDefinition(slot.moduleDefinitionId))
            out.push_back({ShipyardValidationSeverity::Error, slot.slotId, slot.moduleDefinitionId, "UNKNOWN_MODULE", "slot references an unregistered module definition"});
    }
    for (const auto& slot : blueprint.slots) {
        if (!slot.parentSlotId.empty() && !FindSlot(blueprint, slot.parentSlotId))
            out.push_back({ShipyardValidationSeverity::Error, slot.slotId, slot.moduleDefinitionId, "MISSING_PARENT", "slot parent does not exist"});
        if (!slot.mirrorPartnerSlotId.empty()) {
            const auto* partner = FindSlot(blueprint, slot.mirrorPartnerSlotId);
            if (!partner)
                out.push_back({ShipyardValidationSeverity::Error, slot.slotId, slot.moduleDefinitionId, "BROKEN_PAIR", "mirror partner slot does not exist"});
            else if (partner->mirrorPartnerSlotId != slot.slotId)
                out.push_back({ShipyardValidationSeverity::Warning, slot.slotId, slot.moduleDefinitionId, "ONE_WAY_PAIR", "pair relationship is not reciprocal"});
        } else if (slot.pairMode == ShipyardPairMode::MirroredIdentical || slot.pairMode == ShipyardPairMode::MirroredCompatible) {
            out.push_back({ShipyardValidationSeverity::Error, slot.slotId, slot.moduleDefinitionId, "PAIR_REQUIRED", "paired slot is missing its mirror partner"});
        }
    }
    return out;
}

std::vector<ShipyardValidationIssue> ShipyardAuthoringAuthority::ValidatePlacement(const ShipyardSlot& slot,
                                                                                   const ShipyardPlacementScore& score) const {
    std::vector<ShipyardValidationIssue> out;
    for (const auto& reason : score.reasons) {
        out.push_back({score.hardInvalid ? ShipyardValidationSeverity::Error : ShipyardValidationSeverity::Warning,
                       slot.slotId, slot.moduleDefinitionId,
                       score.hardInvalid ? "INVALID_PLACEMENT" : "PLACEMENT_WARNING", reason});
    }
    if (!score.hardInvalid && score.confidence < 0.60f)
        out.push_back({ShipyardValidationSeverity::Warning, slot.slotId, slot.moduleDefinitionId,
                       "LOW_PLACEMENT_CONFIDENCE", "placement confidence is below the preferred threshold"});
    return out;
}

ShipyardMaterialZone ShipyardAuthoringAuthority::InferMaterialZone(const std::string& sourceMaterialName) {
    const std::string n = Lower(sourceMaterialName);
    if (HasToken(n, "glass") || HasToken(n, "window")) return ShipyardMaterialZone::Glass;
    if (HasToken(n, "glow") || HasToken(n, "emiss")) {
        if (HasToken(n, "ora") || HasToken(n, "red") || HasToken(n, "amber")) return ShipyardMaterialZone::EmissionSecondary;
        return ShipyardMaterialZone::EmissionPrimary;
    }
    if (HasToken(n, "hazard")) return ShipyardMaterialZone::Hazard;
    if (HasToken(n, "metald") || HasToken(n, "darkmetal")) return ShipyardMaterialZone::MetalDark;
    if (HasToken(n, "metall") || HasToken(n, "lightmetal")) return ShipyardMaterialZone::MetalLight;
    if (HasToken(n, "metal")) return ShipyardMaterialZone::MetalDark;
    if (HasToken(n, "seco") || HasToken(n, "secondary")) return ShipyardMaterialZone::SecondaryPaint;
    if (HasToken(n, "accent") || HasToken(n, "trim")) return ShipyardMaterialZone::AccentPaint;
    if (HasToken(n, "dark") || HasToken(n, "struct")) return ShipyardMaterialZone::StructuralDark;
    if (HasToken(n, "main") || HasToken(n, "primary") || HasToken(n, "paint")) return ShipyardMaterialZone::PrimaryPaint;
    return ShipyardMaterialZone::Unmapped;
}

std::string ShipyardAuthoringAuthority::PrimaryClassName(ShipyardAuthoringPrimaryClass value) {
    switch (value) {
        case ShipyardAuthoringPrimaryClass::Hull: return "Hull";
        case ShipyardAuthoringPrimaryClass::Surface: return "Surface";
        case ShipyardAuthoringPrimaryClass::Propulsion: return "Propulsion";
        case ShipyardAuthoringPrimaryClass::Command: return "Command";
        case ShipyardAuthoringPrimaryClass::Weapon: return "Weapon";
        case ShipyardAuthoringPrimaryClass::Electronics: return "Electronics";
        case ShipyardAuthoringPrimaryClass::Utility: return "Utility";
        case ShipyardAuthoringPrimaryClass::Adapter: return "Adapter";
        case ShipyardAuthoringPrimaryClass::Detail: return "Detail";
        default: return "Unknown";
    }
}

std::string ShipyardAuthoringAuthority::SubtypeName(ShipyardAuthoringSubtype value) {
#define CASE_NAME(x) case ShipyardAuthoringSubtype::x: return #x
    switch (value) {
        CASE_NAME(HullBow); CASE_NAME(HullMid); CASE_NAME(HullStern); CASE_NAME(HullSpine); CASE_NAME(HullSide); CASE_NAME(HullExtension); CASE_NAME(StructuralSupport);
        CASE_NAME(LateralWing); CASE_NAME(Winglet); CASE_NAME(HorizontalStabilizer); CASE_NAME(VerticalStabilizer); CASE_NAME(DorsalFin); CASE_NAME(VentralFin); CASE_NAME(Keel);
        CASE_NAME(MainEngine); CASE_NAME(SecondaryEngine); CASE_NAME(ManeuverThruster); CASE_NAME(RetroThruster); CASE_NAME(EngineHousing); CASE_NAME(EngineNacelle); CASE_NAME(EngineSupport); CASE_NAME(EngineStrut); CASE_NAME(EnginePylon); CASE_NAME(EngineNozzle); CASE_NAME(ThrustVane);
        CASE_NAME(Cockpit); CASE_NAME(Bridge); CASE_NAME(CompactBridge); CASE_NAME(ForwardBridge); CASE_NAME(SideBridge); CASE_NAME(CommandSuperstructure);
        CASE_NAME(WeaponMount); CASE_NAME(TurretBase); CASE_NAME(Turret); CASE_NAME(PointDefense); CASE_NAME(Launcher);
        CASE_NAME(SensorArray); CASE_NAME(SensorMast); CASE_NAME(Radar); CASE_NAME(Dish); CASE_NAME(Antenna); CASE_NAME(Communications);
        CASE_NAME(Cargo); CASE_NAME(Tank); CASE_NAME(Storage); CASE_NAME(Hangar); CASE_NAME(Docking); CASE_NAME(Industrial); CASE_NAME(Mining); CASE_NAME(Salvage); CASE_NAME(Drone); CASE_NAME(Service);
        CASE_NAME(HullAdapter); CASE_NAME(SizeTransition); CASE_NAME(StructuralCoupler);
        CASE_NAME(Panel); CASE_NAME(Vent); CASE_NAME(Intake); CASE_NAME(Grille); CASE_NAME(Trim); CASE_NAME(Greeble); CASE_NAME(Window);
        default: return "Unknown";
    }
#undef CASE_NAME
}

std::string ShipyardAuthoringAuthority::MaterialZoneName(ShipyardMaterialZone value) {
#define CASE_ZONE(x) case ShipyardMaterialZone::x: return #x
    switch (value) {
        CASE_ZONE(PrimaryPaint); CASE_ZONE(SecondaryPaint); CASE_ZONE(AccentPaint); CASE_ZONE(StructuralDark);
        CASE_ZONE(MetalLight); CASE_ZONE(MetalDark); CASE_ZONE(Glass); CASE_ZONE(EmissionPrimary); CASE_ZONE(EmissionSecondary);
        CASE_ZONE(Hazard); CASE_ZONE(Interior); default: return "Unmapped";
    }
#undef CASE_ZONE
}

uint64_t ShipyardAuthoringAuthority::StableHash(const std::string& value) {
    uint64_t hash = 1469598103934665603ULL;
    for (unsigned char c : value) { hash ^= c; hash *= 1099511628211ULL; }
    return hash;
}

uint64_t ShipyardAuthoringAuthority::Mix(uint64_t value) {
    value ^= value >> 30; value *= 0xBF58476D1CE4E5B9ULL;
    value ^= value >> 27; value *= 0x94D049BB133111EBULL;
    value ^= value >> 31; return value;
}

std::string ShipyardAuthoringAuthority::Lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

float ShipyardAuthoringAuthority::Clamp01(float value) { return std::max(0.0f, std::min(1.0f, value)); }

bool ShipyardAuthoringAuthority::ContainsCapability(const ShipyardAuthoringDefinition& definition, const std::string& capability) {
    return std::find(definition.capabilities.begin(), definition.capabilities.end(), capability) != definition.capabilities.end();
}

ShipyardSlot* ShipyardAuthoringAuthority::FindSlot(ShipyardResolvedBlueprint& blueprint, const std::string& slotId) {
    auto it = std::find_if(blueprint.slots.begin(), blueprint.slots.end(), [&](const ShipyardSlot& s) { return s.slotId == slotId; });
    return it == blueprint.slots.end() ? nullptr : &*it;
}

const ShipyardSlot* ShipyardAuthoringAuthority::FindSlot(const ShipyardResolvedBlueprint& blueprint, const std::string& slotId) {
    auto it = std::find_if(blueprint.slots.begin(), blueprint.slots.end(), [&](const ShipyardSlot& s) { return s.slotId == slotId; });
    return it == blueprint.slots.end() ? nullptr : &*it;
}

} // namespace subspace
