#include "diplomacy/DiplomacySystem.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// Treaty
// ---------------------------------------------------------------------------

std::string Treaty::GetTreatyName(TreatyType type) {
    switch (type) {
        case TreatyType::NonAggression:  return "Non-Aggression Pact";
        case TreatyType::TradeAgreement: return "Trade Agreement";
        case TreatyType::DefensivePact:  return "Defensive Pact";
        case TreatyType::Alliance:       return "Alliance";
        case TreatyType::Ceasefire:      return "Ceasefire";
    }
    return "Unknown";
}

float Treaty::GetProgress() const {
    if (totalDuration <= 0.0f) return 100.0f;
    float elapsed = totalDuration - duration;
    float pct = (elapsed / totalDuration) * 100.0f;
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    return pct;
}

// ---------------------------------------------------------------------------
// DiplomaticRelation
// ---------------------------------------------------------------------------

std::string DiplomaticRelation::GetStatusName(DiplomaticStatus status) {
    switch (status) {
        case DiplomaticStatus::War:           return "War";
        case DiplomaticStatus::Hostile:       return "Hostile";
        case DiplomaticStatus::Neutral:       return "Neutral";
        case DiplomaticStatus::NonAggression: return "Non-Aggression";
        case DiplomaticStatus::Trade:         return "Trade";
        case DiplomaticStatus::Alliance:      return "Alliance";
    }
    return "Neutral";
}

void DiplomaticRelation::ModifyTrust(int amount) {
    trust += amount;
    if (trust < -100) trust = -100;
    if (trust > 100) trust = 100;
}

// ---------------------------------------------------------------------------
// DiplomacyDatabase
// ---------------------------------------------------------------------------

std::string DiplomacyDatabase::AddTreaty(Treaty treaty) {
    if (treaty.treatyId.empty()) {
        treaty.treatyId = "treaty_" + std::to_string(_nextId++);
    }
    _treatyIndex[treaty.treatyId] = _treaties.size();
    _treaties.push_back(treaty);
    return treaty.treatyId;
}

const Treaty* DiplomacyDatabase::FindTreaty(const std::string& treatyId) const {
    auto it = _treatyIndex.find(treatyId);
    if (it == _treatyIndex.end()) return nullptr;
    return &_treaties[it->second];
}

Treaty* DiplomacyDatabase::FindTreaty(const std::string& treatyId) {
    auto it = _treatyIndex.find(treatyId);
    if (it == _treatyIndex.end()) return nullptr;
    return &_treaties[it->second];
}

std::vector<const Treaty*> DiplomacyDatabase::GetTreatiesForFaction(const std::string& factionId) const {
    std::vector<const Treaty*> result;
    for (const auto& t : _treaties) {
        if (t.factionA == factionId || t.factionB == factionId) {
            result.push_back(&t);
        }
    }
    return result;
}

std::vector<const Treaty*> DiplomacyDatabase::GetActiveTreaties() const {
    std::vector<const Treaty*> result;
    for (const auto& t : _treaties) {
        if (t.isActive) {
            result.push_back(&t);
        }
    }
    return result;
}

bool DiplomacyDatabase::RemoveTreaty(const std::string& treatyId) {
    auto it = _treatyIndex.find(treatyId);
    if (it == _treatyIndex.end()) return false;

    size_t idx = it->second;
    _treatyIndex.erase(it);

    // Swap-and-pop for efficient removal
    if (idx < _treaties.size() - 1) {
        _treaties[idx] = std::move(_treaties.back());
        _treatyIndex[_treaties[idx].treatyId] = idx;
    }
    _treaties.pop_back();
    return true;
}

size_t DiplomacyDatabase::GetTreatyCount() const {
    return _treaties.size();
}

