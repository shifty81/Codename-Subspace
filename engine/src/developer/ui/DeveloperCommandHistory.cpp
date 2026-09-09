#include "developer/ui/DeveloperCommandHistory.h"

#include <utility>

namespace subspace {

DeveloperCommandHistory::DeveloperCommandHistory(std::size_t capacity)
    : _capacity(capacity == 0 ? 1 : capacity)
{
}

void DeveloperCommandHistory::Push(std::string commandLine)
{
    if (commandLine.empty()) {
        return;
    }
    if (!_entries.empty() && _entries.back() == commandLine) {
        _cursor = _entries.size();
        return;
    }
    _entries.push_back(std::move(commandLine));
    while (_entries.size() > _capacity) {
        _entries.erase(_entries.begin());
    }
    _cursor = _entries.size();
}

void DeveloperCommandHistory::Clear()
{
    _entries.clear();
    _cursor = 0;
}

std::string DeveloperCommandHistory::Previous()
{
    if (_entries.empty()) {
        return {};
    }
    if (_cursor > 0) {
        --_cursor;
    }
    return _entries[_cursor];
}

std::string DeveloperCommandHistory::Next()
{
    if (_entries.empty()) {
        return {};
    }
    if (_cursor + 1 < _entries.size()) {
        ++_cursor;
        return _entries[_cursor];
    }
    _cursor = _entries.size();
    return {};
}

void DeveloperCommandHistory::ResetNavigation()
{
    _cursor = _entries.size();
}

} // namespace subspace
