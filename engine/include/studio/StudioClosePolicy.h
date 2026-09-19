#pragma once

namespace subspace {
// Window close behavior is Studio-owned. The game does not register a guard.
enum class StudioCloseChoice { SaveAndClose, CloseWithRecovery, Cancel };
struct StudioCloseState {
    bool blueprintDirty=false;
    bool socketDirty=false;
    bool definitionDirty=false;
    bool modelDraft=false;
    bool interiorDraft=false;
};
struct StudioClosePolicy {
    static constexpr bool NeedsPrompt(StudioCloseState state) noexcept {
        return state.blueprintDirty||state.socketDirty||state.definitionDirty||
               state.modelDraft||state.interiorDraft;
    }
    static constexpr bool NeedsExplicitDataLossWarning(StudioCloseState state) noexcept {
        return state.socketDirty||state.definitionDirty||state.modelDraft||state.interiorDraft;
    }
    static constexpr bool MayCloseAfterSave(StudioCloseState state) noexcept {
        return !NeedsPrompt(state);
    }
    static constexpr bool MayCloseWithRecovery(StudioCloseState state,
                                               bool blueprintRecovered,
                                               bool acknowledgedUnsupported) noexcept {
        if(state.blueprintDirty && !blueprintRecovered)return false;
        if(NeedsExplicitDataLossWarning(state)&&!acknowledgedUnsupported)return false;
        return true;
    }
};
}
