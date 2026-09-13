#include "character/CharacterCustomizationSystem.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace subspace {
namespace {

bool Contains(const std::vector<CharacterCustomizationCategory>& values, CharacterCustomizationCategory value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

CharacterMorphChannel Morph(const char* id,
                            const char* name,
                            CharacterCustomizationCategory category,
                            bool creationEditable = true,
                            bool recustomizationEditable = false,
                            float minimum = -1.0f,
                            float maximum = 1.0f) {
    CharacterMorphChannel out;
    out.id = id;
    out.displayName = name;
    out.category = category;
    out.minimum = minimum;
    out.maximum = maximum;
    out.creationEditable = creationEditable;
    out.recustomizationEditable = recustomizationEditable;
    return out;
}

CharacterSculptBinding Sculpt(const char* zone,
                              const char* name,
                              const char* channel,
                              float horizontal,
                              float vertical,
                              float sensitivity = 0.65f) {
    CharacterSculptBinding out;
    out.zoneId = zone;
    out.displayName = name;
    out.channelId = channel;
    out.horizontalWeight = horizontal;
    out.verticalWeight = vertical;
    out.sensitivity = sensitivity;
    return out;
}

} // namespace

CharacterDefinition CharacterCustomizationSystem::CreateDefault(const std::string& characterId) {
    CharacterDefinition d;
    d.characterId = characterId;
    d.displayName = "New Character";

    // The built-in channels are only the starter contract. Production content
    // may register many more channels; the runtime has no fixed morph-count
    // assumption. Direct sculpting is data-driven through sculptBindings.
    d.morphs = {
        Morph("face.width", "Face Width", CharacterCustomizationCategory::Face),
        Morph("face.length", "Face Length", CharacterCustomizationCategory::Face),
        Morph("jaw.width", "Jaw Width", CharacterCustomizationCategory::Face),
        Morph("chin.projection", "Chin Projection", CharacterCustomizationCategory::Face),
        Morph("cheek.volume", "Cheek Volume", CharacterCustomizationCategory::Face),
        Morph("brow.height", "Brow Height", CharacterCustomizationCategory::Face),
        Morph("eyes.spacing", "Eye Spacing", CharacterCustomizationCategory::Face),
        Morph("eyes.size", "Eye Size", CharacterCustomizationCategory::Face),
        Morph("nose.width", "Nose Width", CharacterCustomizationCategory::Face),
        Morph("nose.length", "Nose Length", CharacterCustomizationCategory::Face),
        Morph("nose.bridge", "Nose Bridge", CharacterCustomizationCategory::Face),
        Morph("mouth.width", "Mouth Width", CharacterCustomizationCategory::Face),
        Morph("lips.fullness", "Lip Fullness", CharacterCustomizationCategory::Face),
        Morph("body.height", "Body Height", CharacterCustomizationCategory::Body, true, false, -0.35f, 0.35f),
        Morph("body.shoulders", "Shoulder Width", CharacterCustomizationCategory::Body),
        Morph("body.torso", "Torso Length", CharacterCustomizationCategory::Body),
        Morph("body.chest", "Chest Volume", CharacterCustomizationCategory::Body),
        Morph("body.waist", "Waist", CharacterCustomizationCategory::Body),
        Morph("body.hips", "Hip Width", CharacterCustomizationCategory::Body),
        Morph("body.mass", "Body Mass", CharacterCustomizationCategory::Body)
    };

    d.sculptBindings = {
        Sculpt("face.front", "Face", "face.width", 1.0f, 0.0f),
        Sculpt("face.front", "Face", "face.length", 0.0f, -1.0f),
        Sculpt("jaw.front", "Jaw", "jaw.width", 1.0f, 0.0f),
        Sculpt("chin.profile", "Chin", "chin.projection", 1.0f, 0.0f),
        Sculpt("cheek.front", "Cheek", "cheek.volume", 1.0f, 0.0f),
        Sculpt("brow.front", "Brow", "brow.height", 0.0f, -1.0f),
        Sculpt("eyes.front", "Eyes", "eyes.spacing", 1.0f, 0.0f),
        Sculpt("eyes.front", "Eyes", "eyes.size", 0.0f, -1.0f),
        Sculpt("nose.front", "Nose", "nose.width", 1.0f, 0.0f),
        Sculpt("nose.profile", "Nose", "nose.length", 1.0f, 0.0f),
        Sculpt("nose.profile", "Nose", "nose.bridge", 0.0f, -1.0f),
        Sculpt("mouth.front", "Mouth", "mouth.width", 1.0f, 0.0f),
        Sculpt("mouth.front", "Mouth", "lips.fullness", 0.0f, -1.0f),
        Sculpt("body.shoulders", "Shoulders", "body.shoulders", 1.0f, 0.0f),
        Sculpt("body.torso", "Torso", "body.torso", 0.0f, -1.0f),
        Sculpt("body.waist", "Waist", "body.waist", 1.0f, 0.0f),
        Sculpt("body.hips", "Hips", "body.hips", 1.0f, 0.0f)
    };

    d.appearance = {
        {CharacterCustomizationCategory::Eyes, "eyes", "character.eyes.default", "brown"},
        {CharacterCustomizationCategory::Hair, "hair", "character.hair.short_01", "dark"},
        {CharacterCustomizationCategory::SkinDetail, "complexion", "character.skin_detail.clear", "default"},
        {CharacterCustomizationCategory::Clothing, "inner", "character.clothing.inner.default", "default"},
        {CharacterCustomizationCategory::Clothing, "outer", "character.clothing.outer.none", "default"}
    };
    return d;
}

CharacterCustomizationPolicy CharacterCustomizationSystem::EditorAuthoringPolicy() {
    CharacterCustomizationPolicy p;
    p.mode = CharacterCustomizationMode::EditorAuthoring;
    p.allowStructuralBody = true;
    p.allowStructuralFace = true;
    p.allowRigAuthoring = true;
    p.allowSourceAssetAuthoring = true;
    p.allowAnimationAuthoring = true;
    p.editableCategories = {
        CharacterCustomizationCategory::Identity,
        CharacterCustomizationCategory::Body,
        CharacterCustomizationCategory::Face,
        CharacterCustomizationCategory::Skin,
        CharacterCustomizationCategory::Eyes,
        CharacterCustomizationCategory::Hair,
        CharacterCustomizationCategory::SkinDetail,
        CharacterCustomizationCategory::Clothing,
        CharacterCustomizationCategory::Accessory,
        CharacterCustomizationCategory::Augmentation,
        CharacterCustomizationCategory::Portrait,
        CharacterCustomizationCategory::Rig,
        CharacterCustomizationCategory::Animation,
        CharacterCustomizationCategory::SourceAsset
    };
    return p;
}

CharacterCustomizationPolicy CharacterCustomizationSystem::GameCreationPolicy() {
    CharacterCustomizationPolicy p;
    p.mode = CharacterCustomizationMode::GameCreation;
    p.allowStructuralBody = true;
    p.allowStructuralFace = true;
    p.editableCategories = {
        CharacterCustomizationCategory::Identity,
        CharacterCustomizationCategory::Body,
        CharacterCustomizationCategory::Face,
        CharacterCustomizationCategory::Skin,
        CharacterCustomizationCategory::Eyes,
        CharacterCustomizationCategory::Hair,
        CharacterCustomizationCategory::SkinDetail,
        CharacterCustomizationCategory::Clothing,
        CharacterCustomizationCategory::Accessory,
        CharacterCustomizationCategory::Augmentation,
        CharacterCustomizationCategory::Portrait
    };
    return p;
}

CharacterCustomizationPolicy CharacterCustomizationSystem::GameRecustomizationPolicy(bool allowStructuralResculpt) {
    CharacterCustomizationPolicy p;
    p.mode = CharacterCustomizationMode::GameRecustomization;
    p.allowStructuralBody = allowStructuralResculpt;
    p.allowStructuralFace = allowStructuralResculpt;
    p.editableCategories = {
        CharacterCustomizationCategory::Skin,
        CharacterCustomizationCategory::Eyes,
        CharacterCustomizationCategory::Hair,
        CharacterCustomizationCategory::SkinDetail,
        CharacterCustomizationCategory::Clothing,
        CharacterCustomizationCategory::Accessory,
        CharacterCustomizationCategory::Augmentation,
        CharacterCustomizationCategory::Portrait
    };
    if (allowStructuralResculpt) {
        p.editableCategories.push_back(CharacterCustomizationCategory::Body);
        p.editableCategories.push_back(CharacterCustomizationCategory::Face);
    }
    return p;
}

CharacterCustomizationPolicy CharacterCustomizationSystem::GamePortraitPolicy() {
    CharacterCustomizationPolicy p;
    p.mode = CharacterCustomizationMode::GamePortraitOnly;
    p.editableCategories = {CharacterCustomizationCategory::Portrait};
    return p;
}

bool CharacterCustomizationSystem::CanEdit(const CharacterCustomizationPolicy& policy, CharacterCustomizationCategory category) {
    if (!Contains(policy.editableCategories, category)) return false;
    if (category == CharacterCustomizationCategory::Rig) return policy.allowRigAuthoring;
    if (category == CharacterCustomizationCategory::Animation) return policy.allowAnimationAuthoring;
    if (category == CharacterCustomizationCategory::SourceAsset) return policy.allowSourceAssetAuthoring;
    if (category == CharacterCustomizationCategory::Face && !policy.allowStructuralFace) return false;
    if (category == CharacterCustomizationCategory::Body && !policy.allowStructuralBody) return false;
    if (category == CharacterCustomizationCategory::Portrait) return policy.allowPortrait;
    return true;
}

const char* CharacterCustomizationSystem::CategoryName(CharacterCustomizationCategory category) {
    switch (category) {
        case CharacterCustomizationCategory::Identity: return "IDENTITY";
        case CharacterCustomizationCategory::Body: return "BODY";
        case CharacterCustomizationCategory::Face: return "FACE";
        case CharacterCustomizationCategory::Skin: return "SKIN";
        case CharacterCustomizationCategory::Eyes: return "EYES";
        case CharacterCustomizationCategory::Hair: return "HAIR";
        case CharacterCustomizationCategory::SkinDetail: return "SKIN_DETAIL";
        case CharacterCustomizationCategory::Clothing: return "CLOTHING";
        case CharacterCustomizationCategory::Accessory: return "ACCESSORY";
        case CharacterCustomizationCategory::Augmentation: return "AUGMENTATION";
        case CharacterCustomizationCategory::Portrait: return "PORTRAIT";
        case CharacterCustomizationCategory::Rig: return "RIG";
        case CharacterCustomizationCategory::Animation: return "ANIMATION";
        case CharacterCustomizationCategory::SourceAsset: return "SOURCE_ASSET";
    }
    return "UNKNOWN";
}

CharacterMorphChannel* CharacterCustomizationSystem::FindMorph(CharacterDefinition& definition, const std::string& morphId) {
    for (auto& morph : definition.morphs) if (morph.id == morphId) return &morph;
    return nullptr;
}

const CharacterMorphChannel* CharacterCustomizationSystem::FindMorph(const CharacterDefinition& definition, const std::string& morphId) {
    for (const auto& morph : definition.morphs) if (morph.id == morphId) return &morph;
    return nullptr;
}

bool CharacterCustomizationSystem::SetMorph(CharacterDefinition& definition,
                                             const CharacterCustomizationPolicy& policy,
                                             const std::string& morphId,
                                             float value,
                                             std::string* reason) {
    auto* morph = FindMorph(definition, morphId);
    if (!morph) {
        if (reason) *reason = "Unknown morph channel: " + morphId;
        return false;
    }
    if (!CanEdit(policy, morph->category)) {
        if (reason) *reason = "Customization policy does not permit " + std::string(CategoryName(morph->category));
        return false;
    }
    if (policy.mode == CharacterCustomizationMode::GameCreation && !morph->creationEditable) {
        if (reason) *reason = "Morph is not editable during game character creation";
        return false;
    }
    if (policy.mode == CharacterCustomizationMode::GameRecustomization && !morph->recustomizationEditable &&
        !(policy.allowStructuralBody || policy.allowStructuralFace)) {
        if (reason) *reason = "Morph requires structural resculpt permission";
        return false;
    }
    morph->value = std::clamp(value, morph->minimum, morph->maximum);
    return true;
}

bool CharacterCustomizationSystem::ApplySculptDrag(CharacterDefinition& definition,
                                                    const CharacterCustomizationPolicy& policy,
                                                    const std::string& zoneId,
                                                    float deltaX,
                                                    float deltaY,
                                                    std::string* reason) {
    bool foundZone = false;
    bool changed = false;
    for (const auto& binding : definition.sculptBindings) {
        if (binding.zoneId != zoneId) continue;
        foundZone = true;
        auto* morph = FindMorph(definition, binding.channelId);
        if (!morph || !CanEdit(policy, morph->category)) continue;
        const float delta = (deltaX * binding.horizontalWeight + deltaY * binding.verticalWeight) * binding.sensitivity;
        if (std::fabs(delta) < 0.000001f) continue;
        std::string ignored;
        changed = SetMorph(definition, policy, binding.channelId, morph->value + delta, &ignored) || changed;
    }
    if (!foundZone) {
        if (reason) *reason = "Unknown sculpt zone: " + zoneId;
        return false;
    }
    if (!changed && reason) *reason = "Sculpt zone has no editable channels under the active policy";
    return changed;
}

CharacterValidationReport CharacterCustomizationSystem::Validate(const CharacterDefinition& definition) {
    CharacterValidationReport report;
    if (definition.schemaId != "subspace.character.v1") report.errors.push_back("Unsupported character schema: " + definition.schemaId);
    if (definition.characterId.empty()) report.errors.push_back("CharacterId is required");
    if (definition.rigId.empty()) report.errors.push_back("RigId is required");
    if (definition.bodyMeshId.empty()) report.errors.push_back("Body mesh is required");
    if (definition.headMeshId.empty()) report.errors.push_back("Head mesh is required");
    if (definition.skinMaterialId.empty()) report.errors.push_back("Skin material is required");

    std::unordered_set<std::string> morphIds;
    for (const auto& morph : definition.morphs) {
        if (morph.id.empty()) report.errors.push_back("Morph channel has an empty id");
        else if (!morphIds.insert(morph.id).second) report.errors.push_back("Duplicate morph channel: " + morph.id);
        if (!(morph.minimum <= morph.defaultValue && morph.defaultValue <= morph.maximum))
            report.errors.push_back("Morph default is outside range: " + morph.id);
        if (!(morph.minimum <= morph.value && morph.value <= morph.maximum))
            report.errors.push_back("Morph value is outside range: " + morph.id);
    }
    for (const auto& binding : definition.sculptBindings) {
        if (binding.zoneId.empty()) report.errors.push_back("Sculpt binding has an empty zone id");
        if (!FindMorph(definition, binding.channelId)) report.errors.push_back("Sculpt binding references missing morph: " + binding.channelId);
    }
    if (definition.morphs.size() < 16) report.warnings.push_back("Character definition has a minimal morph library; production characters should expose a richer data-driven sculpt set");
    report.valid = report.errors.empty();
    return report;
}

} // namespace subspace
