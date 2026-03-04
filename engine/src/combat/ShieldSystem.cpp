#include "combat/ShieldSystem.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// ShieldComponent
// ---------------------------------------------------------------------------

float ShieldComponent::AbsorbDamage(float damage) {
    if (!isActive) return damage;

    float absorbed = damage * GetAbsorptionMultiplier(shieldType);
    timeSinceLastHit = 0.0f;

    // First consume overcharge
    if (overchargeAmount > 0.0f) {
        if (absorbed <= overchargeAmount) {
            overchargeAmount -= absorbed;
            return 0.0f;
        }
        absorbed -= overchargeAmount;
        overchargeAmount = 0.0f;
    }

    // Then consume currentShield
    if (absorbed <= currentShield) {
        currentShield -= absorbed;
        return 0.0f;
    }

    float overflow = absorbed - currentShield;
    currentShield = 0.0f;
    return overflow;
}

void ShieldComponent::ApplyOvercharge(float amount) {
    if (amount > 0.0f) {
        overchargeAmount += amount;
    }
}

float ShieldComponent::GetEffectiveShield() const {
    return currentShield + overchargeAmount;
}

float ShieldComponent::GetShieldPercentage() const {
    if (maxShield <= 0.0f) return 0.0f;
    float pct = (currentShield / maxShield) * 100.0f;
    if (pct < 0.0f) pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    return pct;
}

bool ShieldComponent::IsDepleted() const {
    return currentShield <= 0.0f && overchargeAmount <= 0.0f;
}

void ShieldComponent::RestoreShield() {
    currentShield = maxShield;
    overchargeAmount = 0.0f;
}

float ShieldComponent::GetAbsorptionMultiplier(ShieldType type) {
    switch (type) {
        case ShieldType::Standard:     return 1.0f;
        case ShieldType::Hardened:     return 0.7f;
        case ShieldType::Phase:        return 0.85f;
        case ShieldType::Regenerative: return 1.1f;
    }
    return 1.0f;
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData ShieldComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "ShieldComponent";
    cd.data["shieldType"]          = std::to_string(static_cast<int>(shieldType));
    cd.data["maxShield"]           = std::to_string(maxShield);
    cd.data["currentShield"]       = std::to_string(currentShield);
    cd.data["regenRate"]           = std::to_string(regenRate);
    cd.data["regenDelay"]          = std::to_string(regenDelay);
    cd.data["timeSinceLastHit"]    = std::to_string(timeSinceLastHit);
    cd.data["isActive"]            = isActive ? "1" : "0";
    cd.data["overchargeAmount"]    = std::to_string(overchargeAmount);
    cd.data["overchargeDecayRate"] = std::to_string(overchargeDecayRate);
    return cd;
}

void ShieldComponent::Deserialize(const ComponentData& data) {
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

    constexpr int kMaxShieldType = static_cast<int>(ShieldType::Regenerative);
    int typeVal = getInt("shieldType", 0);
    if (typeVal >= 0 && typeVal <= kMaxShieldType) {
        shieldType = static_cast<ShieldType>(typeVal);
    } else {
        shieldType = ShieldType::Standard;
    }

    maxShield           = getFloat("maxShield", 100.0f);
    currentShield       = getFloat("currentShield", 100.0f);
    regenRate           = getFloat("regenRate", 5.0f);
    regenDelay          = getFloat("regenDelay", 3.0f);
    timeSinceLastHit    = getFloat("timeSinceLastHit", 0.0f);
    isActive            = getStr("isActive") != "0";
    overchargeAmount    = getFloat("overchargeAmount", 0.0f);
    overchargeDecayRate = getFloat("overchargeDecayRate", 10.0f);
}

// ---------------------------------------------------------------------------
// ShieldSystem
// ---------------------------------------------------------------------------

ShieldSystem::ShieldSystem() : SystemBase("ShieldSystem") {}

ShieldSystem::ShieldSystem(EntityManager& entityManager)
    : SystemBase("ShieldSystem")
    , _entityManager(&entityManager)
{
}

void ShieldSystem::Update(float deltaTime) {
    if (!_entityManager) return;

    auto shields = _entityManager->GetAllComponents<ShieldComponent>();
    for (auto* shield : shields) {
        if (!shield->isActive) continue;

        // Decay overcharge
        if (shield->overchargeAmount > 0.0f) {
            shield->overchargeAmount -= shield->overchargeDecayRate * deltaTime;
            if (shield->overchargeAmount < 0.0f) {
                shield->overchargeAmount = 0.0f;
            }
        }

        // Advance time since last hit
        shield->timeSinceLastHit += deltaTime;

        // Regenerate shield after delay
        if (shield->timeSinceLastHit >= shield->regenDelay &&
            shield->currentShield < shield->maxShield) {
            shield->currentShield += shield->regenRate * deltaTime;
            if (shield->currentShield > shield->maxShield) {
                shield->currentShield = shield->maxShield;
            }
        }
    }
}

} // namespace subspace
