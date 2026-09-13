#include "integration/ExternalTechnologyAdoptionSystem.h"

namespace subspace {

std::vector<ExternalTechnologyCandidate> ExternalTechnologyAdoptionSystem::CarbonCandidates() {
    return {
        {"carbon.mesh", "Carbon Mesh", "https://github.com/carbonengine/mesh", "MIT",
         ExternalTechnologyDisposition::EvaluateForIntegration,
         "Mesh/skeleton/animation serialization and runtime patterns; candidate import/animation backend behind Subspace asset interfaces.",
         "Must prove format/toolchain fit and preserve Subspace canonical asset IDs before adopting."},
        {"carbon.trinity", "Carbon Trinity", "https://github.com/carbonengine/trinity", "MIT",
         ExternalTechnologyDisposition::ArchitectureReference,
         "Rendering architecture, shader/compiler organization and DX11/DX12 backend reference for the renderer modernization program.",
         "Do not replace the current renderer during Foundation Convergence; repository has its own toolchain/submodule assumptions."},
        {"carbon.resources", "Carbon Resources", "https://github.com/carbonengine/resources", "MIT",
         ExternalTechnologyDisposition::EvaluateForIntegration,
         "Resource delivery, manipulation, indexing and tooling patterns for Subspace canonical asset/Vault workflows.",
         "Adoption must not create a second resource authority beside CanonicalAssetRegistry/ContentAuthorityRegistry."},
        {"carbon.destiny", "Carbon Destiny", "https://github.com/carbonengine/destiny", "MIT",
         ExternalTechnologyDisposition::ArchitectureReference,
         "Large-scale spaceship-world simulation, benchmarking and authoritative simulation design reference.",
         "Current public build instructions still require private Perforce dependencies; treat as reference until independently buildable and interface-compatible."},
        {"carbon.core", "Carbon Core", "https://github.com/carbonengine/core", "MIT",
         ExternalTechnologyDisposition::SelectiveAdoption,
         "Low-level cross-platform utilities only where they replace a bounded Subspace utility cleanly.",
         "Avoid importing a competing platform/runtime foundation wholesale."},
        {"carbon.io", "Carbon IO", "https://github.com/carbonengine/io", "MIT",
         ExternalTechnologyDisposition::Defer,
         "Compare with GameNetworkingSockets/other transport candidates when the 2-8 player networking transport pass begins.",
         "Networking replication/authority design must be completed before selecting low-level transport."},
        {"carbon.audio", "Carbon Audio", "https://github.com/carbonengine/audio", "MIT",
         ExternalTechnologyDisposition::ArchitectureReference,
         "Spatial audio, prioritization and Wwise integration patterns.",
         "Requires Wwise SDK/licensing/toolchain; Subspace should keep an audio-backend abstraction."},
        {"carbon.spatial_audio_clustering", "Carbon Spatial Audio Clustering", "https://github.com/carbonengine/spatial-audio-clustering", "Apache-2.0",
         ExternalTechnologyDisposition::EvaluateForIntegration,
         "Density-aware clustering for dense ship/turret/particle audio scenes; relevant to large battles and busy settlements.",
         "Implementation is a Wwise plugin; algorithm may be adapted behind the selected audio backend after profiling."}
    };
}

const char* ExternalTechnologyAdoptionSystem::DispositionName(ExternalTechnologyDisposition disposition) {
    switch (disposition) {
        case ExternalTechnologyDisposition::EvaluateForIntegration: return "EVALUATE_FOR_INTEGRATION";
        case ExternalTechnologyDisposition::SelectiveAdoption: return "SELECTIVE_ADOPTION";
        case ExternalTechnologyDisposition::ArchitectureReference: return "ARCHITECTURE_REFERENCE";
        case ExternalTechnologyDisposition::Defer: return "DEFER";
    }
    return "UNKNOWN";
}

} // namespace subspace
