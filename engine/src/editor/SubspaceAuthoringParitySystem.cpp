#include "editor/SubspaceAuthoringParitySystem.h"

#include <unordered_set>

namespace subspace {

std::vector<SubspaceAuthoringContract> SubspaceAuthoringParitySystem::BuildDefaultContracts() {
    return {
        {SubspaceAuthoringDomain::Ship, "subspace.ship.assembly.v2", "Assembly", "ship", true, true, true,
         "Hangar Shipyard / Refit", "Player editing is constrained by ownership, inventory, facility, skill, research, cost and validation."},
        {SubspaceAuthoringDomain::Character, "subspace.character.v1", "CharacterDefinition", "character", true, true, true,
         "Character Creation / Customization", "Player editing exposes approved morphs, cosmetics, apparel, augmentations and portrait controls; rig/source authoring remains editor-only."},
        {SubspaceAuthoringDomain::World, "subspace.world.v2", "WorldDefinition", "world", true, true, false,
         "World Runtime", "Player world changes occur through gameplay construction/settlement systems, not raw world-authoring tools."},
        {SubspaceAuthoringDomain::Interior, "subspace.interior.v2", "InteriorDefinition", "interior", true, true, true,
         "Interior Customization", "Player editing is limited to certified safe zones and never exposes raw portal/nav/pressure authoring."},
        {SubspaceAuthoringDomain::Material, "subspace.material.v2", "MaterialDefinition", "materials", true, true, true,
         "Paint / Livery", "Player editing controls approved paint, pattern, decal and wear layers without shader/source mutation."},
        {SubspaceAuthoringDomain::Animation, "subspace.animation.v1", "AnimationDefinition", "animation", true, true, false,
         "Animation Runtime", "Players consume authored animation state; skeleton, retarget and clip authoring remain editor-only."},
        {SubspaceAuthoringDomain::Pcg, "subspace.pcg.v2", "PcgDefinition", "pcg", true, true, false,
         "Procedural Runtime", "Players consume deterministic generated results; PCG grammar authoring is editor-only."},
        {SubspaceAuthoringDomain::Vfx, "subspace.vfx.v1", "VfxDefinition", "vfx", true, true, false,
         "VFX Runtime", "Players consume effects; graph/source authoring is editor-only."},
        {SubspaceAuthoringDomain::Audio, "subspace.audio.v1", "AudioDefinition", "audio", true, true, false,
         "Audio Runtime", "Players consume cues/mixes; source binding and mix authoring are editor-only."},
        {SubspaceAuthoringDomain::Logic, "subspace.logic.v1", "BehaviorDefinition", "logic", true, true, false,
         "Gameplay Runtime", "Players trigger gameplay logic but do not edit authoritative behavior graphs in normal play."}
    };
}

const SubspaceAuthoringContract* SubspaceAuthoringParitySystem::Find(const std::vector<SubspaceAuthoringContract>& contracts,
                                                                    SubspaceAuthoringDomain domain) {
    for (const auto& contract : contracts) if (contract.domain == domain) return &contract;
    return nullptr;
}

SubspaceParityReport SubspaceAuthoringParitySystem::Validate(const std::vector<SubspaceAuthoringContract>& contracts) {
    SubspaceParityReport report;
    std::unordered_set<std::string> ids;
    std::unordered_set<int> domains;
    for (const auto& contract : contracts) {
        if (contract.contractId.empty()) report.errors.push_back("Authoring contract id is empty");
        if (contract.canonicalType.empty()) report.errors.push_back("Canonical type is empty for " + contract.contractId);
        if (contract.editorWorkspaceId.empty()) report.errors.push_back("Editor workspace is empty for " + contract.contractId);
        if (!contract.editorCanAuthorDefinitions) report.errors.push_back("Editor must be the authoring superset for " + contract.contractId);
        if (!contract.gameClientConsumesCanonicalType) report.errors.push_back("Game client must consume the canonical type for " + contract.contractId);
        if (!ids.insert(contract.contractId).second) report.errors.push_back("Duplicate contract id: " + contract.contractId);
        if (!domains.insert(static_cast<int>(contract.domain)).second) report.errors.push_back("Duplicate authoring domain");
        if (contract.gameClientPlayerEditing && contract.playerFacingSurface.empty())
            report.errors.push_back("Player-editable contract lacks a player-facing surface: " + contract.contractId);
    }
    report.valid = report.errors.empty();
    return report;
}

} // namespace subspace
