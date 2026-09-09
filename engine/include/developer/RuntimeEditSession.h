#pragma once

#include "developer/RuntimeEditCommand.h"

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace subspace {

class RuntimeEditSession {
public:
    using ApplyFunction = std::function<RuntimeEditResult(const RuntimeEditCommand&)>;

    void Begin(std::string label = "Development Play Session");
    void End();
    bool IsActive() const { return _active; }

    void ClearHistory();
    void RecordApplied(const RuntimeEditResult& result);

    bool CanUndo() const;
    bool CanRedo() const;
    RuntimeEditResult Undo(const ApplyFunction& apply);
    RuntimeEditResult Redo(const ApplyFunction& apply);

    std::size_t GetUndoCount() const { return _undoStack.size(); }
    std::size_t GetRedoCount() const { return _redoStack.size(); }
    std::size_t GetDirtyEditCount() const { return _dirtyEditCount; }
    std::uint64_t NextSequence();

    const std::vector<RuntimeEditResult>& GetAppliedHistory() const { return _appliedHistory; }
    const std::string& GetLabel() const { return _label; }

    static RuntimeEditCommand BuildUndoCommand(const RuntimeEditCommand& command);

private:
    bool _active = false;
    std::string _label;
    std::uint64_t _nextSequence = 1;
    std::size_t _dirtyEditCount = 0;
    std::vector<RuntimeEditResult> _appliedHistory;
    std::vector<RuntimeEditCommand> _undoStack;
    std::vector<RuntimeEditCommand> _redoStack;
};

} // namespace subspace
