#pragma once

// Studio-owned interaction rules. No game frontend or Engine pause state.
namespace subspace {
enum class StudioEscapeAction { CancelPlacement, CancelTransform, CloseCommandPalette, CloseMenu, ClearConstraint, NoOp };
struct StudioEscapeState {
    bool placement = false;
    bool transform = false;
    bool commandPalette = false;
    bool menu = false;
    bool constrained = false;
};
struct StudioSessionPolicy {
    static constexpr StudioEscapeAction Escape(const StudioEscapeState& state) noexcept {
        return state.placement ? StudioEscapeAction::CancelPlacement :
               state.transform ? StudioEscapeAction::CancelTransform :
               state.commandPalette ? StudioEscapeAction::CloseCommandPalette :
               state.menu ? StudioEscapeAction::CloseMenu :
               state.constrained ? StudioEscapeAction::ClearConstraint : StudioEscapeAction::NoOp;
    }
    static constexpr bool MayDiscard(bool dirty, bool explicitConfirmation) noexcept {
        return !dirty || explicitConfirmation;
    }
    static constexpr bool MayPublish(bool geometryValid, bool collisionClean, bool socketsClean,
                                     bool surfacesClean, bool provenanceReviewed) noexcept {
        return geometryValid && collisionClean && socketsClean && surfacesClean && provenanceReviewed;
    }
};
} // namespace subspace
