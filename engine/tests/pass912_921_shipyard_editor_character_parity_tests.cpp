#include "character/CharacterCustomizationSystem.h"
#include "editor/EditorDockSystem.h"
#include "editor/ProjectWideEditorNormalizationSystem.h"
#include "editor/ShipyardEditorShellSystem.h"
#include "editor/SubspaceAuthoringParitySystem.h"
#include "integration/ExternalTechnologyAdoptionSystem.h"

#include <algorithm>
#include <iostream>

using namespace subspace;

namespace {
int failures = 0;
int assertions = 0;
void Check(bool ok, const char* name) {
    ++assertions;
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << name << "\n";
    if (!ok) ++failures;
}
}

int main() {
    std::cout << "[Pass912-921 Shipyard Editor / Character / Client Parity]\n";

    const auto workspaces = ShipyardEditorShellSystem::BuildWorkspaceRegistry();
    const auto primary = ShipyardEditorShellSystem::PrimaryWorkspaceOrder();
    Check(primary.size() >= 13 && workspaces.Find(EditorWorkspaceKind::Character) &&
              workspaces.Find(EditorWorkspaceKind::World) && workspaces.Find(EditorWorkspaceKind::Animation),
          "Pass912 Shipyard is one editor shell with Blender-like task workspaces rather than separate editor clients");

    auto characterDock = ShipyardEditorShellSystem::CreateWorkspaceDock(EditorWorkspaceKind::Character);
    std::string dockError;
    Check(EditorDockSystem::Validate(characterDock, &dockError) &&
              EditorDockSystem::FindPanel(characterDock, "character_sculpt") &&
              EditorDockSystem::FindPanel(characterDock, "character_apparel") &&
              EditorDockSystem::FindPanel(characterDock, "character_portrait"),
          "Pass913 Character authoring is a dock workspace inside the canonical Shipyard shell");

    auto character = CharacterCustomizationSystem::CreateDefault("test.pilot");
    const auto validation = CharacterCustomizationSystem::Validate(character);
    Check(validation.valid && character.schemaId == "subspace.character.v1" && character.morphs.size() >= 16 &&
              !character.sculptBindings.empty(),
          "Pass914 one canonical CharacterDefinition owns data-driven morphs and direct sculpt zones");

    auto creation = CharacterCustomizationSystem::GameCreationPolicy();
    std::string reason;
    const auto* noseBefore = CharacterCustomizationSystem::FindMorph(character, "nose.width");
    const float oldNose = noseBefore ? noseBefore->value : 0.0f;
    const bool sculpted = CharacterCustomizationSystem::ApplySculptDrag(character, creation, "nose.front", 0.5f, 0.0f, &reason);
    const auto* noseAfter = CharacterCustomizationSystem::FindMorph(character, "nose.width");
    Check(sculpted && noseAfter && noseAfter->value > oldNose &&
              CharacterCustomizationSystem::CanEdit(creation, CharacterCustomizationCategory::Clothing) &&
              !CharacterCustomizationSystem::CanEdit(creation, CharacterCustomizationCategory::Rig),
          "Pass915 game character creation uses the same definition while hiding rig/source authoring");

    auto recustomize = CharacterCustomizationSystem::GameRecustomizationPolicy(false);
    reason.clear();
    Check(!CharacterCustomizationSystem::SetMorph(character, recustomize, "jaw.width", 0.4f, &reason) &&
              CharacterCustomizationSystem::CanEdit(recustomize, CharacterCustomizationCategory::Hair) &&
              CharacterCustomizationSystem::CanEdit(recustomize, CharacterCustomizationCategory::Augmentation),
          "Pass916 normal recustomization keeps cosmetics/apparel/body-modification access separate from structural resculpting");

    auto resculpt = CharacterCustomizationSystem::GameRecustomizationPolicy(true);
    Check(CharacterCustomizationSystem::SetMorph(character, resculpt, "jaw.width", 0.4f) &&
              CharacterCustomizationSystem::CanEdit(resculpt, CharacterCustomizationCategory::Face) &&
              CharacterCustomizationSystem::CanEdit(resculpt, CharacterCustomizationCategory::Body),
          "Pass917 explicit structural-resculpt policy can unlock body/face changes without changing the canonical character type");

    auto portrait = CharacterCustomizationSystem::GamePortraitPolicy();
    const auto editorPolicy = CharacterCustomizationSystem::EditorAuthoringPolicy();
    Check(CharacterCustomizationSystem::CanEdit(portrait, CharacterCustomizationCategory::Portrait) &&
              !CharacterCustomizationSystem::CanEdit(portrait, CharacterCustomizationCategory::Face) &&
              CharacterCustomizationSystem::CanEdit(editorPolicy, CharacterCustomizationCategory::Rig) &&
              CharacterCustomizationSystem::CanEdit(editorPolicy, CharacterCustomizationCategory::SourceAsset),
          "Pass918 portrait-only and full-editor policies are permissions over one shared CharacterDefinition");

    const auto contracts = SubspaceAuthoringParitySystem::BuildDefaultContracts();
    const auto parity = SubspaceAuthoringParitySystem::Validate(contracts);
    const auto* characterContract = SubspaceAuthoringParitySystem::Find(contracts, SubspaceAuthoringDomain::Character);
    const auto* shipContract = SubspaceAuthoringParitySystem::Find(contracts, SubspaceAuthoringDomain::Ship);
    Check(parity.valid && characterContract && characterContract->canonicalType == "CharacterDefinition" &&
              characterContract->gameClientPlayerEditing && shipContract && shipContract->gameClientPlayerEditing,
          "Pass919 editor/game-client parity consumes the exact same ship and character canonical contracts");

    const auto carbon = ExternalTechnologyAdoptionSystem::CarbonCandidates();
    const auto findCarbon = [&](const char* id) -> const ExternalTechnologyCandidate* {
        const auto it = std::find_if(carbon.begin(), carbon.end(), [&](const auto& candidate) { return candidate.id == id; });
        return it == carbon.end() ? nullptr : &*it;
    };
    const auto* mesh = findCarbon("carbon.mesh");
    const auto* destiny = findCarbon("carbon.destiny");
    const auto* trinity = findCarbon("carbon.trinity");
    Check(mesh && mesh->disposition == ExternalTechnologyDisposition::EvaluateForIntegration &&
              destiny && destiny->disposition == ExternalTechnologyDisposition::ArchitectureReference &&
              trinity && trinity->disposition == ExternalTechnologyDisposition::ArchitectureReference,
          "Pass920 Carbon adoption is explicit and bounded rather than a second engine authority");

    const auto normalization = ProjectWideEditorNormalizationSystem::Audit();
    const bool characterNormalized = std::any_of(normalization.entries.begin(), normalization.entries.end(), [](const auto& e) {
        return e.domain == EditorNormalizationDomain::CharacterDefinition && e.normalized;
    });
    const bool parityNormalized = std::any_of(normalization.entries.begin(), normalization.entries.end(), [](const auto& e) {
        return e.domain == EditorNormalizationDomain::AuthoringParity && e.normalized;
    });
    Check(characterNormalized && parityNormalized,
          "Pass921 normalization audit records canonical character authority and one-editor/one-client parity");

    std::cout << "Pass912-921 assertions: " << (assertions - failures) << " / " << assertions << " passed\n";
    return failures ? 1 : 0;
}
