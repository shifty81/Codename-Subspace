#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class ShipyardAuthoringPrimaryClass : uint8_t {
    Unknown, Hull, Surface, Propulsion, Command, Weapon, Electronics, Utility, Adapter, Detail
};

enum class ShipyardAuthoringSubtype : uint16_t {
    Unknown,
    HullBow, HullMid, HullStern, HullSpine, HullSide, HullExtension, StructuralSupport,
    LateralWing, Winglet, HorizontalStabilizer, VerticalStabilizer, DorsalFin, VentralFin, Keel,
    MainEngine, SecondaryEngine, ManeuverThruster, RetroThruster, EngineHousing, EngineNacelle,
    EngineSupport, EngineStrut, EnginePylon, EngineNozzle, ThrustVane,
    Cockpit, Bridge, CompactBridge, ForwardBridge, SideBridge, CommandSuperstructure,
    WeaponMount, TurretBase, Turret, PointDefense, Launcher,
    SensorArray, SensorMast, Radar, Dish, Antenna, Communications,
    Cargo, Tank, Storage, Hangar, Docking, Industrial, Mining, Salvage, Drone, Service,
    HullAdapter, SizeTransition, StructuralCoupler,
    Panel, Vent, Intake, Grille, Trim, Greeble, Window
};

enum class ShipyardStructuralImportance : uint8_t {
    Core, RequiredFunctional, Structural, OptionalFunctional, Decorative
};

enum class ShipyardCertificationState : uint8_t {
    Unreviewed, Inferred, Reviewed, Certified, Quarantined
};

enum class ShipyardOrientationSource : uint8_t {
    Inferred, Authored, Certified, ManualOverride
};

enum class ShipyardPairMode : uint8_t {
    Independent, MirroredIdentical, MirroredCompatible, IntentionalAsymmetric, BrokenPair
};

enum class ShipyardMaterialZone : uint8_t {
    Unmapped, PrimaryPaint, SecondaryPaint, AccentPaint, StructuralDark,
    MetalLight, MetalDark, Glass, EmissionPrimary, EmissionSecondary, Hazard, Interior
};

enum class ShipyardValidationSeverity : uint8_t { Info, Warning, Error };
enum class ShipyardRerollMode : uint8_t { Similar, SameFamily, SameRole, AnyCompatible, Pair, Branch, AllUnlocked };
enum class ShipyardRepairAction : uint8_t {
    Keep, CorrectOrientation, AlternateSameParentSocket, AlternateNearbyParent, SameRoleReplacement, Unresolved
};

struct ShipyardVec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct ShipyardAttachmentFrame {
    ShipyardVec3 rootNormal{-1.0f, 0.0f, 0.0f};
    ShipyardVec3 forward{0.0f, 1.0f, 0.0f};
    ShipyardVec3 up{0.0f, 0.0f, 1.0f};
    ShipyardVec3 outward{1.0f, 0.0f, 0.0f};
    ShipyardOrientationSource source = ShipyardOrientationSource::Inferred;
    bool lateralSurface = false;
    float preferredMaxRollDegrees = 35.0f;
};

struct ShipyardAuthoringDefinition {
    std::string definitionId;
    std::string displayName;
    ShipyardAuthoringPrimaryClass primaryClass = ShipyardAuthoringPrimaryClass::Unknown;
    ShipyardAuthoringSubtype subtype = ShipyardAuthoringSubtype::Unknown;
    std::string family;
    std::string sizeClass = "M";
    std::string functionalRole;
    std::string placementZone;
    std::string rerollGroup;
    ShipyardStructuralImportance importance = ShipyardStructuralImportance::Structural;
    ShipyardCertificationState certification = ShipyardCertificationState::Unreviewed;
    ShipyardAttachmentFrame frame;
    std::vector<std::string> capabilities;
    std::vector<std::string> compatibleSocketTypes;
    std::unordered_map<std::string, ShipyardMaterialZone> materialZones;
    float generationWeight = 1.0f;
    bool generationEnabled = true;
};

struct ShipyardAuthoringSocket {
    std::string socketId;
    std::string socketType;
    std::string sizeClass = "M";
    ShipyardVec3 position;
    ShipyardVec3 normal{1.0f, 0.0f, 0.0f};
    ShipyardVec3 up{0.0f, 0.0f, 1.0f};
    std::vector<std::string> allowedRoles;
    std::vector<std::string> preferredRoles;
    float clearanceRadius = 0.0f;
    bool legacyGrandfathered = false;
};

struct ShipyardPlacementEvidence {
    float socketCompatibility = 1.0f;
    float rootFacesParent = 1.0f;
    float outwardFromCenterline = 1.0f;
    float forwardAlignment = 1.0f;
    float upAlignment = 1.0f;
    float roleLocation = 1.0f;
    float clearance = 1.0f;
    float pairing = 1.0f;
    float structuralIntersectionRatio = 0.0f;
    bool disconnected = false;
    bool exhaustFullyBlocked = false;
    bool hangarMouthFullyBlocked = false;
    bool bridgeBuried = false;
};

struct ShipyardPlacementScore {
    float socket = 1.0f;
    float orientation = 1.0f;
    float role = 1.0f;
    float clearance = 1.0f;
    float pairing = 1.0f;
    float confidence = 1.0f;
    bool hardInvalid = false;
    std::vector<std::string> reasons;
};

struct ShipyardSlotLocks {
    bool module = false;
    bool socket = false;
    bool transform = false;
    bool children = false;
};

struct ShipyardSlotTransform {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float yawDegrees = 0.0f;
    float pitchDegrees = 0.0f;
    float rollDegrees = 0.0f;
};

