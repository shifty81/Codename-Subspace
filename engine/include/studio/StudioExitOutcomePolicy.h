#pragma once

namespace subspace {
// Studio-owned lifecycle result. Unconfirmed covers unexpected loop termination
// (including an automated frame limit); it is never equivalent to user consent.
enum class StudioExitOutcome {
    Unconfirmed,
    Clean,
    Saved,
    UserConfirmedPartialRecovery
};

struct StudioExitOutcomePolicy {
    static constexpr bool IsUserConfirmed(StudioExitOutcome outcome) noexcept {
        return outcome != StudioExitOutcome::Unconfirmed;
    }
    static constexpr int ExitCode(StudioExitOutcome outcome,
                                  bool recoveryFailed,
                                  bool unsupportedUnsavedDrafts) noexcept {
        // A recovery failure is an error even if an outcome was recorded.
        if (recoveryFailed) return 6;
        // An unconfirmed termination must not silently discard unsupported work.
        if (unsupportedUnsavedDrafts && !IsUserConfirmed(outcome)) return 7;
        return 0;
    }
    static constexpr const char* Name(StudioExitOutcome outcome) noexcept {
        switch (outcome) {
        case StudioExitOutcome::Clean: return "CLEAN";
        case StudioExitOutcome::Saved: return "SAVED";
        case StudioExitOutcome::UserConfirmedPartialRecovery:
            return "USER_CONFIRMED_PARTIAL_RECOVERY";
        default: return "UNCONFIRMED";
        }
    }
};
} // namespace subspace
