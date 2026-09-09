#include "navigation/SystemNavigationSystem.h"

#include <algorithm>

namespace subspace {
bool SystemNavigationSystem::RegisterDestination(const SystemDestination& d){ if(d.id==0||d.name.empty())return false; destinations_[d.id]=d; return true; }
bool SystemNavigationSystem::AddBookmark(std::uint64_t id){ if(!destinations_.count(id))return false; if(std::find(bookmarks_.begin(),bookmarks_.end(),id)==bookmarks_.end())bookmarks_.push_back(id); return true; }
std::vector<SystemDestination> SystemNavigationSystem::GetBookmarks() const { std::vector<SystemDestination> out; for(auto id:bookmarks_){auto it=destinations_.find(id);if(it!=destinations_.end())out.push_back(it->second);}return out; }
std::vector<SystemDestination> SystemNavigationSystem::GetDiscovered() const { std::vector<SystemDestination> out; for(const auto& kv:destinations_)if(kv.second.discovered)out.push_back(kv.second); return out; }
const SystemDestination* SystemNavigationSystem::GetDestination(std::uint64_t id) const { auto it=destinations_.find(id); return it==destinations_.end()?nullptr:&it->second; }
WarpPlan SystemNavigationSystem::PlanWarp(const AstronomicalPosition& from, std::uint64_t id, double speed, double fuel) const {
    WarpPlan p;
    auto it = destinations_.find(id);
    if (it == destinations_.end()) { p.reason = "unknown destination"; return p; }
    const auto& d = it->second;
    if (!d.discovered) { p.reason = "destination not discovered"; return p; }
    if (!d.warpable) { p.reason = "destination cannot accept warp"; return p; }
    if (speed <= 0.0) { p.reason = "drive unavailable"; return p; }

    p.destinationId = id;
    p.distanceMeters = scale_.DistanceMeters(from, d.position);
    p.topSpeedMetersPerSecond = speed;

    // Pass515R2: Vector travel is no longer normalized into a fixed 3.5-10 s
    // gameplay window. Charge remains a short anticipation beat, while actual
    // slipstream time is distance / this ship's achievable Vector top speed.
    // Only a small presentation floor remains so a very short hop still has a
    // readable tunnel transition. There is deliberately no artificial upper cap.
    p.chargeSeconds = std::clamp(0.85 + p.distanceMeters / 2.4e11, 0.85, 2.4);
    p.cruiseSeconds = std::max(1.5, p.distanceMeters / speed);
    p.fuelCost = std::max(1.0, p.distanceMeters / 1e9);

    if (fuel < p.fuelCost) { p.reason = "insufficient vector fuel"; return p; }
    p.valid = true;
    return p;
}
} // namespace subspace
