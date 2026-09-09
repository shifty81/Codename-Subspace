#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class RuntimeSelectionKind {
    None,
    Entity,
    Ship,
    Asset,
    WorldObject,
    Component,
};

struct RuntimeSelectionItem {
    RuntimeSelectionKind kind = RuntimeSelectionKind::None;
    std::string id;
    std::string label;
    std::uint64_t entityId = 0;
};

class RuntimeSelectionService {
public:
    void Clear();
    void SetPrimary(RuntimeSelectionItem item);
    void Add(RuntimeSelectionItem item);
    bool Remove(const std::string& id);

    bool HasSelection() const { return !_items.empty(); }
    const RuntimeSelectionItem* GetPrimary() const;
    std::vector<RuntimeSelectionItem> GetItems() const { return _items; }
    std::string DescribePrimary() const;

private:
    std::vector<RuntimeSelectionItem> _items;
};

const char* ToString(RuntimeSelectionKind kind);

} // namespace subspace
