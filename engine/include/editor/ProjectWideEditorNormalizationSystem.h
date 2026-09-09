#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class EditorNormalizationDomain {
    UiTheme,
    UiHelp,
    InputRouting,
    WorkspaceShell,
    Selection,
    CommandHistory,
    AssetBrowser,
    PlacementResolver,
    InspectorSchema,
    ContextActions,
    VisualAttachments,
    StationTaxonomy,
    StationKitbash,
    SurfaceAuthority,
    ItemIdentity,
    BlueprintAuthority,
    PcgDesignLanguage,
    ContentLayout,
    LegacyReference,
    UniversalKitbashSize,
    ShipClassAuthority,
    ShipRoleAuthority,
    ConstructionDomain,
    ConstructionBlueprint,
    FactionHullLineage,
    RuntimePcgCertification,
    KitbashReviewCatalog,
    ConstructionSymmetry,
    ConstructionCamera,
    ThumbnailAuthority
};

struct EditorNormalizationEntry {
    EditorNormalizationDomain domain = EditorNormalizationDomain::UiTheme;
    std::string authority;
    bool normalized = false;
    bool deferredRepositoryMove = false;
    std::string note;
};

struct EditorNormalizationReport {
    std::vector<EditorNormalizationEntry> entries;
    bool runtimeNormalized = false;
    bool repositoryLayoutNormalized = false;
    std::vector<std::string> remainingRepositoryActions;
};

class ProjectWideEditorNormalizationSystem {
public:
    static EditorNormalizationReport Audit();
    static const char* DomainName(EditorNormalizationDomain domain);
};

} // namespace subspace
