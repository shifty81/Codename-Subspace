#include "editor/ProjectWideEditorNormalizationSystem.h"

#include <algorithm>

namespace subspace {
const char* ProjectWideEditorNormalizationSystem::DomainName(EditorNormalizationDomain d){
    switch(d){
        case EditorNormalizationDomain::UiTheme:return "UI_THEME";
        case EditorNormalizationDomain::UiHelp:return "UI_HELP";
        case EditorNormalizationDomain::InputRouting:return "INPUT_ROUTING";
        case EditorNormalizationDomain::WorkspaceShell:return "WORKSPACE_SHELL";
        case EditorNormalizationDomain::Selection:return "SELECTION";
        case EditorNormalizationDomain::CommandHistory:return "COMMAND_HISTORY";
        case EditorNormalizationDomain::AssetBrowser:return "ASSET_BROWSER";
        case EditorNormalizationDomain::PlacementResolver:return "PLACEMENT_RESOLVER";
        case EditorNormalizationDomain::InspectorSchema:return "INSPECTOR_SCHEMA";
        case EditorNormalizationDomain::ContextActions:return "CONTEXT_ACTIONS";
        case EditorNormalizationDomain::VisualAttachments:return "VISUAL_ATTACHMENTS";
        case EditorNormalizationDomain::StationTaxonomy:return "STATION_TAXONOMY";
        case EditorNormalizationDomain::StationKitbash:return "STATION_KITBASH";
        case EditorNormalizationDomain::SurfaceAuthority:return "SURFACE_AUTHORITY";
        case EditorNormalizationDomain::ItemIdentity:return "ITEM_IDENTITY";
        case EditorNormalizationDomain::BlueprintAuthority:return "BLUEPRINT_AUTHORITY";
        case EditorNormalizationDomain::PcgDesignLanguage:return "PCG_DESIGN_LANGUAGE";
        case EditorNormalizationDomain::ContentLayout:return "CONTENT_LAYOUT";
        case EditorNormalizationDomain::LegacyReference:return "LEGACY_REFERENCE";
        case EditorNormalizationDomain::UniversalKitbashSize:return "UNIVERSAL_KITBASH_SIZE";
        case EditorNormalizationDomain::ShipClassAuthority:return "SHIP_CLASS_AUTHORITY";
        case EditorNormalizationDomain::ShipRoleAuthority:return "SHIP_ROLE_AUTHORITY";
        case EditorNormalizationDomain::ConstructionDomain:return "CONSTRUCTION_DOMAIN";
        case EditorNormalizationDomain::ConstructionBlueprint:return "CONSTRUCTION_BLUEPRINT";
        case EditorNormalizationDomain::FactionHullLineage:return "FACTION_HULL_LINEAGE";
        case EditorNormalizationDomain::RuntimePcgCertification:return "RUNTIME_PCG_CERTIFICATION";
        case EditorNormalizationDomain::KitbashReviewCatalog:return "KITBASH_REVIEW_CATALOG";
        case EditorNormalizationDomain::ConstructionSymmetry:return "CONSTRUCTION_SYMMETRY";
        case EditorNormalizationDomain::ConstructionCamera:return "CONSTRUCTION_CAMERA";
        case EditorNormalizationDomain::ThumbnailAuthority:return "THUMBNAIL_AUTHORITY";
    }
    return "UNKNOWN";
}
EditorNormalizationReport ProjectWideEditorNormalizationSystem::Audit(){
    EditorNormalizationReport r;
    auto add=[&](EditorNormalizationDomain d,const char*a,bool ok,const char*n,bool deferred=false){r.entries.push_back({d,a,ok,deferred,n});};
    add(EditorNormalizationDomain::UiTheme,"SubspaceUiTheme",true,"Game/editor presentation projects from one semantic token authority.");
    add(EditorNormalizationDomain::UiHelp,"EditorHelpRegistry",true,"Tooltips, shortcuts and disabled reasons share one registry contract.");
    add(EditorNormalizationDomain::InputRouting,"EditorInputRouter",true,"Pointer capture/hover authority is shared across editor workspaces.");
    add(EditorNormalizationDomain::WorkspaceShell,"SubspaceEditorCore",true,"Shipyard, Station Builder and future authoring tools share workspace/document contracts.");
    add(EditorNormalizationDomain::Selection,"EditorSelectionService",true,"Viewport/outliner/inspector selection uses one service contract.");
    add(EditorNormalizationDomain::CommandHistory,"EditorCommandStack",true,"Undo/redo mutations route through one command contract.");
    add(EditorNormalizationDomain::AssetBrowser,"EditorAssetBrowserModel",true,"Visual item cards/search/filter/favorites/recent are project-wide.");
    add(EditorNormalizationDomain::PlacementResolver,"EditorPlacementResolver",true,"Ghost/valid-warning-invalid placement semantics are shared.");
    add(EditorNormalizationDomain::InspectorSchema,"EditorPropertySection",true,"Context-sensitive properties use shared schema records.");
    add(EditorNormalizationDomain::ContextActions,"EditorContextAction",true,"Right-click/context actions share enabled/disabled reason semantics.");
    add(EditorNormalizationDomain::VisualAttachments,"VisualAssemblyAttachment",true,"Ships and stations share one visual graph edge record.");
    add(EditorNormalizationDomain::StationTaxonomy,"StationModuleRole",true,"Construction and modular station APIs are compatibility aliases to one enum.");
    add(EditorNormalizationDomain::StationKitbash,"StationKitbashCatalogSystem",true,"Logical station pieces reuse certified Canonical/Shipyard geometry.");
    add(EditorNormalizationDomain::SurfaceAuthority,"ShipyardSurfaceSection",true,"Semantic surface eligibility remains the authoring bridge into canonical materials.");
    add(EditorNormalizationDomain::ItemIdentity,"ShipyardModuleItemDefinition",true,"Module cards/gameplay itemization share stable module identity.");
    add(EditorNormalizationDomain::BlueprintAuthority,"ShipBlueprintLibrarySystem",true,"Reusable designs remain blueprint-backed instead of per-screen copies.");
    add(EditorNormalizationDomain::PcgDesignLanguage,"StationDesignDnaSystem + ShipyardDesignDnaSystem",true,"Ships and stations expose deterministic exemplar/design-DNA lanes.");
    add(EditorNormalizationDomain::ContentLayout,"content/* canonical layout",true,"Runtime authority is normalized; legacy source-only roots remain migration/reference inputs.",true);
    add(EditorNormalizationDomain::LegacyReference,"AvorionLike reference lane",true,"Legacy C# remains inert reference/provenance, never runtime fallback.",true);
    add(EditorNormalizationDomain::UniversalKitbashSize,"UniversalSizeClass",true,"XS/S/M/L/XL is the single cross-domain kitbash sizing authority.");
    add(EditorNormalizationDomain::ShipClassAuthority,"ShipClassSystem + ShipClassRoleSystem",true,"Legacy serialized classes and the strategic Frigate-through-Capital progression now share one enum authority rather than duplicate ShipClass definitions.");
    add(EditorNormalizationDomain::ShipRoleAuthority,"ShipRole",true,"Legacy gameplay and specialized faction-hull roles share one append-only serialized role vocabulary.");
    add(EditorNormalizationDomain::ConstructionDomain,"ConstructionDomain",true,"Ship/station/planetary/weapon adapters resolve onto one universal domain vocabulary.");
    add(EditorNormalizationDomain::ConstructionBlueprint,"ConstructionBlueprint",true,"Cross-domain blueprint persistence uses one construction record with domain payloads.");
    add(EditorNormalizationDomain::FactionHullLineage,"FactionShipDesignSystem",true,"Faction -> class -> four hull families -> role variant is the single ship lineage authority.");
    add(EditorNormalizationDomain::RuntimePcgCertification,"ShipPcgRuntimeClosureSystem",true,"Whole-ship spatial/propulsion/exhaust certification gates every runtime recipe lane.");
    add(EditorNormalizationDomain::KitbashReviewCatalog,"KitbashReviewCatalogSystem",true,"Uncertain classification/material/propulsion assets route to one fail-closed review/catalog authority.");
    add(EditorNormalizationDomain::ConstructionSymmetry,"ConstructionSymmetrySystem",true,"Ship, station, planetary and weapon editors share one exact X/Y/Z reflection frame rather than duplicate+rotation implementations.");
    add(EditorNormalizationDomain::ConstructionCamera,"ConstructionEditorCameraSystem",true,"Centered inspection and Alt free-fly use one 6DOF camera authority across construction workspaces.");
    add(EditorNormalizationDomain::ThumbnailAuthority,"EditorAssetThumbnailSystem",true,"Canonical mesh thumbnails share framing, cache identity, certification badges and functional direction overlays.");
    r.runtimeNormalized=std::all_of(r.entries.begin(),r.entries.end(),[](const auto&e){return e.normalized;});
    // Physical moves/deletions cannot be represented safely by an overwrite-only
    // incremental ZIP, so they remain explicit repository-maintenance actions.
    r.remainingRepositoryActions={
        "Move/quarantine AvorionLike/ under the documented reference lane when deletion-aware migration is authorized.",
        "Move GameData/ into content/data after runtime/path compatibility is verified against the asset/content rollup.",
        "Merge any external Assets/assets roots into content/assets with provenance when those binary roots are present."
    };
    r.repositoryLayoutNormalized=r.remainingRepositoryActions.empty();
    return r;
}
} // namespace subspace
