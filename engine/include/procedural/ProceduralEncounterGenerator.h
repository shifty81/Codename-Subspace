#pragma once

#include "factions/FactionEncounterModel.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

struct GeneratedEncounter {
    std::string id;
    std::string displayName;
    EncounterDisposition disposition = EncounterDisposition::Neutral;
    int threat = 1;
    std::vector<std::string> tags;
};

// Storyteller-style state layered over the existing encounter generator. This
// remains one encounter authority: the director adjusts weights/budget/cooldown
// while the sector spawn table continues to define what can actually appear.
struct EncounterDirectorState {
    double elapsedDays = 0.0;
    double daysSinceMajorThreat = 2.0;
    double daysSinceOpportunity = 1.0;
    double tension = 0.25;          // 0..1 rising dramatic pressure
    double recoveryNeed = 0.0;      // 0..1 biases toward relief/opportunity
    int fleetPower = 1;
    int crewCount = 1;
    int economicWealth = 0;
    int colonies = 0;
    int stations = 0;
    int storyPhase = 0;
    bool storyEnabled = true;
    std::vector<std::string> recentTags;
};

struct EncounterDirectorContext {
    std::string locationId;
    bool planetary = false;
    bool interior = false;
    bool docked = false;
    bool deepSpace = true;
    bool playerInCombat = false;
    std::vector<std::string> worldTags;
    std::vector<std::string> storyTags;
};

struct DirectedEncounterResult {
    GeneratedEncounter encounter{};
    bool shouldSpawn = false;
    int adjustedThreat = 0;
    double nextThreatCooldownDays = 0.0;
    double nextOpportunityCooldownDays = 0.0;
    std::vector<std::string> reasons;
};

class ProceduralEncounterGenerator {
public:
    GeneratedEncounter GenerateEncounter(const EncounterSpawnTable& table,
                                         int playerPower,
                                         std::uint32_t seed) const;

    DirectedEncounterResult GenerateDirectedEncounter(const EncounterSpawnTable& table,
                                                       const EncounterDirectorState& state,
                                                       const EncounterDirectorContext& context,
                                                       std::uint32_t seed) const;
};

std::string GeneratedEncounterSummary(const GeneratedEncounter& encounter);

} // namespace subspace
