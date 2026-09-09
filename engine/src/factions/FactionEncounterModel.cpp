#include "factions/FactionEncounterModel.h"

#include "celestial/CelestialSystemGenerator.h"

#include <algorithm>
#include <sstream>

namespace subspace {

EncounterSpawnTable BuildEncounterSpawnTable(const SectorResourceSurvey& survey, std::uint32_t seed)
{
    EncounterSpawnTable table;
    table.sectorId = survey.sectorId;
    table.factions.push_back({"frontier-guild", "Frontier Guild", EncounterDisposition::Friendly, 3, 20});
    table.factions.push_back({"independent-miners", "Independent Miners", EncounterDisposition::Neutral, 2, 0});

    const bool highPirateRisk = survey.asteroidField.pirateRisk > 0.24f || CelestialSeededUnit(seed, 3) > 0.72f;
    if (highPirateRisk) {
        table.factions.push_back({"red-jackals", "Red Jackal Pirates", EncounterDisposition::Pirate, 2 + survey.asteroidField.resourceRichness / 3, -40});
    }
    if (survey.asteroidField.hazardRating > 0.22f) {
        table.factions.push_back({"anomaly-research", "Anomaly Research Team", EncounterDisposition::Hidden, 1, 0});
    }

    table.encounters.push_back({"miner", "Miner Survey Ship", EncounterDisposition::Neutral, 5, 1, {"mining", "civilian"}});
    table.encounters.push_back({"trader", "Trade Courier", EncounterDisposition::Friendly, 4, 1, {"trade", "civilian"}});
    table.encounters.push_back({"salvage", "Salvage Skiff", EncounterDisposition::Neutral, 3, 2, {"salvage"}});
    if (highPirateRisk) {
        table.encounters.push_back({"pirate-raider", "Pirate Raider", EncounterDisposition::Pirate, 3, 3, {"combat", "pirate"}});
        table.encounters.push_back({"pirate-ambush", "Hidden Pirate Ambush", EncounterDisposition::Hostile, 1, 5, {"combat", "pirate", "ambush"}});
    }
    if (survey.asteroidField.resourceRichness >= 7) {
        table.encounters.push_back({"claim-jumper", "Claim Jumper", EncounterDisposition::Hostile, 2, 3, {"mining", "rival"}});
    }

    return table;
}

std::string EncounterDispositionName(EncounterDisposition disposition)
{
    switch (disposition) {
        case EncounterDisposition::Friendly: return "Friendly";
        case EncounterDisposition::Neutral: return "Neutral";
        case EncounterDisposition::Hostile: return "Hostile";
        case EncounterDisposition::Pirate: return "Pirate";
        case EncounterDisposition::Hidden: return "Hidden";
        default: return "Unknown";
    }
}

std::string EncounterSpawnTableSummary(const EncounterSpawnTable& table)
{
    std::ostringstream stream;
    stream << table.sectorId << " factions=" << table.factions.size() << " encounters=" << table.encounters.size();
    return stream.str();
}

} // namespace subspace
