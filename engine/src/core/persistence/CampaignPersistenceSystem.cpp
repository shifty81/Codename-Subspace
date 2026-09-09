#include "core/persistence/CampaignPersistenceSystem.h"

#include <algorithm>
#include <sstream>
#include <vector>

namespace subspace {

static std::vector<std::string> Split(const std::string& value, char delimiter) {
    std::vector<std::string> out; std::stringstream s(value); std::string part;
    while (std::getline(s, part, delimiter)) if (!part.empty()) out.push_back(part);
    return out;
}

std::string CampaignPersistenceSystem::EncodeDoubleMap(const std::unordered_map<std::string, double>& values) {
    std::vector<std::string> keys; keys.reserve(values.size()); for (const auto& p:values) keys.push_back(p.first);
    std::sort(keys.begin(), keys.end()); std::ostringstream out;
    bool first=true; for (const auto& key:keys) { if(!first) out<<';'; first=false; out<<key<<':'<<values.at(key); }
    return out.str();
}

std::string CampaignPersistenceSystem::EncodeUIntMap(const std::unordered_map<std::string, std::uint64_t>& values) {
    std::vector<std::string> keys; keys.reserve(values.size()); for (const auto& p:values) keys.push_back(p.first);
    std::sort(keys.begin(), keys.end()); std::ostringstream out;
    bool first=true; for (const auto& key:keys) { if(!first) out<<';'; first=false; out<<key<<':'<<values.at(key); }
    return out.str();
}

void CampaignPersistenceSystem::WriteToSave(const CampaignState& s, SaveGameData& save) const {
    auto& g=save.gameState;
    g["campaign.credits"]=std::to_string(s.credits);
    g["campaign.sectorX"]=std::to_string(s.sectorX); g["campaign.sectorY"]=std::to_string(s.sectorY); g["campaign.sectorZ"]=std::to_string(s.sectorZ);
    g["campaign.playerX"]=std::to_string(s.playerX); g["campaign.playerY"]=std::to_string(s.playerY);
    g["campaign.activeShipId"]=std::to_string(s.activeShipId); g["campaign.deaths"]=std::to_string(s.deaths);
    g["campaign.logisticsTier"]=std::to_string(s.logisticsTier); g["campaign.economyTick"]=std::to_string(s.economyTick);
    g["campaign.factionStandings"]=EncodeDoubleMap(s.factionStandings); g["campaign.cargo"]=EncodeUIntMap(s.cargo);
    g["campaign.persistentUniverseLedger"]=s.persistentUniverseLedger;
}

bool CampaignPersistenceSystem::ReadFromSave(const SaveGameData& save, CampaignState& s) const {
    auto get=[&](const char* key)->const std::string*{ auto it=save.gameState.find(key); return it==save.gameState.end()?nullptr:&it->second; };
    try {
        if (auto v=get("campaign.credits")) s.credits=std::stod(*v);
        if (auto v=get("campaign.sectorX")) s.sectorX=std::stoi(*v); if (auto v=get("campaign.sectorY")) s.sectorY=std::stoi(*v); if (auto v=get("campaign.sectorZ")) s.sectorZ=std::stoi(*v);
        if (auto v=get("campaign.playerX")) s.playerX=std::stod(*v); if (auto v=get("campaign.playerY")) s.playerY=std::stod(*v);
        if (auto v=get("campaign.activeShipId")) s.activeShipId=std::stoull(*v); if (auto v=get("campaign.deaths")) s.deaths=std::stoull(*v);
        if (auto v=get("campaign.logisticsTier")) s.logisticsTier=std::stoi(*v); if (auto v=get("campaign.economyTick")) s.economyTick=std::stoull(*v);
        s.factionStandings.clear(); s.cargo.clear();
        if (auto v=get("campaign.factionStandings")) for (const auto& pair:Split(*v,';')) { auto p=pair.find(':'); if(p!=std::string::npos) s.factionStandings[pair.substr(0,p)]=std::stod(pair.substr(p+1)); }
        if (auto v=get("campaign.cargo")) for (const auto& pair:Split(*v,';')) { auto p=pair.find(':'); if(p!=std::string::npos) s.cargo[pair.substr(0,p)]=std::stoull(pair.substr(p+1)); }
        if (auto v=get("campaign.persistentUniverseLedger")) s.persistentUniverseLedger=*v; else s.persistentUniverseLedger.clear();
    } catch (...) { return false; }
    return true;
}

} // namespace subspace
