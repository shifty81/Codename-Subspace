#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

struct RuntimeRebuildRequest {
    std::string reason;
    std::string target;
    std::uint64_t sequence = 0;
};

class RuntimeWorld {
public:
    void RequestRebuild(std::string reason, std::string target = {});
    bool HasPendingRebuilds() const { return !_pendingRebuilds.empty(); }
    std::vector<RuntimeRebuildRequest> ConsumePendingRebuilds();
    const std::vector<RuntimeRebuildRequest>& PeekPendingRebuilds() const { return _pendingRebuilds; }

private:
    std::uint64_t _nextRebuildSequence = 1;
    std::vector<RuntimeRebuildRequest> _pendingRebuilds;
};

} // namespace subspace
