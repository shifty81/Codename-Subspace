#pragma once

#include "core/ecs/Entity.h"
#include "core/ecs/IComponent.h"
#include "core/ecs/SystemBase.h"
#include "core/ecs/EntityManager.h"
#include "core/persistence/SaveGameManager.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace subspace {

/// Type of scanner equipped on an entity.
enum class ScannerType {
    Passive,
    Active,
    Deep,
    Military
};

/// Current state of a scan operation.
enum class ScanState {
    Idle,
    Scanning,
    Analyzing,
    Complete,
    Jammed
};

/// Classification of a detected signature.
enum class SignatureClass {
    Unknown,
    Ship,
    Station,
    Asteroid,
    Anomaly,
    Debris
};

/// Result of scanning a single target.
struct ScanResult {
    uint64_t targetId = 0;
    std::string targetName;
    SignatureClass classification = SignatureClass::Unknown;
    float signalStrength = 0.0f;   ///< 0 to 1
    float scanProgress = 0.0f;     ///< 0 to 1
    float distance = 0.0f;
    float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
    bool isFullyScanned = false;

    /// Get the display name for a scanner type.
    static std::string GetTypeName(ScannerType type);

    /// Get the display name for a scan state.
    static std::string GetStateName(ScanState state);

    /// Get the display name for a signature class.
    static std::string GetClassName(SignatureClass cls);
};

/// ECS component that gives an entity scanning capabilities.
class ScannerComponent : public IComponent {
public:
    explicit ScannerComponent(ScannerType type = ScannerType::Passive,
                              float range = 5000.0f);

    ScannerType GetType() const;
    void SetType(ScannerType type);

    float GetRange() const;
    void SetRange(float range);

    float GetResolution() const;

    int GetMaxConcurrentScans() const;
    int GetActiveScanCount() const;

    /// Start scanning a target. Returns false if at scan limit or on cooldown.
    bool StartScan(uint64_t targetId, const std::string& targetName,
                   float distance, float posX = 0.0f, float posY = 0.0f,
                   float posZ = 0.0f);

    /// Cancel a scan by target ID. Returns false if not found.
    bool CancelScan(uint64_t targetId);

    /// Get the scan result for a specific target. Returns nullptr if not found.
    const ScanResult* GetScanResult(uint64_t targetId) const;

    /// Get all active and completed scan results.
    const std::vector<ScanResult>& GetAllScans() const;

    /// Get the number of completed scans.
    int GetCompletedScanCount() const;

    /// Clear all completed scans from the result list.
    void ClearCompletedScans();

    /// Check if the scanner is on cooldown.
    bool IsOnCooldown() const;

    /// Get remaining cooldown time in seconds.
    float GetCooldownRemaining() const;

    /// Type-based scan speed multiplier (Passive 1.0 … Military 2.0).
    float GetScanSpeedMultiplier() const;

    /// Type-based range multiplier (Passive 1.0 … Military 1.8).
    float GetRangeMultiplier() const;

    /// Serialize for save-game persistence.
    ComponentData Serialize() const;

    /// Restore from previously serialized data.
    void Deserialize(const ComponentData& data);

private:
    ScannerType _type = ScannerType::Passive;
    float _baseRange = 5000.0f;
    float _cooldownTimer = 0.0f;
    float _cooldownDuration = 5.0f;
    std::vector<ScanResult> _scans;

    friend class ScanningSystem;
};

/// System that updates scan operations each frame.
class ScanningSystem : public SystemBase {
public:
    ScanningSystem();
    explicit ScanningSystem(EntityManager& entityManager);

    void Update(float deltaTime) override;

    void SetEntityManager(EntityManager* em);

private:
    EntityManager* _entityManager = nullptr;
};

} // namespace subspace
