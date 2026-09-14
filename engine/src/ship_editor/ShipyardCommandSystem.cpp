#include "ship_editor/ShipyardCommandSystem.h"

#include <algorithm>
#include <cctype>

namespace subspace {
namespace {

std::string Lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

ShipyardCommandResult SessionWorkspace(ShipyardWorkspaceMode workspace, ShipyardCommandContext& context) {
    if (!context.session) return {false, false, "No Shipyard session"};
    ShipyardSessionSystem::SetWorkspace(*context.session, workspace);
    return {true, false, context.session->status};
}

ShipyardCommandResult SessionTool(ShipyardTransformTool tool, ShipyardCommandContext& context) {
    if (!context.session) return {false, false, "No Shipyard session"};
    ShipyardSessionSystem::SetTool(*context.session, tool);
    return {true, false, context.session->status};
}

} // namespace

bool ShipyardCommandSystem::Register(ShipyardCommandDescriptor descriptor, ShipyardCommandHandler handler) {
    if (descriptor.id.empty() || !handler) return false;
    // Capture the key before moving the descriptor. Function-argument evaluation
    // order must never be allowed to turn moved-from descriptor.id into the map key.
    const std::string key = descriptor.id;
    return commands_.emplace(key, Record{std::move(descriptor), std::move(handler)}).second;
}

const ShipyardCommandDescriptor* ShipyardCommandSystem::Find(std::string_view id) const {
    const auto it = commands_.find(std::string(id));
    return it == commands_.end() ? nullptr : &it->second.descriptor;
}

std::vector<ShipyardCommandDescriptor> ShipyardCommandSystem::Search(std::string_view query, bool includeAdvanced) const {
    const auto needle = Lower(std::string(query));
    std::vector<ShipyardCommandDescriptor> out;
    for (const auto& pair : commands_) {
        const auto& d = pair.second.descriptor;
        if (d.advanced && !includeAdvanced) continue;
        const auto haystack = Lower(d.id + " " + d.label + " " + d.category + " " + d.defaultShortcut);
        if (needle.empty() || haystack.find(needle) != std::string::npos) out.push_back(d);
    }
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) {
        if (a.category != b.category) return a.category < b.category;
        return a.label < b.label;
    });
    return out;
}

ShipyardCommandResult ShipyardCommandSystem::Execute(std::string_view id, ShipyardCommandContext& context) const {
    const auto it = commands_.find(std::string(id));
    if (it == commands_.end()) return {false, false, "Unknown Shipyard command"};
    const auto& record = it->second;

    bool openedTransaction = false;
    if (record.descriptor.mutatesDocument && record.descriptor.undoable && context.document && context.history) {
        openedTransaction = context.history->Begin(record.descriptor.label, *context.document);
    }

    auto result = record.handler(context);
    if (openedTransaction) {
        if (result.handled && result.changed) context.history->Commit(*context.document);
        else context.history->Cancel();
    }
    if (context.session && !result.message.empty()) context.session->status = result.message;
    return result;
}

std::vector<ShipyardCommandDescriptor> ShipyardCommandSystem::All(bool includeAdvanced) const {
    return Search({}, includeAdvanced);
}

