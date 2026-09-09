#include "character/CharacterAnimationLibrarySystem.h"

#include <algorithm>
#include <cctype>

namespace subspace {
namespace {
std::string Lower(std::string s){std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});return s;}
bool Has(const std::string& s,const char* token){return s.find(token)!=std::string::npos;}
}

CharacterAnimationLibraryProfile CharacterAnimationLibrarySystem::QuaterniusUniversalAnimationLibrary2() {
    CharacterAnimationLibraryProfile p;
    p.libraryId = "quaternius.universal_animation_library_2";
    p.displayName = "Quaternius Universal Animation Library 2";
    p.sourceId = "quaternius_universal_animation_library_2_standard";
    p.revision = "v2.1-2026-07-05";
    p.license = "CC0-1.0";
    p.expectedMinimumClipCount = 130;
    // Subspace is server-authoritative. Locomotion therefore defaults to
    // non-root-motion while authored actions may opt into root motion when the
    // gameplay authority explicitly accepts the displacement.
    p.locomotionPolicy = RootMotionPolicy::Disabled;
    p.actionPolicy = RootMotionPolicy::ActionOnly;
    p.rig.rigId = "quaternius.universal_humanoid.v2";
    p.rig.displayName = "Universal Humanoid Rig v2";
    p.rig.sourceId = p.sourceId;
    p.rig.expectedHeightMeters = 1.80f;
    p.rig.requiredBones = {"root","hips","spine","head","upper_arm_l","upper_arm_r","upper_leg_l","upper_leg_r"};
    return p;
}

const char* CharacterAnimationLibrarySystem::CategoryName(CharacterAnimationCategory c){
    switch(c){
        case CharacterAnimationCategory::Idle:return "IDLE";
        case CharacterAnimationCategory::Locomotion:return "LOCOMOTION";
        case CharacterAnimationCategory::Combat:return "COMBAT";
        case CharacterAnimationCategory::Interaction:return "INTERACTION";
        case CharacterAnimationCategory::Parkour:return "PARKOUR";
        case CharacterAnimationCategory::Work:return "WORK";
        case CharacterAnimationCategory::Farming:return "FARMING";
        case CharacterAnimationCategory::Fishing:return "FISHING";
        case CharacterAnimationCategory::Injury:return "INJURY";
        case CharacterAnimationCategory::CreatureLike:return "CREATURE-LIKE";
        case CharacterAnimationCategory::Emote:return "EMOTE";
        case CharacterAnimationCategory::Other:return "OTHER";
    }
    return "OTHER";
}

bool CharacterAnimationLibrarySystem::IsNetworkSafeLocomotion(const CharacterAnimationLibraryProfile& p){
    return p.locomotionPolicy==RootMotionPolicy::Disabled;
}

CharacterRetargetPlan CharacterAnimationLibrarySystem::BuildRetargetPlan(const CharacterAnimationLibraryProfile& p,
                                                                          float importedRigHeightUnits,
                                                                          float sourceMetersPerUnit,
                                                                          const WorldScaleProfile& scaleProfile){
    CharacterRetargetPlan out;
    const auto calibration=WorldScaleAuthoritySystem::CalibrateHumanoid(importedRigHeightUnits,sourceMetersPerUnit,scaleProfile);
    out.valid=calibration.valid&&p.rig.humanoid&&p.rig.retargetable;
    out.uniformScale=calibration.uniformScale;
    out.locomotionPolicy=p.locomotionPolicy;
    out.actionPolicy=p.actionPolicy;
    out.warnings=calibration.warnings;
    if(!p.rig.humanoid)out.warnings.push_back("Animation library rig is not marked humanoid");
    if(!p.rig.retargetable)out.warnings.push_back("Animation library rig is not marked retargetable");
    if(p.expectedMinimumClipCount<=0)out.warnings.push_back("Animation library does not declare an expected clip-count floor");
    return out;
}

CharacterAnimationCategory CharacterAnimationLibrarySystem::ClassifyClipName(const std::string& raw){
    const auto s=Lower(raw);
    if(Has(s,"fish"))return CharacterAnimationCategory::Fishing;
    if(Has(s,"farm")||Has(s,"hoe")||Has(s,"harvest"))return CharacterAnimationCategory::Farming;
    if(Has(s,"parkour")||Has(s,"vault")||Has(s,"climb")||Has(s,"jump")||Has(s,"ledge"))return CharacterAnimationCategory::Parkour;
    if(Has(s,"attack")||Has(s,"sword")||Has(s,"melee")||Has(s,"shoot")||Has(s,"rifle")||Has(s,"pistol")||Has(s,"combo"))return CharacterAnimationCategory::Combat;
    if(Has(s,"walk")||Has(s,"run")||Has(s,"jog")||Has(s,"sprint")||Has(s,"strafe")||Has(s,"crawl")||Has(s,"swim"))return CharacterAnimationCategory::Locomotion;
    if(Has(s,"hurt")||Has(s,"injur")||Has(s,"death")||Has(s,"fall"))return CharacterAnimationCategory::Injury;
    if(Has(s,"zombie"))return CharacterAnimationCategory::CreatureLike;
    if(Has(s,"wave")||Has(s,"cheer")||Has(s,"emote"))return CharacterAnimationCategory::Emote;
    if(Has(s,"idle"))return CharacterAnimationCategory::Idle;
    if(Has(s,"push")||Has(s,"repair")||Has(s,"work")||Has(s,"carry"))return CharacterAnimationCategory::Work;
    if(Has(s,"use")||Has(s,"interact")||Has(s,"sit")||Has(s,"open"))return CharacterAnimationCategory::Interaction;
    return CharacterAnimationCategory::Other;
}

bool CharacterAnimationLibrarySystem::ShouldUseRootMotion(CharacterAnimationCategory c,
                                                           const CharacterAnimationLibraryProfile& p){
    if(c==CharacterAnimationCategory::Locomotion)return p.locomotionPolicy==RootMotionPolicy::Enabled||p.locomotionPolicy==RootMotionPolicy::SourceDefault;
    const bool action=c==CharacterAnimationCategory::Combat||c==CharacterAnimationCategory::Parkour||c==CharacterAnimationCategory::Interaction||c==CharacterAnimationCategory::Work;
    if(!action)return false;
    return p.actionPolicy==RootMotionPolicy::Enabled||p.actionPolicy==RootMotionPolicy::ActionOnly||p.actionPolicy==RootMotionPolicy::SourceDefault;
}

} // namespace subspace
