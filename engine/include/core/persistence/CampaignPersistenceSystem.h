#pragma once

#include "core/persistence/SaveGameManager.h"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace subspace {

struct CampaignState {
    double credits = 0.0;
    int sectorX = 0;
    int sectorY = 0;
    int sectorZ = 0;
    double playerX = 0.0;
    double playerY = 0.0;
    std::uint64_t activeShipId = 0;
    std::uint64_t deaths = 0;
    int logisticsTier = 0;
    std::uint64_t economyTick = 0;
    std::unordered_map<std::string, double> factionStandings;
    std::unordered_map<std::string, std::uint64_t> cargo;
    // Pass294: canonical persistent-universe event ledger embedded in the campaign save.
    std::string persistentUniverseLedger;
};

class CampaignPersistenceSystem {
public:
    void WriteToSave(const CampaignState& state, SaveGameData& save) const;
    bool ReadFromSave(const SaveGameData& save, CampaignState& state) const;

private:
    static std::string EncodeDoubleMap(const std::unordered_map<std::string, double>& values);
    static std::string EncodeUIntMap(const std::unordered_map<std::string, std::uint64_t>& values);
};

} // namespace subspace
