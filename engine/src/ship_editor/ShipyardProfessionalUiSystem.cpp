#include "ship_editor/ShipyardProfessionalUiSystem.h"

#include <algorithm>

namespace subspace {

std::vector<ShipyardPanelDescriptor> ShipyardProfessionalUiSystem::Panels() {
    using D = ShipyardPanelDockHint;
    using W = ShipyardWorkspaceMode;
    return {
        {"asset-browser", "Asset Browser", D::Left, 280, 260, false, false, {W::Build, W::Interior, W::Systems, W::Appearance}},
        {"outliner", "Outliner", D::RightTop, 260, 180, false, false, {W::Build, W::Interior, W::Systems, W::Appearance, W::Test}},
        {"properties", "Properties", D::RightBottom, 300, 260, false, false, {W::Build, W::Interior, W::Systems, W::Appearance, W::Test}},
        {"history", "History", D::Bottom, 300, 160, false, false, {W::Build, W::Interior, W::Systems, W::Appearance}},
        {"validation", "Validation", D::Bottom, 320, 160, false, false, {W::Build, W::Interior, W::Systems, W::Appearance, W::Test}},
        {"console", "Console", D::Hidden, 420, 180, false, true, {}},
        {"generator", "Generator", D::Hidden, 320, 240, false, true, {W::Build, W::Pcg}},
        {"interior-program", "Interior Program", D::Hidden, 320, 240, false, true, {W::Interior}},
        {"apertures-hangars", "Apertures & Hangars", D::Hidden, 320, 240, false, true, {W::Interior}},
        {"systems", "Systems", D::Hidden, 320, 240, false, false, {W::Systems}},
        {"play-test", "Play / Test", D::Hidden, 320, 220, false, false, {W::Test}},
        {"project-tools", "Forge / Project Tools", D::Hidden, 420, 240, false, true, {W::ProjectTools}},
        {"raw-authoring", "Raw Authoring", D::Hidden, 420, 300, false, true, {W::Authoring}},
        {"pcg-proof", "PCG Proof", D::Hidden, 420, 260, false, true, {W::Pcg}},
        {"dev-world", "Dev World", D::Hidden, 420, 260, false, true, {W::DevWorld}}
    };
}

std::vector<ShipyardLayoutPreset> ShipyardProfessionalUiSystem::LayoutPresets() {
    return {
        {"BUILD", "Build", {"asset-browser", "outliner", "properties", "history", "validation"}, false},
        {"INTERIOR", "Interior", {"asset-browser", "outliner", "properties", "interior-program", "apertures-hangars", "validation"}, false},
        {"SYSTEMS", "Systems", {"outliner", "properties", "systems", "validation"}, false},
        {"APPEARANCE", "Appearance", {"asset-browser", "outliner", "properties", "history"}, false},
        {"TEST", "Test", {"outliner", "properties", "play-test", "validation"}, false},
        {"PCG", "PCG Lab", {"asset-browser", "generator", "pcg-proof", "validation", "console"}, true},
        {"DEBUG", "Developer Debug", {"outliner", "properties", "validation", "console", "project-tools"}, true},
        {"MINIMAL", "Minimal", {"outliner", "properties"}, false}
    };
}

std::vector<ShipyardToolDescriptor> ShipyardProfessionalUiSystem::PrimaryTools() {
    return {
        {"tool.select", "Select", "Q", "Select modules and authored objects"},
        {"tool.move", "Move", "W", "Move selection in View, Ship, or Local space"},
        {"tool.rotate", "Rotate", "E", "Rotate selection using viewport gizmos"},
        {"tool.scale", "Scale", "R", "Scale within certified morph limits"},
        {"tool.attach", "Attach", "A", "Place using the module primary mount surface"},
        {"tool.measure", "Measure", "M", "Measure physical clearances in meters"}
    };
}

std::vector<ShipyardWorkspaceMode> ShipyardProfessionalUiSystem::PrimaryWorkspaceStrip() {
    return {ShipyardWorkspaceMode::Build,
            ShipyardWorkspaceMode::Interior,
            ShipyardWorkspaceMode::Systems,
            ShipyardWorkspaceMode::Appearance,
            ShipyardWorkspaceMode::Test};
}

std::vector<ShipyardWorkspaceMode> ShipyardProfessionalUiSystem::AdvancedWorkspaceMenu() {
    return {ShipyardWorkspaceMode::Model,
            ShipyardWorkspaceMode::Pcg,
            ShipyardWorkspaceMode::World,
            ShipyardWorkspaceMode::DevWorld,
            ShipyardWorkspaceMode::ProjectTools,
            ShipyardWorkspaceMode::Authoring};
}

bool ShipyardProfessionalUiSystem::PanelAppliesTo(const ShipyardPanelDescriptor& panel,
                                                   ShipyardWorkspaceMode workspace) {
    return panel.workspaces.empty() || std::find(panel.workspaces.begin(), panel.workspaces.end(), workspace) != panel.workspaces.end();
}

} // namespace subspace
