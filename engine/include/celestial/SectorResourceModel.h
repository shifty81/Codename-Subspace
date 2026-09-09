#pragma once

#include "celestial/CelestialTypes.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

struct SectorResourceEntry {
    std::string tag;
    int abundance = 0;
    int sourceBodies = 0;
    bool hazardousSource = false;
};

struct SectorHazardEntry {
    std::string tag;
    int severity = 0;
    std::string source;
};

struct SectorStationEconomyHint {
    std::string commodity;
    int demand = 0;
    int supply = 0;
    float priceMultiplier = 1.0f;
    std::string reason;
};

struct AsteroidFieldProfile {
    std::string id;
    std::vector<std::string> resourceTags;
    int resourceRichness = 1;
    float miningYieldMultiplier = 1.0f;
    float salvageChance = 0.08f;
    float pirateRisk = 0.05f;
    float hazardRating = 0.0f;
    std::string dominantResource = "ore";
};

struct SectorResourceSurvey {
    std::string sectorId;
    std::uint32_t seed = 0;
    std::vector<SectorResourceEntry> resources;
    std::vector<SectorHazardEntry> hazards;
    std::vector<SectorStationEconomyHint> economyHints;
    AsteroidFieldProfile asteroidField;
};

SectorResourceSurvey BuildSectorResourceSurvey(const StarSystemDefinition& system);
std::string PickPrimaryResourceTag(const SectorResourceSurvey& survey, std::uint32_t seed, int index = 0);
std::string SectorResourceSummary(const SectorResourceSurvey& survey);
std::string AsteroidFieldSummary(const AsteroidFieldProfile& field);
std::vector<std::string> TopSectorResourceTags(const SectorResourceSurvey& survey, std::size_t maxTags = 4);

} // namespace subspace
