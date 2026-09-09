#include "developer/events/DeveloperEventFeed.h"

namespace subspace {

DeveloperEventFeed::DeveloperEventFeed(std::size_t maxEvents)
    : _maxEvents(maxEvents == 0 ? 1 : maxEvents)
{
}

void DeveloperEventFeed::Push(DeveloperEvent event)
{
    _events.push_back(std::move(event));
    while (_events.size() > _maxEvents) {
        _events.pop_front();
    }
}

std::vector<DeveloperEvent> DeveloperEventFeed::GetEvents() const
{
    return {_events.begin(), _events.end()};
}

std::vector<DeveloperEvent> DeveloperEventFeed::GetLatest(std::size_t count) const
{
    if (count >= _events.size()) {
        return GetEvents();
    }
    return {_events.end() - static_cast<std::ptrdiff_t>(count), _events.end()};
}

void DeveloperEventFeed::Clear()
{
    _events.clear();
}

const char* ToString(DeveloperEventKind kind)
{
    switch (kind) {
        case DeveloperEventKind::Info: return "info";
        case DeveloperEventKind::Command: return "command";
        case DeveloperEventKind::AssetReload: return "asset_reload";
        case DeveloperEventKind::EntityEdit: return "entity_edit";
        case DeveloperEventKind::ShipEdit: return "ship_edit";
        case DeveloperEventKind::Validation: return "validation";
        case DeveloperEventKind::Warning: return "warning";
        case DeveloperEventKind::Error: return "error";
    }
    return "unknown";
}

} // namespace subspace
