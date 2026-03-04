#include "reputation/ReputationSystem.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// FactionReputation
// ---------------------------------------------------------------------------

Standing FactionReputation::GetStanding() const {
    if (reputation <= -500) return Standing::Hostile;
    if (reputation <= -100) return Standing::Unfriendly;
    if (reputation <= 100)  return Standing::Neutral;
    if (reputation <= 500)  return Standing::Friendly;
    return Standing::Allied;
}

void FactionReputation::ModifyReputation(int amount) {
    reputation += amount;
    if (reputation < minReputation) reputation = minReputation;
    if (reputation > maxReputation) reputation = maxReputation;
}

float FactionReputation::GetNormalizedReputation() const {
    if (maxReputation == 0) return 0.0f;
    return static_cast<float>(reputation) / static_cast<float>(maxReputation);
}

std::string FactionReputation::GetStandingName(Standing standing) {
    switch (standing) {
        case Standing::Hostile:    return "Hostile";
        case Standing::Unfriendly: return "Unfriendly";
        case Standing::Neutral:    return "Neutral";
        case Standing::Friendly:   return "Friendly";
        case Standing::Allied:     return "Allied";
    }
    return "Neutral";
}

int FactionReputation::GetStandingThreshold(Standing standing) {
    switch (standing) {
        case Standing::Hostile:    return -500;
        case Standing::Unfriendly: return -100;
        case Standing::Neutral:    return 100;
        case Standing::Friendly:   return 500;
        case Standing::Allied:     return 500;
    }
    return 0;
}

// ---------------------------------------------------------------------------
// ReputationComponent
// ---------------------------------------------------------------------------

FactionReputation* ReputationComponent::GetFaction(const std::string& factionId) {
    for (auto& f : factions) {
        if (f.factionId == factionId) return &f;
    }
    return nullptr;
}

const FactionReputation* ReputationComponent::GetFaction(const std::string& factionId) const {
    for (const auto& f : factions) {
        if (f.factionId == factionId) return &f;
    }
    return nullptr;
}

FactionReputation& ReputationComponent::AddFaction(const std::string& factionId, int initialRep) {
    // Return existing entry if already tracked
    FactionReputation* existing = GetFaction(factionId);
    if (existing) return *existing;

    FactionReputation fr;
    fr.factionId = factionId;
    fr.reputation = initialRep;
    if (fr.reputation < fr.minReputation) fr.reputation = fr.minReputation;
    if (fr.reputation > fr.maxReputation) fr.reputation = fr.maxReputation;
    factions.push_back(fr);
    return factions.back();
}

void ReputationComponent::ModifyReputation(const std::string& factionId, int amount,
                                           const std::string& reason) {
    FactionReputation* fr = GetFaction(factionId);
    if (!fr) {
        AddFaction(factionId, 0);
        fr = GetFaction(factionId);
    }
    fr->ModifyReputation(amount);

    ReputationEvent evt;
    evt.factionId = factionId;
    evt.amount = amount;
    evt.reason = reason;
    recentEvents.push_back(evt);

    // Trim history
    while (static_cast<int>(recentEvents.size()) > maxEventHistory) {
        recentEvents.erase(recentEvents.begin());
    }
}

Standing ReputationComponent::GetStanding(const std::string& factionId) const {
    const FactionReputation* fr = GetFaction(factionId);
    if (!fr) return Standing::Neutral;
    return fr->GetStanding();
}

std::vector<std::string> ReputationComponent::GetFactionsWithStanding(Standing standing) const {
    std::vector<std::string> result;
    for (const auto& f : factions) {
        if (f.GetStanding() == standing) {
            result.push_back(f.factionId);
        }
    }
    return result;
}

