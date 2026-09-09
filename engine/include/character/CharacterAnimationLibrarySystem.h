#pragma once

#include "world/WorldScaleAuthoritySystem.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class CharacterAnimationCategory {
    Idle,
    Locomotion,
    Combat,
    Interaction,
    Parkour,
    Work,
    Farming,
    Fishing,
    Injury,
    CreatureLike,
    Emote,
    Other
};

enum class RootMotionPolicy {
    Disabled,
    Enabled,
    ActionOnly,
    SourceDefault
};

struct CharacterRigProfile {
    std::string rigId;
    std::string displayName;
    std::string sourceId;
    float expectedHeightMeters = 1.80f;
    bool humanoid = true;
    bool retargetable = true;
    std::vector<std::string> requiredBones;
};

struct CharacterAnimationClipDef {
    std::string clipId;
    std::string sourceClipName;
    CharacterAnimationCategory category = CharacterAnimationCategory::Other;
    bool looping = false;
    bool sourceHasRootMotion = false;
    bool devPreview = true;
    std::vector<std::string> tags;
};

struct CharacterAnimationLibraryProfile {
    std::string libraryId;
    std::string displayName;
    std::string sourceId;
    std::string revision;
    std::string license;
    int expectedMinimumClipCount = 0;
    RootMotionPolicy locomotionPolicy = RootMotionPolicy::Disabled;
    RootMotionPolicy actionPolicy = RootMotionPolicy::ActionOnly;
    CharacterRigProfile rig{};
};

struct CharacterRetargetPlan {
    bool valid = false;
    float uniformScale = 1.0f;
    RootMotionPolicy locomotionPolicy = RootMotionPolicy::Disabled;
    RootMotionPolicy actionPolicy = RootMotionPolicy::ActionOnly;
    std::vector<std::string> warnings;
};

struct DevAnimationPreviewState {
    std::string libraryId;
    std::string selectedClipId;
    float playbackRate = 1.0f;
    bool playing = true;
    bool rootMotionVisualization = false;
    bool showSkeleton = false;
    bool showPlayerScaleGuide = true;
};

class CharacterAnimationLibrarySystem {
public:
    static CharacterAnimationLibraryProfile QuaterniusUniversalAnimationLibrary2();
    static const char* CategoryName(CharacterAnimationCategory category);
    static bool IsNetworkSafeLocomotion(const CharacterAnimationLibraryProfile& profile);
    static CharacterRetargetPlan BuildRetargetPlan(const CharacterAnimationLibraryProfile& profile,
                                                   float importedRigHeightUnits,
                                                   float sourceMetersPerUnit,
                                                   const WorldScaleProfile& scaleProfile);
    static CharacterAnimationCategory ClassifyClipName(const std::string& clipName);
    static bool ShouldUseRootMotion(CharacterAnimationCategory category,
                                    const CharacterAnimationLibraryProfile& profile);
};

} // namespace subspace