ShipyardCommandSystem ShipyardCommandSystem::BuildProfessionalDefaults() {
    ShipyardCommandSystem out;
    out.Register({"workspace.build", "Build", "Workspace", "", false, false, false},
                 [](auto& c) { return SessionWorkspace(ShipyardWorkspaceMode::Build, c); });
    out.Register({"workspace.interior", "Interior", "Workspace", "", false, false, false},
                 [](auto& c) { return SessionWorkspace(ShipyardWorkspaceMode::Interior, c); });
    out.Register({"workspace.systems", "Systems", "Workspace", "", false, false, false},
                 [](auto& c) { return SessionWorkspace(ShipyardWorkspaceMode::Systems, c); });
    out.Register({"workspace.appearance", "Appearance", "Workspace", "", false, false, false},
                 [](auto& c) { return SessionWorkspace(ShipyardWorkspaceMode::Appearance, c); });
    out.Register({"workspace.test", "Test", "Workspace", "", false, false, false},
                 [](auto& c) { return SessionWorkspace(ShipyardWorkspaceMode::Test, c); });

    out.Register({"tool.select", "Select", "Tool", "Q", false, false, false},
                 [](auto& c) { return SessionTool(ShipyardTransformTool::Select, c); });
    out.Register({"tool.move", "Move", "Tool", "W", false, false, false},
                 [](auto& c) { return SessionTool(ShipyardTransformTool::Move, c); });
    out.Register({"tool.rotate", "Rotate", "Tool", "E", false, false, false},
                 [](auto& c) { return SessionTool(ShipyardTransformTool::Rotate, c); });
    out.Register({"tool.scale", "Scale", "Tool", "R", false, false, false},
                 [](auto& c) { return SessionTool(ShipyardTransformTool::Scale, c); });

    out.Register({"view.toggle-advanced", "Advanced / Dev", "View", "", false, false, false},
                 [](auto& c) {
                     if (!c.session) return ShipyardCommandResult{false, false, "No Shipyard session"};
                     c.session->advancedVisible = !c.session->advancedVisible;
                     return ShipyardCommandResult{true, false,
                         c.session->advancedVisible ? "Advanced Shipyard tools visible" : "Advanced Shipyard tools hidden"};
                 });

    auto runtime=[](const char* id,const char* unavailable){
        return [id,unavailable](ShipyardCommandContext& c){
            if(!c.runtimeExecutor)return ShipyardCommandResult{false,false,unavailable};
            return c.runtimeExecutor(id,c.value);
        };
    };
    out.Register({"generator.generate", "Generate", "Generator", "", true, true, false},
                 runtime("generator.generate","Generator runtime is not bound"));
    out.Register({"generator.new-seed-generate", "New Seed + Generate", "Generator", "", true, true, false},
                 runtime("generator.new-seed-generate","Generator runtime is not bound"));
    out.Register({"generator.explain", "Explain Candidate", "Generator", "", false, false, false},
                 runtime("generator.explain","Generator audit runtime is not bound"));
    out.Register({"validation.run", "Validate Ship", "Validation", "", false, false, false},
                 runtime("validation.run","Validation runtime is not bound"));

    out.Register({"search.open", "Universal Shipyard Search", "View", "F3", false, false, false},
                 [](auto& c){if(!c.session)return ShipyardCommandResult{false,false,"No Shipyard session"};c.session->universalSearchOpen=true;return ShipyardCommandResult{true,false,"Universal Shipyard Search"};});
    out.Register({"view.toggle-aux-docks", "Collapse / Restore Auxiliary Docks", "View", "F12", false, false, false},
                 [](auto& c){if(!c.session)return ShipyardCommandResult{false,false,"No Shipyard session"};c.session->auxiliaryDocksCollapsed=!c.session->auxiliaryDocksCollapsed;return ShipyardCommandResult{true,false,c.session->auxiliaryDocksCollapsed?"Auxiliary docks collapsed":"Auxiliary docks restored"};});
    out.Register({"view.maximize-hovered", "Maximize Hovered Panel", "View", "Ctrl+Space", false, false, false},
                 [](auto& c){if(!c.session||c.session->hoveredPanelId.empty())return ShipyardCommandResult{false,false,"No hovered Shipyard panel"};c.session->maximizedPanelId=c.session->maximizedPanelId==c.session->hoveredPanelId?std::string{}:c.session->hoveredPanelId;return ShipyardCommandResult{true,false,c.session->maximizedPanelId.empty()?"Panel restored":"Panel maximized: "+c.session->maximizedPanelId};});
    out.Register({"layout.reset-current", "Reset Current Workspace Layout", "View", "", false, false, false},
                 [](auto& c){if(!c.session)return ShipyardCommandResult{false,false,"No Shipyard session"};c.session->maximizedPanelId.clear();c.session->auxiliaryDocksCollapsed=false;return ShipyardCommandResult{true,false,"Current workspace layout reset requested"};});
    out.Register({"panel.asset-browser", "Asset Browser", "Panel", "", false, false, false}, runtime("panel.asset-browser","Panel runtime is not bound"));
    out.Register({"panel.generator", "Generator", "Panel", "", false, false, false}, runtime("panel.generator","Panel runtime is not bound"));
    out.Register({"panel.outliner", "Outliner", "Panel", "", false, false, false}, runtime("panel.outliner","Panel runtime is not bound"));
    out.Register({"panel.properties", "Properties", "Panel", "", false, false, false}, runtime("panel.properties","Panel runtime is not bound"));
    out.Register({"selection.open-definition", "Open Selected Definition", "Selection", "", false, false, true}, runtime("selection.open-definition","Definition editor runtime is not bound"));

    out.Register({"edit.undo", "Undo", "Edit", "Ctrl+Z", true, false, false},
                 [](auto& c) {
                     if (!c.document || !c.history) return ShipyardCommandResult{false, false, "Undo unavailable"};
                     std::string label;
                     const bool ok = c.history->Undo(*c.document, &label);
                     return ShipyardCommandResult{ok, ok, ok ? "Undo: " + label : "Nothing to undo"};
                 });
    out.Register({"edit.redo", "Redo", "Edit", "Ctrl+Shift+Z", true, false, false},
                 [](auto& c) {
                     if (!c.document || !c.history) return ShipyardCommandResult{false, false, "Redo unavailable"};
                     std::string label;
                     const bool ok = c.history->Redo(*c.document, &label);
                     return ShipyardCommandResult{ok, ok, ok ? "Redo: " + label : "Nothing to redo"};
                 });

    // Advanced workspace entries remain searchable but do not compete with the
    // five primary workflow tabs.
    const struct AdvancedWorkspace { const char* id; const char* label; ShipyardWorkspaceMode mode; } advanced[] = {
        {"workspace.model", "Model", ShipyardWorkspaceMode::Model},
        {"workspace.character", "Character", ShipyardWorkspaceMode::Character},
        {"workspace.pcg", "PCG Lab", ShipyardWorkspaceMode::Pcg},
        {"workspace.world", "World", ShipyardWorkspaceMode::World},
        {"workspace.devworld", "Dev World", ShipyardWorkspaceMode::DevWorld},
        {"workspace.project", "Project Tools", ShipyardWorkspaceMode::ProjectTools},
        {"workspace.authoring", "Raw Authoring", ShipyardWorkspaceMode::Authoring}
    };
    for (const auto& item : advanced) {
        out.Register({item.id, item.label, "Advanced Workspace", "", false, false, true},
                     [mode = item.mode](auto& c) { return SessionWorkspace(mode, c); });
    }
    return out;
}

} // namespace subspace
