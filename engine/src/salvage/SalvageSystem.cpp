#include "salvage/SalvageSystem.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// SalvageTarget helpers
// ---------------------------------------------------------------------------

std::string SalvageTarget::GetTierName(SalvageTier tier) {
    switch (tier) {
        case SalvageTier::Basic:        return "Basic";
        case SalvageTier::Advanced:     return "Advanced";
        case SalvageTier::Industrial:   return "Industrial";
        case SalvageTier::Military:     return "Military";
        case SalvageTier::Experimental: return "Experimental";
    }
    return "Unknown";
}

std::string SalvageTarget::GetStateName(SalvageState state) {
    switch (state) {
        case SalvageState::Idle:        return "Idle";
        case SalvageState::Approaching: return "Approaching";
        case SalvageState::Salvaging:   return "Salvaging";
        case SalvageState::Completed:   return "Completed";
        case SalvageState::Failed:      return "Failed";
    }
    return "Unknown";
}

std::vector<SalvageTarget> SalvageTarget::GetDefaultWreckTypes() {
    return {
        {0, "Small Debris",       "Iron",     50,  50,  0.3f, SalvageState::Idle, 0.0f, 0.0f},
        {0, "Fighter Wreck",      "Titanium", 100, 100, 0.5f, SalvageState::Idle, 0.0f, 0.0f},
        {0, "Frigate Wreck",      "Naonite",  250, 250, 0.6f, SalvageState::Idle, 0.0f, 0.0f},
        {0, "Cruiser Wreck",      "Trinium",  500, 500, 0.7f, SalvageState::Idle, 0.0f, 0.0f},
        {0, "Battleship Wreck",   "Xanion",   800, 800, 0.8f, SalvageState::Idle, 0.0f, 0.0f},
        {0, "Station Debris",     "Ogonite",  1200, 1200, 0.9f, SalvageState::Idle, 0.0f, 0.0f},
        {0, "Ancient Artifact",   "Avorion",  200, 200, 1.0f, SalvageState::Idle, 0.0f, 0.0f},
        {0, "Cargo Container",    "Iron",     30,  30,  0.2f, SalvageState::Idle, 0.0f, 0.0f},
    };
}

// ---------------------------------------------------------------------------
// SalvageComponent
// ---------------------------------------------------------------------------

SalvageComponent::SalvageComponent(SalvageTier tier, float range)
    : _tier(tier)
    , _range(range)
{
}

SalvageTier SalvageComponent::GetTier() const {
    return _tier;
}

void SalvageComponent::SetTier(SalvageTier tier) {
    _tier = tier;
}

float SalvageComponent::GetRange() const {
    return _range;
}

void SalvageComponent::SetRange(float range) {
    _range = range;
}

int SalvageComponent::GetMaxTargets() const {
    // More targets at higher tiers: Basic 2, Advanced 3, Industrial 4, Military 5, Experimental 6
    return 2 + static_cast<int>(_tier);
}

int SalvageComponent::GetActiveTargetCount() const {
    int count = 0;
    for (const auto& t : _targets) {
        if (t.state == SalvageState::Approaching ||
            t.state == SalvageState::Salvaging) {
            ++count;
        }
    }
    return count;
}

bool SalvageComponent::StartSalvage(const SalvageTarget& target) {
    // Check capacity (only active targets count against limit)
    if (GetActiveTargetCount() >= GetMaxTargets()) {
        return false;
    }

    SalvageTarget t = target;
    t.state = SalvageState::Approaching;
    t.progress = 0.0f;

    _targets.push_back(t);
    return true;
}

bool SalvageComponent::CancelSalvage(uint64_t targetId) {
    auto it = std::find_if(_targets.begin(), _targets.end(),
        [targetId](const SalvageTarget& t) { return t.targetId == targetId; });

    if (it == _targets.end()) return false;

    _targets.erase(it);
    return true;
}

std::pair<std::string, int> SalvageComponent::CollectSalvage(uint64_t targetId) {
    auto it = std::find_if(_targets.begin(), _targets.end(),
        [targetId](const SalvageTarget& t) { return t.targetId == targetId; });

    if (it == _targets.end() || it->state != SalvageState::Completed) {
        return {"", 0};
    }

    float yield = static_cast<float>(it->totalYield)
                * it->integrity
                * GetEfficiencyMultiplier();
    int outputAmount = static_cast<int>(yield);
    std::string material = it->primaryMaterial;

    _totalCollected += outputAmount;
    _targets.erase(it);
    return {material, outputAmount};
}

const SalvageTarget* SalvageComponent::GetTarget(uint64_t targetId) const {
    for (const auto& t : _targets) {
        if (t.targetId == targetId) return &t;
    }
    return nullptr;
}

const std::vector<SalvageTarget>& SalvageComponent::GetAllTargets() const {
    return _targets;
}

int SalvageComponent::GetCompletedCount() const {
    int count = 0;
    for (const auto& t : _targets) {
        if (t.state == SalvageState::Completed) ++count;
    }
    return count;
}