DiplomacyDatabase DiplomacyDatabase::CreateDefaultDatabase() {
    DiplomacyDatabase db;

    // Trade agreement between Traders and Miners
    {
        Treaty t;
        t.type = TreatyType::TradeAgreement;
        t.factionA = "traders_guild";
        t.factionB = "miners_union";
        t.duration = -1.0f;
        t.totalDuration = -1.0f;
        db.AddTreaty(t);
    }

    // Non-aggression between Empire and Republic
    {
        Treaty t;
        t.type = TreatyType::NonAggression;
        t.factionA = "galactic_empire";
        t.factionB = "free_republic";
        t.duration = 300.0f;
        t.totalDuration = 300.0f;
        db.AddTreaty(t);
    }

    // Alliance between Republic and Miners
    {
        Treaty t;
        t.type = TreatyType::Alliance;
        t.factionA = "free_republic";
        t.factionB = "miners_union";
        t.duration = -1.0f;
        t.totalDuration = -1.0f;
        db.AddTreaty(t);
    }

    // Ceasefire between Empire and Pirates
    {
        Treaty t;
        t.type = TreatyType::Ceasefire;
        t.factionA = "galactic_empire";
        t.factionB = "pirate_clans";
        t.duration = 60.0f;
        t.totalDuration = 60.0f;
        db.AddTreaty(t);
    }

    return db;
}

// ---------------------------------------------------------------------------
// DiplomacyComponent
// ---------------------------------------------------------------------------

DiplomaticRelation* DiplomacyComponent::GetRelation(const std::string& otherFaction) {
    for (auto& r : relations) {
        if ((r.factionA == factionId && r.factionB == otherFaction) ||
            (r.factionB == factionId && r.factionA == otherFaction)) {
            return &r;
        }
    }
    return nullptr;
}

const DiplomaticRelation* DiplomacyComponent::GetRelation(const std::string& otherFaction) const {
    for (const auto& r : relations) {
        if ((r.factionA == factionId && r.factionB == otherFaction) ||
            (r.factionB == factionId && r.factionA == otherFaction)) {
            return &r;
        }
    }
    return nullptr;
}

DiplomaticRelation& DiplomacyComponent::AddRelation(const std::string& otherFaction,
                                                     DiplomaticStatus status) {
    DiplomaticRelation* existing = GetRelation(otherFaction);
    if (existing) return *existing;

    DiplomaticRelation rel;
    rel.factionA = factionId;
    rel.factionB = otherFaction;
    rel.status = status;
    relations.push_back(rel);
    return relations.back();
}

void DiplomacyComponent::DeclareWar(const std::string& otherFaction) {
    SetStatus(otherFaction, DiplomaticStatus::War);
}

void DiplomacyComponent::ProposePeace(const std::string& otherFaction) {
    SetStatus(otherFaction, DiplomaticStatus::Neutral);
}

void DiplomacyComponent::SetStatus(const std::string& otherFaction, DiplomaticStatus status) {
    DiplomaticRelation* rel = GetRelation(otherFaction);
    if (!rel) {
        AddRelation(otherFaction, status);
        return;
    }
    rel->status = status;
}

DiplomaticStatus DiplomacyComponent::GetStatus(const std::string& otherFaction) const {
    const DiplomaticRelation* rel = GetRelation(otherFaction);
    if (!rel) return DiplomaticStatus::Neutral;
    return rel->status;
}

std::vector<std::string> DiplomacyComponent::GetFactionsWithStatus(DiplomaticStatus status) const {
    std::vector<std::string> result;
    for (const auto& r : relations) {
        if (r.status == status) {
            // Return the "other" faction
            if (r.factionA == factionId) {
                result.push_back(r.factionB);
            } else {
                result.push_back(r.factionA);
            }
        }
    }
    return result;
}

size_t DiplomacyComponent::GetRelationCount() const {
    return relations.size();
}

bool DiplomacyComponent::IsAtWar() const {
    return GetWarCount() > 0;
}

