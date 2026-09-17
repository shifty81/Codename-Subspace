#include "ui/SubspaceUiFramework.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace subspace;
namespace {
void Check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "[FAIL] " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
    std::cout << "[PASS] " << message << '\n';
}
bool Has(const std::vector<SubspaceDockLayout>& layouts,
         const std::string& id, bool floating = false) {
    for (const auto& layout : layouts)
        if (layout.panelId == id && layout.visible && layout.floating == floating)
            return true;
    return false;
}
int DockCount(const std::vector<SubspaceDockLayout>& layouts, const std::string& a,
              const std::string& b) {
    int count = 0;
    for (const auto& layout : layouts)
        if ((layout.panelId == a || layout.panelId == b) && !layout.floating)
            ++count;
    return count;
}
std::vector<SubspaceDockLayout> Layout(const SubspaceDockWorkspace& workspace) {
    return SubspaceDockSystem::Materialize(workspace, 1280, 768);
}
}
int main() {
    auto workspace = SubspaceDockSystem::CreateMinimalWorkspace("pass1507");
    SubspaceDockPanel properties;
    properties.id = "properties";
    properties.title = "Properties";
    properties.defaultLeafId = "right";
    SubspaceDockPanel modeling = properties;
    modeling.id = "modeling";
    modeling.title = "Modeling";

    Check(SubspaceDockSystem::RegisterPanel(workspace, properties), "register Properties");
    Check(SubspaceDockSystem::RegisterPanel(workspace, modeling), "register Modeling");
    Check(Has(Layout(workspace), "properties") &&
          !Has(Layout(workspace), "modeling") &&
          DockCount(Layout(workspace), "properties", "modeling") == 1,
          "tab stack paints only its initial active panel");
    Check(SubspaceDockSystem::ActivatePanel(workspace, "modeling"), "activate Modeling");
    Check(Has(Layout(workspace), "modeling") &&
          !Has(Layout(workspace), "properties") &&
          DockCount(Layout(workspace), "properties", "modeling") == 1,
          "activation paints only the selected tab");
    Check(SubspaceDockSystem::ClosePanel(workspace, "modeling"), "close Modeling");
    Check(Has(Layout(workspace), "properties") &&
          !Has(Layout(workspace), "modeling"),
          "closing active tab restores available sibling");
    Check(SubspaceDockSystem::OpenPanel(workspace, "modeling"), "reopen Modeling");
    Check(Has(Layout(workspace), "modeling") && !Has(Layout(workspace), "properties"),
          "reopening a tab makes it active");
    Check(SubspaceDockSystem::FloatPanel(workspace, "modeling", {100, 120, 350, 300}),
          "float Modeling");
    Check(Has(Layout(workspace), "properties") && Has(Layout(workspace), "modeling", true) &&
          !Has(Layout(workspace), "modeling"),
          "floating selected panel exposes docked sibling without duplicating panel");
    Check(SubspaceDockSystem::DockPanel(workspace, "modeling", "right"),
          "redock Modeling");
    Check(Has(Layout(workspace), "modeling") && !Has(Layout(workspace), "properties"),
          "redocking makes Modeling the sole active dock panel");
    const std::string saved = SubspaceDockSystem::Serialize(workspace);
    SubspaceDockWorkspace restored;
    std::string error;
    Check(SubspaceDockSystem::Deserialize(saved, restored, &error),
          "restore serialized dock state");
    Check(Has(Layout(restored), "modeling") && !Has(Layout(restored), "properties"),
          "restored layout preserves active tab");
    Check(SubspaceDockSystem::SetAutoHide(restored, "modeling", true),
          "enable Modeling auto-hide");
    Check(SubspaceDockSystem::SetHoverReveal(restored, "modeling", false),
          "hide auto-hidden Modeling");
    Check(Has(Layout(restored), "properties") && !Has(Layout(restored), "modeling"),
          "auto-hidden active tab yields to visible sibling");
    Check(SubspaceDockSystem::SetHoverReveal(restored, "modeling", true),
          "reveal auto-hidden Modeling");
    Check(Has(Layout(restored), "modeling") && !Has(Layout(restored), "properties"),
          "hover reveal restores selected tab without overlap");
    Check(SubspaceDockSystem::ToggleCollapsed(restored, "modeling"),
          "collapse Modeling");
    Check(Has(Layout(restored), "modeling") && !Has(Layout(restored), "properties"),
          "collapsed tab retains exclusive materialization");
    Check(SubspaceDockSystem::ClosePanel(restored, "modeling"), "close active Modeling");
    Check(Has(Layout(restored), "properties") && !Has(Layout(restored), "modeling"),
          "close selects visible sibling after collapse");
    Check(SubspaceDockSystem::ClosePanel(restored, "properties"), "close Properties");
    Check(!Has(Layout(restored), "properties") && !Has(Layout(restored), "modeling"),
          "no tab renders when all panels are closed");
    std::cout << "PASS1507 dock active-tab regression PASS\n";
    return EXIT_SUCCESS;
}
