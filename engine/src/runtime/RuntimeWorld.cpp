#include "runtime/RuntimeWorld.h"

#include <utility>

namespace subspace {

void RuntimeWorld::RequestRebuild(std::string reason, std::string target)
{
    RuntimeRebuildRequest request;
    request.reason = std::move(reason);
    request.target = std::move(target);
    request.sequence = _nextRebuildSequence++;
    _pendingRebuilds.push_back(std::move(request));
}

std::vector<RuntimeRebuildRequest> RuntimeWorld::ConsumePendingRebuilds()
{
    std::vector<RuntimeRebuildRequest> consumed = std::move(_pendingRebuilds);
    _pendingRebuilds.clear();
    return consumed;
}

} // namespace subspace
