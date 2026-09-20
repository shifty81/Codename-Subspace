#pragma once

namespace subspace {
struct StudioUnsavedWorkState {
    bool blueprint=false;
    bool socketOverrides=false;
    bool definitionOverrides=false;
    bool modelDraft=false;
    bool interiorDraft=false;
};
struct StudioUnsavedWorkPolicy {
    static constexpr bool HasUnsaved(const StudioUnsavedWorkState& s) noexcept {
        return s.blueprint||s.socketOverrides||s.definitionOverrides||s.modelDraft||s.interiorDraft;
    }
    // A blueprint recovery is necessarily partial when any independent
    // authoring lane is dirty; do not certify it as a complete session save.
    static constexpr bool HasUnsupportedRecovery(const StudioUnsavedWorkState& s,
                                                  bool modelRecoveryVerified=false) noexcept {
        return s.socketOverrides||s.definitionOverrides||
               (s.modelDraft&&!modelRecoveryVerified)||s.interiorDraft;
    }
};
}
