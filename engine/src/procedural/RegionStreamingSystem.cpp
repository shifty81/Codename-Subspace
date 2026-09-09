#include "procedural/RegionStreamingSystem.h"

#include <algorithm>
#include <random>

namespace subspace {
namespace {
std::uint64_t Mix(std::uint64_t seed, std::int64_t x, std::int64_t y) {
    seed ^= static_cast<std::uint64_t>(x) * 0x9E3779B185EBCA87ULL;
    seed ^= static_cast<std::uint64_t>(y) * 0xC2B2AE3D27D4EB4FULL;
    seed ^= seed >> 29; seed *= 0x165667B19E3779F9ULL; seed ^= seed >> 32;
    return seed;
}
}

StreamedRegionCell RegionStreamingSystem::Materialize(const BeltMacroRegion& belt, RegionCellKey key) const {
    StreamedRegionCell cell; cell.key = key; cell.kind = belt.kind; cell.surveyed = IsSurveyed(belt.id, key);
    const auto mixed = Mix(belt.seed, key.x, key.y);
    std::mt19937_64 rng(mixed);
    std::uniform_real_distribution<double> p(-belt.cellSizeMeters * 0.5, belt.cellSizeMeters * 0.5);
    std::uniform_real_distribution<double> rich(0.55, 2.4);
    const int baseBudget = std::max(8, belt.localAsteroidBudget);
    const int count = belt.kind == SpaceRegionKind::PlanetaryRing ? std::max(64, baseBudget + 24) : baseBudget;
    static const char* resources[] = {"iron_ore","nickel_ore","titanium_ore","silicates","ice"};
    for (int i=0;i<count;++i) {
        const std::uint64_t id = Mix(mixed, i + 1, i * 31 + 7);
        if (IsDepleted(id)) continue;
        cell.asteroids.push_back({id,p(rng),p(rng),rich(rng),resources[(mixed + static_cast<std::uint64_t>(i)) % 5]});
    }
    return cell;
}

std::vector<RegionCellKey> RegionStreamingSystem::StreamWindow(RegionCellKey center, int radius) const {
    radius = std::max(0, radius); std::vector<RegionCellKey> out;
    for (int y=-radius;y<=radius;++y) for (int x=-radius;x<=radius;++x) out.push_back({center.x+x,center.y+y});
    return out;
}
void RegionStreamingSystem::MarkDepleted(std::uint64_t nodeId) { depleted_.insert(nodeId); }
bool RegionStreamingSystem::IsDepleted(std::uint64_t nodeId) const { return depleted_.count(nodeId) != 0; }
std::string RegionStreamingSystem::CellToken(const std::string& beltId, RegionCellKey key) { return beltId+":"+std::to_string(key.x)+":"+std::to_string(key.y); }
void RegionStreamingSystem::MarkSurveyed(const std::string& beltId, RegionCellKey key) { surveyed_.insert(CellToken(beltId,key)); }
bool RegionStreamingSystem::IsSurveyed(const std::string& beltId, RegionCellKey key) const { return surveyed_.count(CellToken(beltId,key)) != 0; }

RegionCellKey RegionStreamingSystem::FindUnsurveyedWarpCell(const BeltMacroRegion& belt,RegionCellKey near,std::uint64_t salt) const {
    const std::int64_t span=std::max<std::int64_t>(64,static_cast<std::int64_t>(std::sqrt(static_cast<double>(std::max<std::int64_t>(1,belt.conceptualCellCount)))));
    for(std::uint64_t attempt=0;attempt<4096;++attempt){
        const auto mixed=Mix(belt.seed^salt,near.x+static_cast<std::int64_t>(attempt),near.y-static_cast<std::int64_t>(attempt*3));
        const std::int64_t dx=static_cast<std::int64_t>(mixed%static_cast<std::uint64_t>(span*2+1))-span;
        const std::int64_t dy=static_cast<std::int64_t>((mixed>>23)%static_cast<std::uint64_t>(span*2+1))-span;
        RegionCellKey key{near.x+dx,near.y+dy};
        if(!IsSurveyed(belt.id,key))return key;
    }
    return {near.x+span,near.y+span};
}

} // namespace subspace
