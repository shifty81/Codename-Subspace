#pragma once

#include "ship_editor/ShipyardDocumentSystem.h"
#include "ship_editor/ShipyardHistorySystem.h"
#include "ship_editor/ShipyardSessionSystem.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

struct ShipyardCommandDescriptor {
    std::string id;
    std::string label;
    std::string category;
    std::string defaultShortcut;
    bool mutatesDocument = false;
    bool undoable = false;
    bool advanced = false;
};

struct ShipyardCommandResult {
    bool handled = false;
    bool changed = false;
    std::string message;
};

using ShipyardRuntimeCommandExecutor = std::function<ShipyardCommandResult(std::string_view, int)>;

struct ShipyardCommandContext {
    ShipyardDocument* document = nullptr;
    ShipyardSession* session = nullptr;
    ShipyardHistorySystem* history = nullptr;
    ShipyardRuntimeCommandExecutor runtimeExecutor{};
    int value = 0;
    std::string text;
};

using ShipyardCommandHandler = std::function<ShipyardCommandResult(ShipyardCommandContext&)>;

class ShipyardCommandSystem {
public:
    bool Register(ShipyardCommandDescriptor descriptor, ShipyardCommandHandler handler);
    const ShipyardCommandDescriptor* Find(std::string_view id) const;
    std::vector<ShipyardCommandDescriptor> Search(std::string_view query, bool includeAdvanced) const;
    ShipyardCommandResult Execute(std::string_view id, ShipyardCommandContext& context) const;
    std::vector<ShipyardCommandDescriptor> All(bool includeAdvanced) const;

    static ShipyardCommandSystem BuildProfessionalDefaults();

private:
    struct Record {
        ShipyardCommandDescriptor descriptor;
        ShipyardCommandHandler handler;
    };
    std::unordered_map<std::string, Record> commands_;
};

} // namespace subspace
