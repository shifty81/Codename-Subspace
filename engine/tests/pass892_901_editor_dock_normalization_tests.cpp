#include "editor/EditorDockSystem.h"
#include "editor/EditorWorkspaceStateSystem.h"
#include "editor/ProjectWideEditorNormalizationSystem.h"
#include "editor/SubspaceEditorLayoutSystem.h"

#include <algorithm>
#include <iostream>

using namespace subspace;
namespace {
int failures = 0;
int assertions = 0;
void Check(bool ok, const char* name) {
    ++assertions;
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << name << "\n";
    if (!ok) ++failures;
}
bool Has(const std::vector<std::string>& values, const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}
}

int main() {
    std::cout << "[Pass892-901 Editor Dock Normalization]\n";

    auto dock = EditorDockSystem::CreateDefault(EditorWorkspaceKind::Shipyard);
    std::string error;
    Check(EditorDockSystem::Validate(dock, &error), "Pass892 default editor dock tree validates as one canonical workspace authority");

    const auto initialOpen = EditorDockSystem::OpenPanelIds(dock);
    Check(Has(initialOpen, "asset_browser") && Has(initialOpen, "outliner") && Has(initialOpen, "inspector") && Has(initialOpen, "viewport"),
          "Pass893 default dock workspace exposes Assets, Outliner, Inspector and Viewport without making Assets an exclusive mode");

    Check(EditorDockSystem::OpenPanel(dock, "diagnostics") && EditorDockSystem::OpenPanel(dock, "history") &&
              Has(EditorDockSystem::OpenPanelIds(dock), "diagnostics") && Has(EditorDockSystem::OpenPanelIds(dock), "history"),
          "Pass894 any registered editor panel can be opened alongside the current workspace");

    Check(EditorDockSystem::MovePanel(dock, "history", "right") && EditorDockSystem::LeafForPanel(dock, "history") == "right" &&
              EditorDockSystem::ActivatePanel(dock, "history"),
          "Pass895 panels can move between dock tab groups and become the active tab");

    Check(EditorDockSystem::SplitLeaf(dock, "left", "left.secondary", EditorDockSplitAxis::Vertical, .55f, "diagnostics") &&
              EditorDockSystem::LeafForPanel(dock, "diagnostics") == "left.secondary" && EditorDockSystem::Validate(dock, &error),
          "Pass896 users can split a dock region and place a panel into the new split");

    Check(EditorDockSystem::FloatPanel(dock, "history", {100, 120, 460, 330}) && EditorDockSystem::IsFloating(dock, "history") &&
              EditorDockSystem::DockPanel(dock, "history", "bottom") && !EditorDockSystem::IsFloating(dock, "history"),
          "Pass897 dock panels can float and redock without creating a second panel authority");

    Check(!EditorDockSystem::ClosePanel(dock, "viewport") && EditorDockSystem::ClosePanel(dock, "asset_browser") &&
              !Has(EditorDockSystem::OpenPanelIds(dock), "asset_browser") && EditorDockSystem::OpenPanel(dock, "asset_browser"),
          "Pass898 required viewport stays protected while ordinary panels can close and reopen");

    const auto materialized = EditorDockSystem::Materialize(dock, 1920, 1080, 42.0f);
    const bool viewportActive = std::any_of(materialized.begin(), materialized.end(), [](const auto& p) {
        return p.panelId == "viewport" && p.visible && p.active && p.rect.width > 0 && p.rect.height > 0;
    });
    Check(viewportActive, "Pass899 dock tree materializes into bounded viewport/panel rectangles for the renderer");

    EditorWorkspaceStateSystem state;
    state.SetDockWorkspace(dock);
    auto restored = state.GetDockWorkspace(EditorWorkspaceKind::Shipyard);
    Check(EditorDockSystem::LeafForPanel(restored, "history") == "bottom" && EditorDockSystem::FindNode(restored, "left.secondary") != nullptr,
          "Pass900 per-workspace state persists the user's dock arrangement rather than resetting to the Asset tab");

    const auto legacyLayout = SubspaceEditorLayoutSystem::Build(1920, 1080, true, true);
    std::string layoutError;
    const auto normalization = ProjectWideEditorNormalizationSystem::Audit();
    const bool hasDockAuthority = std::any_of(normalization.entries.begin(), normalization.entries.end(), [](const auto& e) {
        return e.domain == EditorNormalizationDomain::DockWorkspace && e.normalized && e.authority == "EditorDockSystem";
    });
    Check(SubspaceEditorLayoutSystem::Validate(legacyLayout, &layoutError) && hasDockAuthority,
          "Pass901 legacy editor layout now projects from the canonical dock authority and normalization audit reports it");

    std::cout << "Pass892-901 assertions: " << (assertions - failures) << " / " << assertions << " passed\n";
    return failures ? 1 : 0;
}
