#pragma once

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace subspace {

enum class ShipyardJobStatus{Queued,Running,Succeeded,Failed,Cancelled};

struct ShipyardJobDescriptor{
    std::string id;
    std::string label;
    std::string category;
    bool cancellable=true;
};
struct ShipyardJobRecord{
    std::uint64_t sequence=0;
    ShipyardJobDescriptor descriptor{};
    ShipyardJobStatus status=ShipyardJobStatus::Queued;
    int exitCode=0;
    float progress=0.0f;
    std::string statusText;
    std::vector<std::string> output;
};

/// Thread-safe Activity/Jobs model. Platform process creation is a separate
/// runner; panels consume this state instead of owning detached process/log state.
class ShipyardJobSystem{
public:
    std::uint64_t Queue(ShipyardJobDescriptor descriptor);
    bool Start(std::uint64_t sequence,std::string status={});
    bool SetProgress(std::uint64_t sequence,float progress,std::string status={});
    bool AppendOutput(std::uint64_t sequence,std::string line);
    bool Complete(std::uint64_t sequence,int exitCode,std::string status={});
    bool Cancel(std::uint64_t sequence,std::string status={});
    std::optional<ShipyardJobRecord> Find(std::uint64_t sequence)const;
    std::vector<ShipyardJobRecord> Snapshot()const;
    std::vector<ShipyardJobRecord> Active()const;
private:
    ShipyardJobRecord* FindMutable(std::uint64_t sequence);
    mutable std::mutex mutex_;
    std::vector<ShipyardJobRecord> jobs_;
    std::uint64_t nextSequence_=1;
};

} // namespace subspace
