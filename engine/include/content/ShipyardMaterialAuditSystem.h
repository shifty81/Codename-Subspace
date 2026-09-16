#pragma once

#include "content/ShipyardCertificationSystem.h"
#include "content/UniversalKitbashAuthority.h"

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

enum class ShipyardMaterialAuditIssueKind {
    NoMaterialSlots,
    ReferencedMtlUnresolved,
    MaterialSlotUnresolved,
    MissingBaseColorTexture,
    MissingNormalTexture,
    MissingMetallicTexture,
    MissingRoughnessTexture,
    MissingTexcoords,
    MissingNormals
};

struct ShipyardMaterialAuditIssue {
    ShipyardMaterialAuditIssueKind kind = ShipyardMaterialAuditIssueKind::NoMaterialSlots;
    std::string materialName;
    std::string message;
    bool blocking = false;
};

struct ShipyardMaterialAuditReport {
    std::string sourceName;
    std::size_t materialSlots = 0;
    std::size_t resolvedSlots = 0;
    std::size_t texturedSlots = 0;
    bool semanticFallbackRecommended = false;
    KitbashMaterialCertification certification = KitbashMaterialCertification::ReviewRequired;
    std::vector<ShipyardMaterialAuditIssue> issues;
    bool Healthy() const { return certification == KitbashMaterialCertification::Complete; }
};

struct ShipyardMaterialCorpusAudit {
    std::size_t modules = 0;
    std::size_t complete = 0;
    std::size_t normalizedFallback = 0;
    std::size_t reviewRequired = 0;
    std::size_t brokenDependency = 0;
    std::vector<ShipyardMaterialAuditReport> reports;
};

/// PASS1498-1505: one material-health authority for Shipyard/kitbash intake.
/// This does not mutate vendor/source assets. It reports missing dependencies,
/// UV/normal coverage and texture-map gaps, then maps that evidence onto the
/// existing UniversalKitbashAuthority material certification vocabulary.
class ShipyardMaterialAuditSystem {
public:
    static const char* IssueName(ShipyardMaterialAuditIssueKind kind);
    static ShipyardMaterialAuditReport Audit(const ShipyardObjCertification& module);
    static ShipyardMaterialCorpusAudit AuditCorpus(const std::vector<ShipyardObjCertification>& modules);
};

} // namespace subspace
