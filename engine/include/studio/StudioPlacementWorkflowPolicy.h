#pragma once
#include "rendering/ProceduralVisualVariantSystem.h"
#include <cmath>
#include <cstddef>

namespace subspace {
struct StudioPlacementWorkflowPolicy {
    static constexpr float kDuplicateSnapOffset = 0.25f;
    static constexpr float kDuplicateFreeOffset = 0.10f;
    static constexpr float DuplicateOffset(bool snapEnabled) noexcept { return snapEnabled ? kDuplicateSnapOffset : kDuplicateFreeOffset; }
    static constexpr std::size_t SelectionAfterRemoval(std::size_t removed,std::size_t remaining) noexcept {
        return remaining == 0 ? 0 : (removed < remaining ? removed : remaining - 1);
    }
    static void PreserveDuplicateAuthoredTraits(const VisualModulePlacement& authored,VisualModulePlacement& snapped) noexcept {
        // The snap candidate owns geometry needed for socket coincidence.
        // Material state is independent of that fit and survives duplication.
        snapped.material=authored.material;
        snapped.sourceMaterialsEnabled=authored.sourceMaterialsEnabled;
    }
    static bool SnapNormalizesGeometry(const VisualModulePlacement& authored,const VisualModulePlacement& candidate,float epsilon=0.0001f) noexcept {
        return std::fabs(authored.scaleX-candidate.scaleX)>epsilon||std::fabs(authored.scaleY-candidate.scaleY)>epsilon||
               std::fabs(authored.scaleZ-candidate.scaleZ)>epsilon||authored.mirrorX!=candidate.mirrorX||
               authored.mirrorY!=candidate.mirrorY||authored.mirrorZ!=candidate.mirrorZ;
    }
    static constexpr bool CommitIsSingleUndoUnit() noexcept { return true; }
    static constexpr bool NormalDeletePreservesDescendants() noexcept { return true; }
};
} // namespace subspace