struct ShipyardSlot {
    std::string slotId;
    uint32_t sourcePlacementIndex = 0;
    std::string role;
    std::string moduleDefinitionId;
    std::string parentSlotId;
    std::string parentSocketId;
    ShipyardSlotTransform transform;
    std::string mirrorPartnerSlotId;
    ShipyardPairMode pairMode = ShipyardPairMode::Independent;
    ShipyardSlotLocks locks;
    uint32_t rerollIndex = 0;
    std::vector<std::string> rerollHistory;
};

struct ShipyardLivery {
    std::string preset = "Custom";
    std::string primary = "#29364A";
    std::string secondary = "#242931";
    std::string accent = "#C57A2A";
    std::string emission = "#64D8FF";
};

struct ShipyardResolvedBlueprint {
    uint64_t shipSeed = 0;
    uint32_t generatorVersion = 1;
    std::string blueprintId;
    ShipyardLivery livery;
    std::vector<ShipyardSlot> slots;
};

struct ShipyardValidationIssue {
    ShipyardValidationSeverity severity = ShipyardValidationSeverity::Info;
    std::string slotId;
    std::string moduleDefinitionId;
    std::string code;
    std::string message;
};

struct ShipyardRerollCandidate {
    std::string definitionId;
    float score = 0.0f;
};

struct ShipyardRerollPreview {
    bool valid = false;
    std::string slotId;
    std::string previousDefinitionId;
    std::string candidateDefinitionId;
    uint32_t rerollIndex = 0;
    float candidateScore = 0.0f;
    std::string explanation;
};

class ShipyardSelectionAuthority {
public:
    bool SelectSlot(const ShipyardResolvedBlueprint& blueprint, const std::string& slotId);
    void Clear();
    const std::string& SelectedSlotId() const { return m_selectedSlotId; }
    const std::string& SelectedDefinitionId() const { return m_selectedDefinitionId; }
    uint64_t Generation() const { return m_generation; }

private:
    std::string m_selectedSlotId;
    std::string m_selectedDefinitionId;
    uint64_t m_generation = 0;
};

class ShipyardAuthoringAuthority {
public:
    void RegisterDefinition(ShipyardAuthoringDefinition definition);
    bool HasDefinition(const std::string& definitionId) const;
    const ShipyardAuthoringDefinition* FindDefinition(const std::string& definitionId) const;

    ShipyardAuthoringDefinition InferDefinition(const std::string& definitionId, const std::string& displayName = {}) const;
    void RegisterBuiltInCertifiedOverrides();

    ShipyardPlacementScore ScorePlacement(const ShipyardAuthoringDefinition& definition,
                                          const ShipyardPlacementEvidence& evidence) const;
    ShipyardRepairAction RecommendRepair(const ShipyardPlacementScore& score,
                                         bool sameModuleCanRotate,
                                         bool sameParentHasAlternateSocket,
                                         bool nearbyParentAvailable,
                                         bool sameRoleReplacementAvailable) const;

    std::vector<ShipyardRerollCandidate> BuildRerollPool(const ShipyardResolvedBlueprint& blueprint,
                                                         const ShipyardSlot& slot,
                                                         ShipyardRerollMode mode) const;
    ShipyardRerollPreview PreviewReroll(const ShipyardResolvedBlueprint& blueprint,
                                        const std::string& slotId,
                                        ShipyardRerollMode mode) const;
    ShipyardRerollPreview PreviewReplacement(const ShipyardResolvedBlueprint& blueprint,
                                             const std::string& slotId,
                                             const std::string& candidateDefinitionId) const;
    std::vector<ShipyardRerollPreview> PreviewRerollOperation(const ShipyardResolvedBlueprint& blueprint,
                                                              const std::string& slotId,
                                                              ShipyardRerollMode mode) const;
    bool CommitReroll(ShipyardResolvedBlueprint& blueprint, const ShipyardRerollPreview& preview) const;
    bool CommitRerollOperation(ShipyardResolvedBlueprint& blueprint,
                               const std::vector<ShipyardRerollPreview>& previews) const;
    bool RestorePreviousReroll(ShipyardResolvedBlueprint& blueprint, const std::string& slotId) const;

    bool LockSlot(ShipyardResolvedBlueprint& blueprint, const std::string& slotId, ShipyardSlotLocks locks) const;
    bool PairSlots(ShipyardResolvedBlueprint& blueprint,
                   const std::string& firstSlotId,
                   const std::string& secondSlotId,
                   ShipyardPairMode mode) const;

    std::vector<ShipyardValidationIssue> ValidateBlueprint(const ShipyardResolvedBlueprint& blueprint) const;
    std::vector<ShipyardValidationIssue> ValidatePlacement(const ShipyardSlot& slot,
                                                           const ShipyardPlacementScore& score) const;

    static ShipyardMaterialZone InferMaterialZone(const std::string& sourceMaterialName);
    static std::string PrimaryClassName(ShipyardAuthoringPrimaryClass value);
    static std::string SubtypeName(ShipyardAuthoringSubtype value);
    static std::string MaterialZoneName(ShipyardMaterialZone value);

private:
    std::unordered_map<std::string, ShipyardAuthoringDefinition> m_definitions;

    static uint64_t StableHash(const std::string& value);
    static uint64_t Mix(uint64_t value);
    static std::string Lower(std::string value);
    static float Clamp01(float value);
    static bool ContainsCapability(const ShipyardAuthoringDefinition& definition, const std::string& capability);
    static ShipyardSlot* FindSlot(ShipyardResolvedBlueprint& blueprint, const std::string& slotId);
    static const ShipyardSlot* FindSlot(const ShipyardResolvedBlueprint& blueprint, const std::string& slotId);
};

} // namespace subspace
