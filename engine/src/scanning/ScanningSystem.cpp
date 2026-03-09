#include "scanning/ScanningSystem.h"

#include <algorithm>

namespace subspace {

// ---------------------------------------------------------------------------
// ScanResult helpers
// ---------------------------------------------------------------------------

std::string ScanResult::GetTypeName(ScannerType type) {
    switch (type) {
        case ScannerType::Passive:  return "Passive";
        case ScannerType::Active:   return "Active";
        case ScannerType::Deep:     return "Deep";
        case ScannerType::Military: return "Military";
    }
    return "Unknown";
}

std::string ScanResult::GetStateName(ScanState state) {
    switch (state) {
        case ScanState::Idle:      return "Idle";
        case ScanState::Scanning:  return "Scanning";
        case ScanState::Analyzing: return "Analyzing";
        case ScanState::Complete:  return "Complete";
        case ScanState::Jammed:    return "Jammed";
    }
    return "Unknown";
}

std::string ScanResult::GetClassName(SignatureClass cls) {
    switch (cls) {
        case SignatureClass::Unknown:  return "Unknown";
        case SignatureClass::Ship:     return "Ship";
        case SignatureClass::Station:  return "Station";
        case SignatureClass::Asteroid: return "Asteroid";
        case SignatureClass::Anomaly:  return "Anomaly";
        case SignatureClass::Debris:   return "Debris";
    }
    return "Unknown";
}

// ---------------------------------------------------------------------------
// ScannerComponent
// ---------------------------------------------------------------------------

ScannerComponent::ScannerComponent(ScannerType type, float range)
    : _type(type)
    , _baseRange(range)
{
}

ScannerType ScannerComponent::GetType() const {
    return _type;
}

void ScannerComponent::SetType(ScannerType type) {
    _type = type;
}

float ScannerComponent::GetRange() const {
    return _baseRange * GetRangeMultiplier();
}

void ScannerComponent::SetRange(float range) {
    _baseRange = range;
}

float ScannerComponent::GetResolution() const {
    // Higher tier scanners have better resolution (lower is better)
    // Passive: 1.0, Active: 0.8, Deep: 0.5, Military: 0.3
    switch (_type) {
        case ScannerType::Passive:  return 1.0f;
        case ScannerType::Active:   return 0.8f;
        case ScannerType::Deep:     return 0.5f;
        case ScannerType::Military: return 0.3f;
    }
    return 1.0f;
}

int ScannerComponent::GetMaxConcurrentScans() const {
    switch (_type) {
        case ScannerType::Passive:  return 2;
        case ScannerType::Active:   return 4;
        case ScannerType::Deep:     return 3;
        case ScannerType::Military: return 6;
    }
    return 2;
}

int ScannerComponent::GetActiveScanCount() const {
    int count = 0;
    for (const auto& scan : _scans) {
        if (!scan.isFullyScanned) ++count;
    }
    return count;
}

bool ScannerComponent::StartScan(uint64_t targetId, const std::string& targetName,
                                  float distance, float posX, float posY, float posZ) {
    // Check capacity
    if (GetActiveScanCount() >= GetMaxConcurrentScans()) return false;

    // Check if already scanning this target
    for (const auto& scan : _scans) {
        if (scan.targetId == targetId && !scan.isFullyScanned) return false;
    }

    ScanResult result;
    result.targetId = targetId;
    result.targetName = targetName;
    result.distance = distance;
    result.posX = posX;
    result.posY = posY;
    result.posZ = posZ;
    result.signalStrength = 0.0f;
    result.scanProgress = 0.0f;
    result.classification = SignatureClass::Unknown;
    result.isFullyScanned = false;

    _scans.push_back(result);

    // Start cooldown after initiating scans
    _cooldownTimer = _cooldownDuration;

    return true;
}

bool ScannerComponent::CancelScan(uint64_t targetId) {
    auto it = std::find_if(_scans.begin(), _scans.end(),
        [targetId](const ScanResult& s) { return s.targetId == targetId && !s.isFullyScanned; });

    if (it == _scans.end()) return false;

    _scans.erase(it);
    return true;
}

const ScanResult* ScannerComponent::GetScanResult(uint64_t targetId) const {
    for (const auto& scan : _scans) {
        if (scan.targetId == targetId) return &scan;
    }
    return nullptr;
}

const std::vector<ScanResult>& ScannerComponent::GetAllScans() const {
    return _scans;
}

int ScannerComponent::GetCompletedScanCount() const {
    int count = 0;
    for (const auto& scan : _scans) {
        if (scan.isFullyScanned) ++count;
    }
    return count;
}

void ScannerComponent::ClearCompletedScans() {
    _scans.erase(
        std::remove_if(_scans.begin(), _scans.end(),
            [](const ScanResult& s) { return s.isFullyScanned; }),
        _scans.end());
}

bool ScannerComponent::IsOnCooldown() const {
    return _cooldownTimer > 0.0f;
}

float ScannerComponent::GetCooldownRemaining() const {
    return _cooldownTimer;
}

float ScannerComponent::GetScanSpeedMultiplier() const {
    // Passive: 1.0, Active: 1.5, Deep: 1.2, Military: 2.0
    return 1.0f + 0.33f * static_cast<float>(static_cast<int>(_type));
}

float ScannerComponent::GetRangeMultiplier() const {
    // Passive: 1.0, Active: 1.3, Deep: 1.5, Military: 1.8
    return 1.0f + 0.27f * static_cast<float>(static_cast<int>(_type));
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

ComponentData ScannerComponent::Serialize() const {
    ComponentData cd;
    cd.componentType = "ScannerComponent";
    cd.data["type"]             = std::to_string(static_cast<int>(_type));
    cd.data["baseRange"]        = std::to_string(_baseRange);
    cd.data["cooldownTimer"]    = std::to_string(_cooldownTimer);
    cd.data["cooldownDuration"] = std::to_string(_cooldownDuration);

    cd.data["scanCount"] = std::to_string(_scans.size());
    for (size_t i = 0; i < _scans.size(); ++i) {
        std::string prefix = "scan_" + std::to_string(i) + "_";
        const auto& s = _scans[i];
        cd.data[prefix + "targetId"]       = std::to_string(s.targetId);
        cd.data[prefix + "targetName"]     = s.targetName;
        cd.data[prefix + "classification"] = std::to_string(static_cast<int>(s.classification));
        cd.data[prefix + "signalStrength"] = std::to_string(s.signalStrength);
        cd.data[prefix + "scanProgress"]   = std::to_string(s.scanProgress);
        cd.data[prefix + "distance"]       = std::to_string(s.distance);
        cd.data[prefix + "posX"]           = std::to_string(s.posX);
        cd.data[prefix + "posY"]           = std::to_string(s.posY);
        cd.data[prefix + "posZ"]           = std::to_string(s.posZ);
        cd.data[prefix + "isFullyScanned"] = s.isFullyScanned ? "1" : "0";
    }

    return cd;
}

void ScannerComponent::Deserialize(const ComponentData& data) {
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

    int typeVal = getInt("type", 0);
    constexpr int kMaxType = static_cast<int>(ScannerType::Military);
    if (typeVal >= 0 && typeVal <= kMaxType) {
        _type = static_cast<ScannerType>(typeVal);
    } else {
        _type = ScannerType::Passive;
    }

    _baseRange        = getFloat("baseRange", 5000.0f);
    _cooldownTimer    = getFloat("cooldownTimer", 0.0f);
    _cooldownDuration = getFloat("cooldownDuration", 5.0f);

    int scanCount = getInt("scanCount", 0);
    _scans.clear();
    _scans.reserve(static_cast<size_t>(scanCount));
    for (int i = 0; i < scanCount; ++i) {
        std::string prefix = "scan_" + std::to_string(i) + "_";
        ScanResult s;
        s.targetId       = getUint64(prefix + "targetId", 0);
        s.targetName     = getStr(prefix + "targetName");
        int clsVal       = getInt(prefix + "classification", 0);
        constexpr int kMaxCls = static_cast<int>(SignatureClass::Debris);
        if (clsVal >= 0 && clsVal <= kMaxCls) {
            s.classification = static_cast<SignatureClass>(clsVal);
        }
        s.signalStrength = getFloat(prefix + "signalStrength", 0.0f);
        s.scanProgress   = getFloat(prefix + "scanProgress", 0.0f);
        s.distance       = getFloat(prefix + "distance", 0.0f);
        s.posX           = getFloat(prefix + "posX", 0.0f);
        s.posY           = getFloat(prefix + "posY", 0.0f);
        s.posZ           = getFloat(prefix + "posZ", 0.0f);
        s.isFullyScanned = getInt(prefix + "isFullyScanned", 0) != 0;
        _scans.push_back(s);
    }
}

// ---------------------------------------------------------------------------
// ScanningSystem
// ---------------------------------------------------------------------------

ScanningSystem::ScanningSystem() : SystemBase("ScanningSystem") {}

ScanningSystem::ScanningSystem(EntityManager& entityManager)
    : SystemBase("ScanningSystem")
    , _entityManager(&entityManager)
{
}

void ScanningSystem::SetEntityManager(EntityManager* em) {
    _entityManager = em;
}

void ScanningSystem::Update(float deltaTime) {
    if (!_entityManager) return;

    auto scanners = _entityManager->GetAllComponents<ScannerComponent>();
    for (auto* scanner : scanners) {
        // Tick cooldown
        if (scanner->_cooldownTimer > 0.0f) {
            scanner->_cooldownTimer -= deltaTime;
            if (scanner->_cooldownTimer < 0.0f) scanner->_cooldownTimer = 0.0f;
        }

        float speedMult = scanner->GetScanSpeedMultiplier();
        float range = scanner->GetRange();

        for (auto& scan : scanner->_scans) {
            if (scan.isFullyScanned) continue;

            // Distance affects scan speed: closer = faster
            float distanceFactor = 1.0f;
            if (range > 0.0f && scan.distance > 0.0f) {
                distanceFactor = 1.0f - (scan.distance / range) * 0.5f;
                if (distanceFactor < 0.1f) distanceFactor = 0.1f;
            }

            // Base scan time: 10 seconds for a full scan at default speed
            float baseScanTime = 10.0f;
            float progressPerSecond = (speedMult * distanceFactor) / baseScanTime;

            scan.scanProgress += progressPerSecond * deltaTime;
            scan.signalStrength = scan.scanProgress;

            // Classify at 25% progress
            if (scan.scanProgress >= 0.25f && scan.classification == SignatureClass::Unknown) {
                // Simple classification based on target ID hash
                int cls = static_cast<int>(scan.targetId % 5) + 1;
                scan.classification = static_cast<SignatureClass>(cls);
            }

            if (scan.scanProgress >= 1.0f) {
                scan.scanProgress = 1.0f;
                scan.signalStrength = 1.0f;
                scan.isFullyScanned = true;
            }
        }
    }
}

} // namespace subspace
