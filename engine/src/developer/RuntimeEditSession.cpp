#include "developer/RuntimeEditSession.h"

#include <utility>

namespace subspace {

void RuntimeEditSession::Begin(std::string label)
{
    _active = true;
    _label = std::move(label);
}

void RuntimeEditSession::End()
{
    _active = false;
}

void RuntimeEditSession::ClearHistory()
{
    _appliedHistory.clear();
    _undoStack.clear();
    _redoStack.clear();
    _dirtyEditCount = 0;
}

void RuntimeEditSession::RecordApplied(const RuntimeEditResult& result)
{
    if (!result.handled || !result.success) {
        return;
    }

    _appliedHistory.push_back(result);
    if (result.undoable) {
        _undoStack.push_back(result.command);
        _redoStack.clear();
        ++_dirtyEditCount;
    }
}

bool RuntimeEditSession::CanUndo() const
{
    return !_undoStack.empty();
}

bool RuntimeEditSession::CanRedo() const
{
    return !_redoStack.empty();
}

RuntimeEditResult RuntimeEditSession::Undo(const ApplyFunction& apply)
{
    if (!CanUndo()) {
        RuntimeEditCommand command;
        command.name = "dev.undo";
        return RuntimeEditResult::Failure(command, "Nothing to undo.");
    }

    RuntimeEditCommand original = _undoStack.back();
    _undoStack.pop_back();

    RuntimeEditCommand undo = BuildUndoCommand(original);
    undo.sequence = NextSequence();
    RuntimeEditResult result = apply(undo);

    if (result.success) {
        _redoStack.push_back(original);
        if (_dirtyEditCount > 0) {
            --_dirtyEditCount;
        }
    } else {
        _undoStack.push_back(original);
    }

    return result;
}

RuntimeEditResult RuntimeEditSession::Redo(const ApplyFunction& apply)
{
    if (!CanRedo()) {
        RuntimeEditCommand command;
        command.name = "dev.redo";
        return RuntimeEditResult::Failure(command, "Nothing to redo.");
    }

    RuntimeEditCommand command = _redoStack.back();
    _redoStack.pop_back();
    command.sequence = NextSequence();
    RuntimeEditResult result = apply(command);

    if (result.success) {
        _undoStack.push_back(command);
        ++_dirtyEditCount;
    } else {
        _redoStack.push_back(command);
    }

    return result;
}

std::uint64_t RuntimeEditSession::NextSequence()
{
    return _nextSequence++;
}

RuntimeEditCommand RuntimeEditSession::BuildUndoCommand(const RuntimeEditCommand& command)
{
    RuntimeEditCommand undo = command;
    undo.source = "runtime-edit-undo";
    undo.description = "Undo of " + command.name;

    if (command.name == "ship.block.place") {
        undo.name = "ship.block.remove";
    } else if (command.name == "ship.block.remove") {
        undo.name = "ship.block.place";
    } else if (command.name == "entity.spawn") {
        undo.name = "entity.delete";
    } else if (command.name == "entity.delete") {
        undo.name = "entity.spawn";
    } else if (command.name == "entity.component.add") {
        undo.name = "entity.component.remove";
    } else if (command.name == "entity.component.remove") {
        undo.name = "entity.component.add";
    } else if (command.name == "entity.activate") {
        undo.name = "entity.deactivate";
    } else if (command.name == "entity.deactivate") {
        undo.name = "entity.activate";
    }

    if ((command.name == "ship.block.paint" || command.name == "entity.component.set") &&
        undo.HasArg("before") && undo.HasArg("after")) {
        const std::string before = undo.GetArg("before");
        undo.SetArg("before", undo.GetArg("after"));
        undo.SetArg("after", before);
        undo.SetArg("value", before);
    }

    return undo;
}

} // namespace subspace
