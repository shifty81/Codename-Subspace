#include "migration/CppNormalizationRoadmap.h"

#include <sstream>

namespace subspace {

CppNormalizationRoadmap CreatePostPass114CppNormalizationRoadmap() {
    CppNormalizationRoadmap roadmap;
    roadmap.gates = {
        {"client-split", "Split Win32PlayableClient into client app/input/render/view units", "client", NormalizationGateStatus::InProgress, {"build passes", "client launches", "no lost controls"}},
        {"home-surface", "Make H primary home-world surface builder", "home", NormalizationGateStatus::Ready, {"large build grid", "structure placement", "production overlay"}},
        {"csharp-ledger", "Every C# source file has a migration status", "migration", NormalizationGateStatus::InProgress, {"ledger csv", "validator", "zero unknown files"}},
        {"content-layout", "Normalize Assets/assets/GameData into content authority", "content", NormalizationGateStatus::NotStarted, {"dry run", "path audit", "manifest"}},
        {"builder-home-only", "Ship part hot-swap is home-only before adventure launch", "shipyard", NormalizationGateStatus::Ready, {"install validation", "launch lockout", "ship stats update"}},
        {"travel-rail", "Interstellar travel uses on-rails route model", "travel", NormalizationGateStatus::InProgress, {"route fit", "route events", "arrival state"}}
    };
    return roadmap;
}

std::string NormalizationGateStatusName(NormalizationGateStatus status) {
    switch (status) {
        case NormalizationGateStatus::NotStarted: return "NotStarted";
        case NormalizationGateStatus::InProgress: return "InProgress";
        case NormalizationGateStatus::Ready: return "Ready";
        case NormalizationGateStatus::Complete: return "Complete";
    }
    return "Unknown";
}

std::string CppNormalizationRoadmapSummary(const CppNormalizationRoadmap& roadmap) {
    int ready = 0;
    int complete = 0;
    for (const auto& gate : roadmap.gates) {
        if (gate.status == NormalizationGateStatus::Ready) ++ready;
        if (gate.status == NormalizationGateStatus::Complete) ++complete;
    }
    std::ostringstream out;
    out << "cppNormalization gates=" << roadmap.gates.size()
        << " ready=" << ready
        << " complete=" << complete;
    return out.str();
}

} // namespace subspace
