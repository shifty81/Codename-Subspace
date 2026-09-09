#pragma once

#include "celestial/SectorResourceModel.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class EncounterDisposition {
    Friendly,
    Neutral,
    Hostile,
    Pirate,
    Hidden
};

struct FactionPresence {
    std::string factionId;
    std::string displayName;
    EncounterDisposition disposition = EncounterDisposition::Neutral;
    int strength = 1;
    int reputation = 0;
};

struct EncounterArchetype {
    std::string id;
    std::string displayName;
    EncounterDisposition disposition = EncounterDisposition::Neutral;
    int weight = 1;
    int threat = 1;
    std::vector<std::string> tags;
};

struct EncounterSpawnTable {
    std::string sectorId;
    std::vector<FactionPresence> factions;
    std::vector<EncounterArchetype> encounters;
};

EncounterSpawnTable BuildEncounterSpawnTable(const SectorResourceSurvey& survey, std::uint32_t seed);
std::string EncounterDispositionName(EncounterDisposition disposition);
std::string EncounterSpawnTableSummary(const EncounterSpawnTable& table);

} // namespace subspace
