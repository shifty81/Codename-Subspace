#include "fleet/FleetMissionSystem.h"

#include <algorithm>

namespace subspace {

std::uint64_t FleetMissionSystemNative::Create(FleetMissionType type, double durationSeconds, double risk, double rewardCredits) {
    FleetMissionNative m;
    m.id = nextId_++;
    m.type = type;
    m.durationSeconds = std::max(0.1, durationSeconds);
    m.risk = std::clamp(risk, 0.0, 1.0);
    m.rewardCredits = std::max(0.0, rewardCredits);
    missions_[m.id] = m;
    return m.id;
}

bool FleetMissionSystemNative::AssignShip(std::uint64_t missionId, std::uint64_t shipId) {
    auto it = missions_.find(missionId);
    if (it == missions_.end() || it->second.state != FleetMissionState::Planned || shipId == 0) return false;
    auto& ships = it->second.shipIds;
    if (std::find(ships.begin(), ships.end(), shipId) == ships.end()) ships.push_back(shipId);
    return true;
}

bool FleetMissionSystemNative::Start(std::uint64_t missionId, double readiness) {
    auto it = missions_.find(missionId);
    if (it == missions_.end() || it->second.state != FleetMissionState::Planned || it->second.shipIds.empty()) return false;
    it->second.readiness = std::clamp(readiness, 0.0, 1.0);
    it->second.state = FleetMissionState::Active;
    return true;
}

bool FleetMissionSystemNative::Cancel(std::uint64_t missionId) {
    auto it = missions_.find(missionId);
    if (it == missions_.end() || it->second.state == FleetMissionState::Completed || it->second.state == FleetMissionState::Failed) return false;
    it->second.state = FleetMissionState::Cancelled;
    return true;
}

void FleetMissionSystemNative::Update(double deltaSeconds) {
    if (deltaSeconds <= 0.0) return;
    for (auto& pair : missions_) {
        auto& m = pair.second;
        if (m.state != FleetMissionState::Active) continue;
        m.elapsedSeconds += deltaSeconds * std::max(0.1, m.readiness);
        if (m.elapsedSeconds >= m.durationSeconds) {
            const double successScore = m.readiness + 0.08 * static_cast<double>(m.shipIds.size()) - m.risk;
            m.state = successScore >= 0.25 ? FleetMissionState::Completed : FleetMissionState::Failed;
            m.elapsedSeconds = m.durationSeconds;
        }
    }
}

const FleetMissionNative* FleetMissionSystemNative::Get(std::uint64_t missionId) const {
    auto it = missions_.find(missionId); return it == missions_.end() ? nullptr : &it->second;
}

std::vector<FleetMissionNative> FleetMissionSystemNative::GetActive() const {
    std::vector<FleetMissionNative> out;
    for (const auto& pair : missions_) if (pair.second.state == FleetMissionState::Active) out.push_back(pair.second);
    return out;
}

} // namespace subspace
