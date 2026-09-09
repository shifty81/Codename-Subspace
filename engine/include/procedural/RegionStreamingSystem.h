#pragma once

#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

namespace subspace {

enum class SpaceRegionKind { Orbital, AsteroidBelt, PlanetaryRing, SalvageField, StationSpace, DeepSpace };

struct RegionCellKey {
    std::int64_t x = 0;
    std::int64_t y = 0;
    bool operator==(const RegionCellKey& other) const { return x == other.x && y == other.y; }
};

struct RegionAsteroidNode {
    std::uint64_t id = 0;
    double x = 0.0;
    double y = 0.0;
    double richness = 1.0;
    std::string resource = "iron_ore";
};

struct StreamedRegionCell {
    RegionCellKey key;
    SpaceRegionKind kind = SpaceRegionKind::DeepSpace;
    std::vector<RegionAsteroidNode> asteroids;
    bool surveyed = false;
};

struct BeltMacroRegion {
    std::uint64_t seed = 0;
    std::string id;
    SpaceRegionKind kind = SpaceRegionKind::AsteroidBelt;
    std::int64_t conceptualCellCount = 1000000;
    double cellSizeMeters = 250000.0;
    double conceptualLengthMeters = 3.0e12;
    int localAsteroidBudget = 36;
    double densityVariation = 0.55;
};

class RegionStreamingSystem {
public:
    StreamedRegionCell Materialize(const BeltMacroRegion& belt, RegionCellKey key) const;
    std::vector<RegionCellKey> StreamWindow(RegionCellKey center, int radius) const;
    void MarkDepleted(std::uint64_t nodeId);
    bool IsDepleted(std::uint64_t nodeId) const;
    void MarkSurveyed(const std::string& beltId, RegionCellKey key);
    bool IsSurveyed(const std::string& beltId, RegionCellKey key) const;
    RegionCellKey FindUnsurveyedWarpCell(const BeltMacroRegion& belt, RegionCellKey near, std::uint64_t salt = 0) const;

private:
    static std::string CellToken(const std::string& beltId, RegionCellKey key);
    std::unordered_set<std::uint64_t> depleted_;
    std::unordered_set<std::string> surveyed_;
};

} // namespace subspace
