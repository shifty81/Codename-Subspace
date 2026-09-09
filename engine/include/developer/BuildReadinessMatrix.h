#pragma once

#include <string>
#include <vector>

namespace subspace {

struct BuildReadinessItem {
    std::string area;
    std::string status;
    std::string action;
};

class BuildReadinessMatrix {
public:
    void Add(BuildReadinessItem item);
    bool HasBlockingItems() const;
    std::vector<BuildReadinessItem> Items() const { return _items; }
    std::string Summary() const;

private:
    std::vector<BuildReadinessItem> _items;
};

} // namespace subspace