size_t ReputationComponent::GetFactionCount() const {
    return factions.size();
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData ReputationComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "ReputationComponent";
    cd.data["decayRate"]       = std::to_string(decayRate);
    cd.data["maxEventHistory"] = std::to_string(maxEventHistory);
    cd.data["factionCount"]    = std::to_string(factions.size());

    for (size_t i = 0; i < factions.size(); ++i) {
        std::string prefix = "faction_" + std::to_string(i) + "_";
        const auto& f = factions[i];
        cd.data[prefix + "id"]     = f.factionId;
        cd.data[prefix + "rep"]    = std::to_string(f.reputation);
        cd.data[prefix + "minRep"] = std::to_string(f.minReputation);
        cd.data[prefix + "maxRep"] = std::to_string(f.maxReputation);
    }

    cd.data["eventCount"] = std::to_string(recentEvents.size());
    for (size_t i = 0; i < recentEvents.size(); ++i) {
        std::string prefix = "event_" + std::to_string(i) + "_";
        const auto& e = recentEvents[i];
        cd.data[prefix + "factionId"] = e.factionId;
        cd.data[prefix + "amount"]    = std::to_string(e.amount);
        cd.data[prefix + "reason"]    = e.reason;
    }

    return cd;
}

void ReputationComponent::Deserialize(const ComponentData& data) {
    auto getStr = [&](const std::string& key) -> std::string {
        auto it = data.data.find(key);
        return it != data.data.end() ? it->second : "";
    };
    auto getInt = [&](const std::string& key, int def = 0) -> int {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stoi(it->second); } catch (...) { return def; }
    };
    auto getFloat = [&](const std::string& key, float def = 0.0f) -> float {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stof(it->second); } catch (...) { return def; }
    };

    decayRate       = getFloat("decayRate", 0.0f);
    maxEventHistory = getInt("maxEventHistory", 20);

    int factionCount = getInt("factionCount", 0);
    factions.clear();
    factions.reserve(static_cast<size_t>(factionCount));

    for (int i = 0; i < factionCount; ++i) {
        std::string prefix = "faction_" + std::to_string(i) + "_";
        FactionReputation fr;
        fr.factionId     = getStr(prefix + "id");
        fr.reputation    = getInt(prefix + "rep", 0);
        fr.minReputation = getInt(prefix + "minRep", -1000);
        fr.maxReputation = getInt(prefix + "maxRep", 1000);
        factions.push_back(fr);
    }

    int eventCount = getInt("eventCount", 0);
    recentEvents.clear();
    recentEvents.reserve(static_cast<size_t>(eventCount));

    for (int i = 0; i < eventCount; ++i) {
        std::string prefix = "event_" + std::to_string(i) + "_";
        ReputationEvent evt;
        evt.factionId = getStr(prefix + "factionId");
        evt.amount    = getInt(prefix + "amount", 0);
        evt.reason    = getStr(prefix + "reason");
        recentEvents.push_back(evt);
    }
}

// ---------------------------------------------------------------------------
// ReputationSystem
// ---------------------------------------------------------------------------

ReputationSystem::ReputationSystem() : SystemBase("ReputationSystem") {}

ReputationSystem::ReputationSystem(EntityManager& entityManager)
    : SystemBase("ReputationSystem")
    , _entityManager(&entityManager)
{
}

void ReputationSystem::Update(float deltaTime) {
    if (!_entityManager) return;

    auto reputations = _entityManager->GetAllComponents<ReputationComponent>();
    for (auto* rep : reputations) {
        if (rep->decayRate <= 0.0f) continue;

        float decay = rep->decayRate * deltaTime;
        int decayInt = static_cast<int>(decay);
        if (decayInt < 1 && decay > 0.0f) decayInt = 1;

        for (auto& f : rep->factions) {
            if (f.reputation > 0) {
                f.reputation -= decayInt;
                if (f.reputation < 0) f.reputation = 0;
            } else if (f.reputation < 0) {
                f.reputation += decayInt;
                if (f.reputation > 0) f.reputation = 0;
            }
        }
    }
}

} // namespace subspace
