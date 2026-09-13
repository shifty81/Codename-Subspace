#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class CharacterCustomizationCategory {
    Identity = 0,
    Body,
    Face,
    Skin,
    Eyes,
    Hair,
    SkinDetail,
    Clothing,
    Accessory,
    Augmentation,
    Portrait,
    Rig,
    Animation,
    SourceAsset
};

enum class CharacterCustomizationMode {
    EditorAuthoring = 0,
    GameCreation,
    GameRecustomization,
    GamePortraitOnly
};

struct CharacterMorphChannel {
    std::string id;
    std::string displayName;
    CharacterCustomizationCategory category = CharacterCustomizationCategory::Face;
    float minimum = -1.0f;
    float maximum = 1.0f;
    float defaultValue = 0.0f;
    float value = 0.0f;
    bool creationEditable = true;
    bool recustomizationEditable = false;
};

struct CharacterSculptBinding {
    std::string zoneId;
    std::string displayName;
    std::string channelId;
    float horizontalWeight = 0.0f;
    float verticalWeight = 0.0f;
    float sensitivity = 1.0f;
};

struct CharacterAppearanceSelection {
    CharacterCustomizationCategory category = CharacterCustomizationCategory::Hair;
    std::string slotId;
    std::string assetId;
    std::string variantId;
    float tintR = 1.0f;
    float tintG = 1.0f;
    float tintB = 1.0f;
};

struct CharacterPortraitSettings {
    std::string poseId = "neutral";
    std::string expressionId = "neutral";
    std::string backgroundId = "default";
    std::string lightRigId = "portrait.default";
    float yawDegrees = 0.0f;
    float pitchDegrees = 0.0f;
    float fieldOfViewDegrees = 35.0f;
};

struct CharacterDefinition {
    std::string schemaId = "subspace.character.v1";
    std::string characterId;
    std::string displayName;
    std::string bodyArchetypeId = "human.standard";
    std::string rigId = "subspace.humanoid.v1";
    std::string bodyMeshId = "character.body.standard";
    std::string headMeshId = "character.head.standard";
    std::string skinMaterialId = "character.skin.default";
    std::vector<CharacterMorphChannel> morphs;
    std::vector<CharacterSculptBinding> sculptBindings;
    std::vector<CharacterAppearanceSelection> appearance;
    CharacterPortraitSettings portrait{};
};

struct CharacterCustomizationPolicy {
    CharacterCustomizationMode mode = CharacterCustomizationMode::EditorAuthoring;
    bool allowStructuralBody = false;
    bool allowStructuralFace = false;
    bool allowRigAuthoring = false;
    bool allowSourceAssetAuthoring = false;
    bool allowAnimationAuthoring = false;
    bool allowPortrait = true;
    std::vector<CharacterCustomizationCategory> editableCategories;
};

struct CharacterValidationReport {
    bool valid = false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

/// Canonical character appearance authority shared by the Shipyard editor and
/// the game client. The editor authors the full definition; player-facing
/// character creation/customization is a policy-constrained view over the same
/// CharacterDefinition rather than a duplicate runtime format.
class CharacterCustomizationSystem {
public:
    static CharacterDefinition CreateDefault(const std::string& characterId = "player.character");

    static CharacterCustomizationPolicy EditorAuthoringPolicy();
    static CharacterCustomizationPolicy GameCreationPolicy();
    static CharacterCustomizationPolicy GameRecustomizationPolicy(bool allowStructuralResculpt = false);
    static CharacterCustomizationPolicy GamePortraitPolicy();

    static bool CanEdit(const CharacterCustomizationPolicy& policy, CharacterCustomizationCategory category);
    static const char* CategoryName(CharacterCustomizationCategory category);

    static CharacterMorphChannel* FindMorph(CharacterDefinition& definition, const std::string& morphId);
    static const CharacterMorphChannel* FindMorph(const CharacterDefinition& definition, const std::string& morphId);

    static bool SetMorph(CharacterDefinition& definition,
                         const CharacterCustomizationPolicy& policy,
                         const std::string& morphId,
                         float value,
                         std::string* reason = nullptr);

    static bool ApplySculptDrag(CharacterDefinition& definition,
                                const CharacterCustomizationPolicy& policy,
                                const std::string& zoneId,
                                float deltaX,
                                float deltaY,
                                std::string* reason = nullptr);

    static CharacterValidationReport Validate(const CharacterDefinition& definition);
};

} // namespace subspace
