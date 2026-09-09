#pragma once

#include "developer/assets/AssetReloadPipeline.h"

#include <deque>
#include <string>
#include <vector>

namespace subspace {

struct RuntimeHotReloadQueuedItem {
    AssetReloadRequest request;
    std::string reason;
    std::uint64_t sequence = 0;
};

class RuntimeHotReloadQueue {
public:
    std::uint64_t Enqueue(AssetReloadRequest request, std::string reason = {});
    bool Empty() const { return _items.empty(); }
    std::size_t Size() const { return _items.size(); }
    RuntimeHotReloadQueuedItem Pop();
    std::vector<RuntimeHotReloadQueuedItem> Snapshot() const;
    void Clear();

private:
    std::uint64_t _nextSequence = 1;
    std::deque<RuntimeHotReloadQueuedItem> _items;
};

} // namespace subspace
