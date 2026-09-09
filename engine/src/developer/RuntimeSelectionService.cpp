#include "developer/selection/RuntimeSelectionService.h"

#include <algorithm>

namespace subspace {

void RuntimeSelectionService::Clear()
{
    _items.clear();
}

void RuntimeSelectionService::SetPrimary(RuntimeSelectionItem item)
{
    _items.clear();
    if (!item.id.empty() || item.entityId != 0) {
        _items.push_back(std::move(item));
    }
}

void RuntimeSelectionService::Add(RuntimeSelectionItem item)
{
    if (item.id.empty() && item.entityId == 0) {
        return;
    }
    _items.push_back(std::move(item));
}

bool RuntimeSelectionService::Remove(const std::string& id)
{
    auto oldSize = _items.size();
    _items.erase(std::remove_if(_items.begin(), _items.end(), [&](const RuntimeSelectionItem& item) {
        return item.id == id;
    }), _items.end());
    return _items.size() != oldSize;
}

const RuntimeSelectionItem* RuntimeSelectionService::GetPrimary() const
{
    return _items.empty() ? nullptr : &_items.front();
}

std::string RuntimeSelectionService::DescribePrimary() const
{
    const auto* item = GetPrimary();
    if (!item) {
        return "none";
    }
    std::string label = item->label.empty() ? item->id : item->label;
    return std::string(ToString(item->kind)) + ":" + label;
}

const char* ToString(RuntimeSelectionKind kind)
{
    switch (kind) {
        case RuntimeSelectionKind::None: return "none";
        case RuntimeSelectionKind::Entity: return "entity";
        case RuntimeSelectionKind::Ship: return "ship";
        case RuntimeSelectionKind::Asset: return "asset";
        case RuntimeSelectionKind::WorldObject: return "world_object";
        case RuntimeSelectionKind::Component: return "component";
    }
    return "unknown";
}

} // namespace subspace