int SalvageComponent::GetTotalMaterialsCollected() const {
    return _totalCollected;
}

float SalvageComponent::GetEfficiencyMultiplier() const {
    return 1.0f + 0.125f * static_cast<float>(static_cast<int>(_tier));
}

float SalvageComponent::GetSpeedMultiplier() const {
    return 1.0f + 0.2f * static_cast<float>(static_cast<int>(_tier));
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData SalvageComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "SalvageComponent";
    cd.data["tier"]           = std::to_string(static_cast<int>(_tier));
    cd.data["range"]          = std::to_string(_range);
    cd.data["maxTargets"]     = std::to_string(_maxTargets);
    cd.data["totalCollected"] = std::to_string(_totalCollected);

    cd.data["targetCount"] = std::to_string(_targets.size());
    for (size_t i = 0; i < _targets.size(); ++i) {
        std::string prefix = "target_" + std::to_string(i) + "_";
        const auto& t = _targets[i];
        cd.data[prefix + "targetId"]        = std::to_string(t.targetId);
        cd.data[prefix + "wreckName"]       = t.wreckName;
        cd.data[prefix + "primaryMaterial"] = t.primaryMaterial;
        cd.data[prefix + "totalYield"]      = std::to_string(t.totalYield);
        cd.data[prefix + "remainingYield"]  = std::to_string(t.remainingYield);
        cd.data[prefix + "integrity"]       = std::to_string(t.integrity);
        cd.data[prefix + "state"]           = std::to_string(static_cast<int>(t.state));
        cd.data[prefix + "progress"]        = std::to_string(t.progress);
        cd.data[prefix + "distance"]        = std::to_string(t.distance);
    }

    return cd;
}

void SalvageComponent::Deserialize(const ComponentData& data) {
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
    auto getUint64 = [&](const std::string& key, uint64_t def = 0) -> uint64_t {
        auto it = data.data.find(key);
        if (it == data.data.end()) return def;
        try { return std::stoull(it->second); } catch (...) { return def; }
    };

    int tierVal = getInt("tier", 0);
    constexpr int kMaxTier = static_cast<int>(SalvageTier::Experimental);
    if (tierVal >= 0 && tierVal <= kMaxTier) {
        _tier = static_cast<SalvageTier>(tierVal);
    } else {
        _tier = SalvageTier::Basic;
    }

    _range          = getFloat("range", 500.0f);
    _maxTargets     = getInt("maxTargets", 2);
    _totalCollected = getInt("totalCollected", 0);

    int targetCount = getInt("targetCount", 0);
    _targets.clear();
    _targets.reserve(static_cast<size_t>(targetCount));
    for (int i = 0; i < targetCount; ++i) {
        std::string prefix = "target_" + std::to_string(i) + "_";
        SalvageTarget t;
        t.targetId        = getUint64(prefix + "targetId", 0);
        t.wreckName       = getStr(prefix + "wreckName");
        t.primaryMaterial = getStr(prefix + "primaryMaterial");
        t.totalYield      = getInt(prefix + "totalYield", 0);
        t.remainingYield  = getInt(prefix + "remainingYield", 0);
        t.integrity       = getFloat(prefix + "integrity", 1.0f);
        int stateVal      = getInt(prefix + "state", 0);
        constexpr int kMaxState = static_cast<int>(SalvageState::Failed);
        if (stateVal >= 0 && stateVal <= kMaxState) {
            t.state = static_cast<SalvageState>(stateVal);
        } else {
            t.state = SalvageState::Idle;
        }
        t.progress = getFloat(prefix + "progress", 0.0f);
        t.distance = getFloat(prefix + "distance", 0.0f);
        _targets.push_back(t);
    }
}

// ---------------------------------------------------------------------------
// SalvageSystem
// ---------------------------------------------------------------------------

SalvageSystem::SalvageSystem() : SystemBase("SalvageSystem") {}

SalvageSystem::SalvageSystem(EntityManager& entityManager)
    : SystemBase("SalvageSystem")
    , _entityManager(&entityManager)
{
}

void SalvageSystem::SetEntityManager(EntityManager* em) {
    _entityManager = em;
}

void SalvageSystem::Update(float deltaTime) {
    if (!_entityManager) return;

    auto salvagers = _entityManager->GetAllComponents<SalvageComponent>();
    for (auto* salvager : salvagers) {
        float speedMult = salvager->GetSpeedMultiplier();

        for (auto& target : salvager->_targets) {
            switch (target.state) {
                case SalvageState::Approaching: {
                    target.state = SalvageState::Salvaging;
                    break;
                }

                case SalvageState::Salvaging: {
                    // Base salvage time: 8 seconds per cycle
                    float baseSalvageTime = 8.0f;
                    target.progress += (speedMult / baseSalvageTime) * deltaTime;

                    if (target.progress >= 1.0f) {
                        target.progress = 1.0f;
                        target.state = SalvageState::Completed;
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }
}

} // namespace subspace