int DiplomacyComponent::GetWarCount() const {
    int count = 0;
    for (const auto& r : relations) {
        if (r.status == DiplomaticStatus::War) ++count;
    }
    return count;
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData DiplomacyComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "DiplomacyComponent";
    cd.data["factionId"]        = factionId;
    cd.data["warWeariness"]     = std::to_string(warWeariness);
    cd.data["warWearinessRate"] = std::to_string(warWearinessRate);
    cd.data["trustGainRate"]    = std::to_string(trustGainRate);
    cd.data["relationCount"]    = std::to_string(relations.size());

    for (size_t i = 0; i < relations.size(); ++i) {
        std::string prefix = "rel_" + std::to_string(i) + "_";
        const auto& r = relations[i];
        cd.data[prefix + "factionA"]  = r.factionA;
        cd.data[prefix + "factionB"]  = r.factionB;
        cd.data[prefix + "status"]    = std::to_string(static_cast<int>(r.status));
        cd.data[prefix + "trust"]     = std::to_string(r.trust);
        cd.data[prefix + "treatyCount"] = std::to_string(r.activeTreatyIds.size());
        for (size_t j = 0; j < r.activeTreatyIds.size(); ++j) {
            cd.data[prefix + "treaty_" + std::to_string(j)] = r.activeTreatyIds[j];
        }
    }

    return cd;
}

void DiplomacyComponent::Deserialize(const ComponentData& data) {
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

    factionId        = getStr("factionId");
    warWeariness     = getFloat("warWeariness", 0.0f);
    warWearinessRate = getFloat("warWearinessRate", 1.0f);
    trustGainRate    = getFloat("trustGainRate", 0.1f);

    int relCount = getInt("relationCount", 0);
    relations.clear();
    relations.reserve(static_cast<size_t>(relCount));

    for (int i = 0; i < relCount; ++i) {
        std::string prefix = "rel_" + std::to_string(i) + "_";
        DiplomaticRelation r;
        r.factionA = getStr(prefix + "factionA");
        r.factionB = getStr(prefix + "factionB");

        constexpr int kMaxStatus = static_cast<int>(DiplomaticStatus::Alliance);
        int statusVal = getInt(prefix + "status", static_cast<int>(DiplomaticStatus::Neutral));
        if (statusVal >= 0 && statusVal <= kMaxStatus) {
            r.status = static_cast<DiplomaticStatus>(statusVal);
        } else {
            r.status = DiplomaticStatus::Neutral;
        }

        r.trust = getInt(prefix + "trust", 0);
        if (r.trust < -100) r.trust = -100;
        if (r.trust > 100) r.trust = 100;

        int treatyCount = getInt(prefix + "treatyCount", 0);
        for (int j = 0; j < treatyCount; ++j) {
            std::string tid = getStr(prefix + "treaty_" + std::to_string(j));
            if (!tid.empty()) r.activeTreatyIds.push_back(tid);
        }

        relations.push_back(r);
    }
}

// ---------------------------------------------------------------------------
// DiplomacySystem
// ---------------------------------------------------------------------------

DiplomacySystem::DiplomacySystem() : SystemBase("DiplomacySystem") {}

DiplomacySystem::DiplomacySystem(EntityManager& entityManager)
    : SystemBase("DiplomacySystem")
    , _entityManager(&entityManager)
{
}

void DiplomacySystem::Update(float deltaTime) {
    if (!_entityManager) return;

    auto diplomats = _entityManager->GetAllComponents<DiplomacyComponent>();
    for (auto* dc : diplomats) {
        // Increase war weariness for factions at war
        if (dc->IsAtWar()) {
            dc->warWeariness += dc->warWearinessRate * deltaTime;
            if (dc->warWeariness > 100.0f) dc->warWeariness = 100.0f;
        }

        // Gain trust with allied/trade partners
        for (auto& rel : dc->relations) {
            if (rel.status == DiplomaticStatus::Alliance ||
                rel.status == DiplomaticStatus::Trade) {
                float gain = dc->trustGainRate * deltaTime;
                int gainInt = static_cast<int>(gain);
                if (gainInt < 1 && gain > 0.0f) gainInt = 1;
                rel.ModifyTrust(gainInt);
            }
        }
    }
}

} // namespace subspace
