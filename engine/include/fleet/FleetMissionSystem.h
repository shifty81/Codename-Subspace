#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class FleetMissionType { Patrol, Escort, Mine, Salvage, Haul, Explore, Defend };
enum class FleetMissionState { Planned, Active, Completed, Failed, Cancelled };

struct FleetMissionNative {
    std::uint64_t id = 0;
    FleetMissionType type = FleetMissionType::Patrol;
    FleetMissionState state = FleetMissionState::Planned;
    std::vector<std::uint64_t> shipIds;
    double durationSeconds = 60.0;
    double elapsedSeconds = 0.0;
    double risk = 0.25;
    double readiness = 1.0;
    double rewardCredits = 0.0;
};

class FleetMissionSystemNative {
public:
    std::uint64_t Create(FleetMissionType type, double durationSeconds, double risk, double rewardCredits);
    bool AssignShip(std::uint64_t missionId, std::uint64_t shipId);
    bool Start(std::uint64_t missionId, double readiness = 1.0);
    bool Cancel(std::uint64_t missionId);
    void Update(double deltaSeconds);
    const FleetMissionNative* Get(std::uint64_t missionId) const;
    std::vector<FleetMissionNative> GetActive() const;

private:
    std::uint64_t nextId_ = 1;
    std::unordered_map<std::uint64_t, FleetMissionNative> missions_;
};

} // namespace subspace
