#include "ships/DamageComponent.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// DamageRecord helpers
// ---------------------------------------------------------------------------

static std::string DamageTypeToString(DamageType t) {
    switch (t) {
        case DamageType::Kinetic:   return "Kinetic";
        case DamageType::Energy:    return "Energy";
        case DamageType::Explosive: return "Explosive";
        case DamageType::Thermal:   return "Thermal";
        case DamageType::EMP:       return "EMP";
    }
    return "Kinetic";
}

static DamageType DamageTypeFromString(const std::string& s) {
    if (s == "Energy")    return DamageType::Energy;
    if (s == "Explosive") return DamageType::Explosive;
    if (s == "Thermal")   return DamageType::Thermal;
    if (s == "EMP")       return DamageType::EMP;
    return DamageType::Kinetic;
}

// ---------------------------------------------------------------------------
// DamageComponent
// ---------------------------------------------------------------------------

void DamageComponent::AddDamageRecord(const DamageRecord& record) {
    if (damageHistory.size() >= kMaxHistorySize) {
        damageHistory.erase(damageHistory.begin());
    }
    damageHistory.push_back(record);
}

float DamageComponent::GetTotalDamageReceived() const {
    float total = 0.0f;
    for (const auto& r : damageHistory) {
        total += r.damageAmount;
    }
    return total;
}

float DamageComponent::GetRecentDamage(float withinSeconds, float currentTime) const {
    float cutoff = currentTime - withinSeconds;
    float total = 0.0f;
    for (const auto& r : damageHistory) {
        if (r.timestamp >= cutoff) {
            total += r.damageAmount;
        }
    }
    return total;
}

// ---------------------------------------------------------------------------
// Serialization (follows AudioComponent / AchievementComponent pattern)
// ---------------------------------------------------------------------------

ComponentData DamageComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "DamageComponent";
    cd.data["damageMultiplier"] = std::to_string(damageMultiplier);
    cd.data["repairRate"]       = std::to_string(repairRate);
    cd.data["isInvulnerable"]   = isInvulnerable ? "true" : "false";
    cd.data["hasStructuralDamage"] = hasStructuralDamage ? "true" : "false";
    cd.data["disconnectedFragments"] = std::to_string(disconnectedFragments);
    cd.data["recordCount"]      = std::to_string(damageHistory.size());

    for (size_t i = 0; i < damageHistory.size(); ++i) {
        const auto& r = damageHistory[i];
        std::string p = "rec_" + std::to_string(i) + "_";
        cd.data[p + "time"]   = std::to_string(r.timestamp);
        cd.data[p + "amount"] = std::to_string(r.damageAmount);
        cd.data[p + "type"]   = DamageTypeToString(r.damageType);
        cd.data[p + "posX"]   = std::to_string(r.hitPosition.x);
        cd.data[p + "posY"]   = std::to_string(r.hitPosition.y);
        cd.data[p + "posZ"]   = std::to_string(r.hitPosition.z);
    }
    return cd;
}

void DamageComponent::Deserialize(const ComponentData& data) {
    damageHistory.clear();

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

    damageMultiplier       = getFloat("damageMultiplier", 1.0f);
    repairRate             = getFloat("repairRate", 0.0f);
    isInvulnerable         = getStr("isInvulnerable") == "true";
    hasStructuralDamage    = getStr("hasStructuralDamage") == "true";
    disconnectedFragments  = getInt("disconnectedFragments", 0);

    int count = getInt("recordCount", 0);
    for (int i = 0; i < count; ++i) {
        std::string p = "rec_" + std::to_string(i) + "_";
        DamageRecord r;
        r.timestamp    = getFloat(p + "time", 0.0f);
        r.damageAmount = getFloat(p + "amount", 0.0f);
        r.damageType   = DamageTypeFromString(getStr(p + "type"));
        r.hitPosition  = Vector3Int(getInt(p + "posX"), getInt(p + "posY"), getInt(p + "posZ"));
        damageHistory.push_back(r);
    }
}

} // namespace subspace
