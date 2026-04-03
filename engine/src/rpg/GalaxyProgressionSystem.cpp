#include "rpg/GalaxyProgressionSystem.h"

#include <cmath>

namespace subspace {

const char* MaterialTierName(MaterialTier tier) {
    switch (tier) {
        case MaterialTier::Iron:     return "Iron";
        case MaterialTier::Titanium: return "Titanium";
        case MaterialTier::Naonite:  return "Naonite";
        case MaterialTier::Trinium:  return "Trinium";
        case MaterialTier::Xanion:   return "Xanion";
        case MaterialTier::Ogonite:  return "Ogonite";
        case MaterialTier::Avorion:  return "Avorion";
        default:                     return "Unknown";
    }
}

// ---------------------------------------------------------------------------
// PlayerProgressionComponent
// ---------------------------------------------------------------------------

ComponentData PlayerProgressionComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "PlayerProgressionComponent";
    cd.data["sx"] = std::to_string(currentSector.x);
    cd.data["sy"] = std::to_string(currentSector.y);
    cd.data["sz"] = std::to_string(currentSector.z);
    cd.data["dist"] = std::to_string(distanceFromCenter);
    cd.data["tier"] = std::to_string(static_cast<int>(currentZoneTier));
    cd.data["maxTier"] = std::to_string(static_cast<int>(highestTierReached));
    cd.data["diff"] = std::to_string(difficultyMult);
    cd.data["loot"] = std::to_string(lootQualityMult);
    return cd;
}

void PlayerProgressionComponent::Deserialize(const ComponentData& data) {
    auto f = [&](const std::string& k, const std::string& def = "0") {
        auto it = data.data.find(k);
        return (it != data.data.end()) ? it->second : def;
    };
    currentSector.x = std::stoi(f("sx"));
    currentSector.y = std::stoi(f("sy"));
    currentSector.z = std::stoi(f("sz"));
    distanceFromCenter = std::stoi(f("dist"));
    currentZoneTier    = static_cast<MaterialTier>(std::stoi(f("tier")));
    highestTierReached = static_cast<MaterialTier>(std::stoi(f("maxTier")));
    difficultyMult     = std::stof(f("diff", "1.0"));
    lootQualityMult    = std::stof(f("loot", "1.0"));
}

// ---------------------------------------------------------------------------
// GalaxyProgressionSystem
// ---------------------------------------------------------------------------

GalaxyProgressionSystem::GalaxyProgressionSystem()
    : SystemBase("GalaxyProgressionSystem") {}
GalaxyProgressionSystem::GalaxyProgressionSystem(EntityManager& em)
    : SystemBase("GalaxyProgressionSystem"), _em(&em) {}

void GalaxyProgressionSystem::SetEntityManager(EntityManager* em) { _em = em; }

void GalaxyProgressionSystem::Update(float /*deltaTime*/) {
    if (!_em) return;
    auto players = _em->GetAllComponents<PlayerProgressionComponent>();
    for (auto* p : players) {
        if (p) UpdatePlayerProgression(*p);
    }
}

int GalaxyProgressionSystem::DistanceFromCenter(const SectorCoordinate& s) {
    // Chebyshev distance
    int dx = std::abs(s.x);
    int dy = std::abs(s.y);
    int dz = std::abs(s.z);
    return std::max(dx, std::max(dy, dz));
}

MaterialTier GalaxyProgressionSystem::AvailableTier(int dist) {
    if (dist <  25) return MaterialTier::Avorion;
    if (dist <  50) return MaterialTier::Ogonite;
    if (dist <  75) return MaterialTier::Xanion;
    if (dist < 150) return MaterialTier::Trinium;
    if (dist < 250) return MaterialTier::Naonite;
    if (dist < 350) return MaterialTier::Titanium;
    return MaterialTier::Iron;
}

float GalaxyProgressionSystem::DifficultyMultiplier(int dist) {
    if (dist <  25) return 10.0f;
    if (dist <  50) return  6.0f;
    if (dist <  75) return  4.0f;
    if (dist < 150) return  2.5f;
    if (dist < 250) return  1.8f;
    if (dist < 350) return  1.3f;
    return 1.0f;
}

float GalaxyProgressionSystem::LootQualityMultiplier(int dist) {
    if (dist <  25) return 5.0f;
    if (dist <  50) return 4.0f;
    if (dist <  75) return 3.0f;
    if (dist < 150) return 2.0f;
    if (dist < 250) return 1.5f;
    if (dist < 350) return 1.2f;
    return 1.0f;
}

void GalaxyProgressionSystem::UpdatePlayerProgression(
        PlayerProgressionComponent& comp) const {
    int dist = DistanceFromCenter(comp.currentSector);
    comp.distanceFromCenter = dist;
    comp.currentZoneTier    = AvailableTier(dist);
    comp.difficultyMult     = DifficultyMultiplier(dist);
    comp.lootQualityMult    = LootQualityMultiplier(dist);

    if (static_cast<int>(comp.currentZoneTier) >
        static_cast<int>(comp.highestTierReached))
        comp.highestTierReached = comp.currentZoneTier;
}

} // namespace subspace
