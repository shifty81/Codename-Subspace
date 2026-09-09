#include "developer/assets/RuntimeHotReloadQueue.h"

namespace subspace {

std::uint64_t RuntimeHotReloadQueue::Enqueue(AssetReloadRequest request, std::string reason) {
    RuntimeHotReloadQueuedItem item;
    item.request = std::move(request);
    item.reason = std::move(reason);
    item.sequence = _nextSequence++;
    _items.push_back(item);
    return item.sequence;
}

RuntimeHotReloadQueuedItem RuntimeHotReloadQueue::Pop() {
    if (_items.empty()) {
        return {};
    }
    RuntimeHotReloadQueuedItem item = _items.front();
    _items.pop_front();
    return item;
}

std::vector<RuntimeHotReloadQueuedItem> RuntimeHotReloadQueue::Snapshot() const {
    return std::vector<RuntimeHotReloadQueuedItem>(_items.begin(), _items.end());
}

void RuntimeHotReloadQueue::Clear() {
    _items.clear();
}

} // namespace subspace
