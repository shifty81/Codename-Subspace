#include "editor/SubspaceEditorAcceptanceSystem.h"
#include "editor/ShipyardEditorShellSystem.h"

namespace subspace {

EditorWorkspaceRegistry SubspaceEditorAcceptanceSystem::BuildDefaultWorkspaceRegistry() {
    return ShipyardEditorShellSystem::BuildWorkspaceRegistry();
}

EditorAcceptanceResult SubspaceEditorAcceptanceSystem::ValidateProjectWideEditor(const EditorWorkspaceRegistry& r,
                                                                                 bool theme,
                                                                                 bool help,
                                                                                 bool browser,
                                                                                 bool place,
                                                                                 bool commands,
                                                                                 bool selection) {
    EditorAcceptanceResult o;
    auto req=[&](bool ok,const char*m){if(!ok)o.failures.push_back(m);};
    req(r.Find(EditorWorkspaceKind::Shipyard),"Ship workspace missing");
    req(r.Find(EditorWorkspaceKind::Character),"Character workspace missing");
    req(r.Find(EditorWorkspaceKind::World),"World workspace missing");
    req(r.Find(EditorWorkspaceKind::Interior),"Interior workspace missing");
    req(r.Find(EditorWorkspaceKind::MaterialStudio),"Materials workspace missing");
    req(r.Find(EditorWorkspaceKind::Animation),"Animation workspace missing");
    req(r.Find(EditorWorkspaceKind::PcgStudio),"PCG workspace missing");
    req(r.Find(EditorWorkspaceKind::Vfx),"VFX workspace missing");
    req(r.Find(EditorWorkspaceKind::Audio),"Audio workspace missing");
    req(r.Find(EditorWorkspaceKind::Logic),"Logic workspace missing");
    req(r.Find(EditorWorkspaceKind::Diagnostics),"Diagnostics workspace missing");
    req(r.Find(EditorWorkspaceKind::StationBuilder),"Station compatibility context missing");
    req(r.Find(EditorWorkspaceKind::TurretLab),"Turret compatibility context missing");
    req(theme,"Shared UI theme authority missing");
    req(help,"Shared help registry missing");
    req(browser,"Universal asset browser missing");
    req(place,"Universal placement resolver missing");
    req(commands,"Global command stack missing");
    req(selection,"Global selection service missing");
    o.passed=o.failures.empty();
    return o;
}

} // namespace subspace
