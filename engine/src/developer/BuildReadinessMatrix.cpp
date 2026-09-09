#include "developer/BuildReadinessMatrix.h"

#include <sstream>

namespace subspace {

void BuildReadinessMatrix::Add(BuildReadinessItem item)
{
    _items.push_back(std::move(item));
}

bool BuildReadinessMatrix::HasBlockingItems() const
{
    for (const auto& item : _items) {
        if (item.status == "BLOCKED" || item.status == "FAIL") {
            return true;
        }
    }
    return false;
}

std::string BuildReadinessMatrix::Summary() const
{
    std::size_t blocked = 0;
    for (const auto& item : _items) {
        if (item.status == "BLOCKED" || item.status == "FAIL") {
            ++blocked;
        }
    }
    std::ostringstream out;
    out << _items.size() << " item(s), " << blocked << " blocker(s)";
    return out.str();
}

} // namespace subspace
