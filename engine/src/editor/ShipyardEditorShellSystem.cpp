#include "editor/ShipyardEditorShellSystem.h"

#include <algorithm>

namespace subspace {
namespace {

void AddPanel(EditorDockWorkspace& dock,
              const char* id,
              const char* title,
              const char* leaf,
              bool visible = false,
              bool canFloat = true) {
    EditorDockSystem::RegisterPanel(dock, {id, title, leaf, visible, true, canFloat, 160.0f, 100.0f});
    if (visible) EditorDockSystem::OpenPanel(dock, id);
}

} // namespace

EditorWorkspaceRegistry ShipyardEditorShellSystem::BuildWorkspaceRegistry() {
    EditorWorkspaceRegistry r;
    r.Register({EditorWorkspaceKind::Layout, "layout", "Layout", "General-purpose scene inspection and authoring layout.", false, true});
    r.Register({EditorWorkspaceKind::Shipyard, "ship", "Ship", "Ship/station/vehicle assembly, fitting, appearance and validation.", true, true});
    r.Register({EditorWorkspaceKind::Character, "character", "Character", "Character sculpting, appearance, rigging, apparel and portrait authoring.", true, true});
    r.Register({EditorWorkspaceKind::World, "world", "World", "Solar-system, planet, settlement, POI and streaming authoring.", false, true});
    r.Register({EditorWorkspaceKind::Interior, "interior", "Interior", "Rooms, portals, service spaces, props and walkable interior authoring.", false, true});
    r.Register({EditorWorkspaceKind::Modeling, "modeling", "Modeling", "Parametric geometry, mesh review, sockets and construction-source authoring.", false, true});
    r.Register({EditorWorkspaceKind::MaterialStudio, "materials", "Materials", "Materials, liveries, paint zones, patterns, decals and surface policy.", false, true});
    r.Register({EditorWorkspaceKind::Animation, "animation", "Animation", "Skeletons, retargeting, clips, timelines and animation-state authoring.", false, true});
    r.Register({EditorWorkspaceKind::PcgStudio, "pcg", "PCG", "DesignDNA, procedural grammars, exemplars and deterministic generation.", false, true});
    r.Register({EditorWorkspaceKind::Vfx, "vfx", "VFX", "Particles, shields, thrusters, impacts, atmosphere and effect authoring.", false, true});
    r.Register({EditorWorkspaceKind::Audio, "audio", "Audio", "Spatial audio, cue binding, mixing, priorities and authoring.", false, true});
    r.Register({EditorWorkspaceKind::Logic, "logic", "Logic", "Gameplay logic, behavior, interaction and validation graphs.", false, true});
    r.Register({EditorWorkspaceKind::Diagnostics, "diagnostics", "Diagnostics", "Validation, runtime diff, profiler, logs, history and debug tools.", false, true});

    // Compatibility/specialized document contexts remain available inside the
    // single editor shell without becoming separate applications.
    r.Register({EditorWorkspaceKind::StationBuilder, "station", "Station", "Station-focused assembly context within the Ship workspace.", true, true});
    r.Register({EditorWorkspaceKind::TurretLab, "turret", "Turret", "Turret/weapon subassembly context within the Ship workspace.", false, true});
    r.Register({EditorWorkspaceKind::PlanetSector, "planet_sector", "Planet Sector", "Focused planetary region context within World.", false, true});
    r.Register({EditorWorkspaceKind::BlueprintLibrary, "blueprints", "Blueprint Library", "Reusable authored definitions and variants.", true, true});
    return r;
}

std::vector<EditorWorkspaceKind> ShipyardEditorShellSystem::PrimaryWorkspaceOrder() {
    return {
        EditorWorkspaceKind::Layout,
        EditorWorkspaceKind::Shipyard,
        EditorWorkspaceKind::Character,
        EditorWorkspaceKind::World,
        EditorWorkspaceKind::Interior,
        EditorWorkspaceKind::Modeling,
        EditorWorkspaceKind::MaterialStudio,
        EditorWorkspaceKind::Animation,
        EditorWorkspaceKind::PcgStudio,
        EditorWorkspaceKind::Vfx,
        EditorWorkspaceKind::Audio,
        EditorWorkspaceKind::Logic,
        EditorWorkspaceKind::Diagnostics
    };
}

bool ShipyardEditorShellSystem::IsPrimaryWorkspace(EditorWorkspaceKind workspace) {
    const auto order = PrimaryWorkspaceOrder();
    return std::find(order.begin(), order.end(), workspace) != order.end();
}

const char* ShipyardEditorShellSystem::WorkspaceName(EditorWorkspaceKind workspace) {
    switch (workspace) {
        case EditorWorkspaceKind::Layout: return "Layout";
        case EditorWorkspaceKind::Shipyard: return "Ship";
        case EditorWorkspaceKind::Character: return "Character";
        case EditorWorkspaceKind::World: return "World";
        case EditorWorkspaceKind::Interior: return "Interior";
        case EditorWorkspaceKind::Modeling: return "Modeling";
        case EditorWorkspaceKind::MaterialStudio: return "Materials";
        case EditorWorkspaceKind::Animation: return "Animation";
        case EditorWorkspaceKind::PcgStudio: return "PCG";
        case EditorWorkspaceKind::Vfx: return "VFX";
        case EditorWorkspaceKind::Audio: return "Audio";
        case EditorWorkspaceKind::Logic: return "Logic";
        case EditorWorkspaceKind::Diagnostics: return "Diagnostics";
        case EditorWorkspaceKind::StationBuilder: return "Station";
        case EditorWorkspaceKind::TurretLab: return "Turret";
        case EditorWorkspaceKind::PlanetSector: return "Planet Sector";
        case EditorWorkspaceKind::BlueprintLibrary: return "Blueprint Library";
    }
    return "Unknown";
}

EditorDockWorkspace ShipyardEditorShellSystem::CreateWorkspaceDock(EditorWorkspaceKind workspace) {
    auto dock = EditorDockSystem::CreateDefault(workspace);

    switch (workspace) {
        case EditorWorkspaceKind::Shipyard:
        case EditorWorkspaceKind::StationBuilder:
        case EditorWorkspaceKind::TurretLab:
            AddPanel(dock, "ship_hierarchy", "Assembly", "left", true);
            AddPanel(dock, "ship_fit", "Fit / Systems", "right", true);
            AddPanel(dock, "ship_stats", "Stats Delta", "right", false);
            AddPanel(dock, "ship_appearance", "Appearance", "right", false);
            AddPanel(dock, "ship_validation", "Ship Validation", "bottom", false);
            AddPanel(dock, "socket_authoring", "Socket Authoring", "bottom", false);
            break;
        case EditorWorkspaceKind::Character:
            AddPanel(dock, "character_sculpt", "Sculpt", "right", true);
            AddPanel(dock, "character_appearance", "Appearance", "right", false);
            AddPanel(dock, "character_apparel", "Apparel / Mods", "right", false);
            AddPanel(dock, "character_portrait", "Portrait", "right", false);
            AddPanel(dock, "character_rig", "Rig", "bottom", false);
            AddPanel(dock, "character_morphs", "Morph Channels", "bottom", false);
            AddPanel(dock, "animation_timeline", "Timeline", "bottom", false);
            break;
        case EditorWorkspaceKind::World:
        case EditorWorkspaceKind::PlanetSector:
            AddPanel(dock, "world_hierarchy", "World Hierarchy", "left", true);
            AddPanel(dock, "terrain", "Terrain", "right", true);
            AddPanel(dock, "settlements", "Settlements / POIs", "right", false);
            AddPanel(dock, "streaming", "Streaming", "bottom", false);
            break;
        case EditorWorkspaceKind::Interior:
            AddPanel(dock, "interior_hierarchy", "Interior Hierarchy", "left", true);
            AddPanel(dock, "portals", "Rooms / Portals", "right", true);
            AddPanel(dock, "clearance", "Clearance", "bottom", false);
            break;
        case EditorWorkspaceKind::Modeling:
            AddPanel(dock, "mesh_data", "Mesh Data", "right", true);
            AddPanel(dock, "parametric", "Parametric", "right", false);
            AddPanel(dock, "uv_surface", "UV / Surface", "bottom", false);
            break;
        case EditorWorkspaceKind::Animation:
            AddPanel(dock, "animation_timeline", "Timeline", "bottom", true);
            AddPanel(dock, "animation_graph", "Animation Graph", "right", true);
            AddPanel(dock, "rig", "Rig / Skeleton", "left", false);
            break;
        case EditorWorkspaceKind::MaterialStudio:
            AddPanel(dock, "material_graph", "Material Graph", "bottom", true);
            AddPanel(dock, "surface_policy", "Surface Policy", "right", true);
            break;
        case EditorWorkspaceKind::PcgStudio:
            AddPanel(dock, "pcg_graph", "PCG Graph", "bottom", true);
            AddPanel(dock, "pcg_parameters", "Parameters", "right", true);
            break;
        case EditorWorkspaceKind::Vfx:
            AddPanel(dock, "vfx_graph", "VFX Graph", "bottom", true);
            AddPanel(dock, "vfx_properties", "Effect Properties", "right", true);
            break;
        case EditorWorkspaceKind::Audio:
            AddPanel(dock, "audio_mixer", "Mixer", "bottom", true);
            AddPanel(dock, "audio_cues", "Cues / Spatial", "right", true);
            break;
        case EditorWorkspaceKind::Logic:
            AddPanel(dock, "logic_graph", "Logic Graph", "center", true, false);
            AddPanel(dock, "logic_variables", "Variables", "right", true);
            break;
        case EditorWorkspaceKind::Diagnostics:
            EditorDockSystem::OpenPanel(dock, "validation");
            EditorDockSystem::OpenPanel(dock, "diagnostics");
            EditorDockSystem::OpenPanel(dock, "history");
            EditorDockSystem::OpenPanel(dock, "runtime_diff");
            break;
        default:
            break;
    }
    return dock;
}

} // namespace subspace
