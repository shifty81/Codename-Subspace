#pragma once
#include <string>
#include <vector>

namespace subspace {

enum class CompletionTruthState {
    CertifiedRuntime,
    ImplementedUncertified,
    Partial,
    Scaffold,
    Placeholder,
    Legacy,
    Superseded,
    Planned
};

struct CompletionTruthEvidence {
    bool sourceExists = false;
    bool compiles = false;
    bool unitBehavior = false;
    bool runtimeWired = false;
    bool runtimeSmoke = false;
    bool persistenceRoundTrip = false;
    bool containsPlaceholder = false;
    bool legacy = false;
    bool superseded = false;
    bool plannedOnly = false;
    std::vector<std::string> notes;
};

struct CompletionTruthRecord {
    std::string systemId;
    CompletionTruthState state = CompletionTruthState::Planned;
    CompletionTruthEvidence evidence{};
    std::string reason;
};

class ProjectCompletionTruthSystem {
public:
    static CompletionTruthState Evaluate(const CompletionTruthEvidence& evidence);
    static CompletionTruthRecord Build(std::string systemId, CompletionTruthEvidence evidence);
    static const char* Name(CompletionTruthState state);
    static bool MayClaimDone(CompletionTruthState state);
};

} // namespace subspace
