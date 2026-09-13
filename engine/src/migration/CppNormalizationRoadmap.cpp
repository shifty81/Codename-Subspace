#include "migration/CppNormalizationRoadmap.h"

#include <sstream>

namespace subspace {

CppNormalizationRoadmap CreatePostPass114CppNormalizationRoadmap() {
    CppNormalizationRoadmap roadmap;
    roadmap.gates = {
        {"runtime-composition", "Converge Engine services and NativeGameApplication state behind one runtime composition authority", "runtime", NormalizationGateStatus::InProgress, {"one state owner", "shell adapters", "no duplicate simulation authority"}},
        {"spatial-persistence", "Finish rotation-aware hierarchical frame reparenting, stable IDs, migrations and representation handoffs", "world", NormalizationGateStatus::InProgress, {"nested frame tests", "save round trip", "ship/cell identity"}},
        {"content-layout", "Keep GameData as authored runtime data and content as governed metadata/schema/provenance authority", "content", NormalizationGateStatus::Ready, {"no implicit GameData move", "loader authority", "provenance"}},
        {"construction", "Converge Shipyard onto sparse parametric + authored kitbash + semantic socket assembly authority", "shipyard", NormalizationGateStatus::InProgress, {"BuildElement", "commands", "compiled runtime product"}},
        {"planet-continuity", "Implement continuous planet streaming from orbital representation through atmosphere to local surface physics", "world", NormalizationGateStatus::NotStarted, {"spherical LOD", "atmosphere", "landing", "surface vehicles"}},
        {"jump-gates", "Implement physical interstellar jump gates with destination-ready warp-tunnel handoff", "navigation", NormalizationGateStatus::NotStarted, {"gate entity", "transit state", "same ShipId", "destination handoff"}}
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
