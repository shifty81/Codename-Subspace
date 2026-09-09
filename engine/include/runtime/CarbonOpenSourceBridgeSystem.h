#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class CarbonBridgeDisposition { NativeAdaptation, IsolatedLibraryCandidate, ArchitectureReference, RejectedDirectRuntime };

struct CarbonOpenSourceComponent {
    std::string name;
    std::string upstream;
    std::string license;
    CarbonBridgeDisposition disposition = CarbonBridgeDisposition::ArchitectureReference;
    std::string subspaceAuthority;
    std::string rationale;
};

struct CarbonOpenSourceBridgeReport {
    std::vector<CarbonOpenSourceComponent> components;
    bool preservesSubspaceRuntime = true;
    bool importsGameContent = false;
    bool importsTrademarkedUi = false;
};

class CarbonOpenSourceBridgeSystem {
public:
    CarbonOpenSourceBridgeReport Build2026Plan() const;
    const CarbonOpenSourceComponent* Find(const CarbonOpenSourceBridgeReport& report,const std::string& name) const;
};

} // namespace subspace
