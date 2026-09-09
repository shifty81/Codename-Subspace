#pragma once

#include "content/UniversalKitbashAuthority.h"
#include "ships/PropulsionRoleSystem.h"
#include "ships/ShipClassRoleSystem.h"
#include "ships/ShipFunctionalCoreSystem.h"
#include "ships/ShipSpatialAssemblySystem.h"

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

enum class ShipPcgRejectReason {
    None,
    SpatialConflict,
    CommandBuried,
    DetailDensity,
    PropulsionOrientation,
    ExhaustBlocked,
    MaterialReview,
    MissingFunctionalCore,
    ClassSizeMismatch,
    HullRoleMismatch
};

struct ExhaustClearanceVolume {
    std::size_t moduleIndex = 0;
    Vector3 origin{};
    Vector3 direction{0.0f,-1.0f,0.0f};
    float radius = 0.35f;
    float length = 4.0f;
    bool blocked = false;
    std::size_t blockingModuleIndex = static_cast<std::size_t>(-1);
};

struct PropulsionCatalogAuditEntry {
    std::string moduleId;
    PropulsionRole role = PropulsionRole::None;
    float confidence = 0.0f;
    bool knownOrientation = false;
    bool generatorEligible = false;
    bool requiresReview = false;
    std::string reason;
};

struct ShipGenerationCandidateReport {
    bool accepted = false;
    bool retrySuggested = false;
    ShipPcgRejectReason reason = ShipPcgRejectReason::None;
    ShipSpatialAssemblyReport spatial{};
    ShipFunctionalCoreReport functional{};
    std::vector<ExhaustClearanceVolume> exhaust;
    std::vector<std::string> messages;
};

struct ShipRoleFitPlan {
    ShipClass shipClass = ShipClass::Frigate;
    ShipRole role = ShipRole::GeneralCombat;
    UniversalSizeClass structuralSize = UniversalSizeClass::XS;
    std::string hullFamilyId;
    ShipRoleBudget budget{};
    std::vector<ShipFunctionalCapability> mandatoryCapabilities;
    std::vector<std::string> preferredModuleRoles;
    bool compatible = false;
};

struct HullFamilyRuntimeProfile {
    std::string factionId;
    std::string familyId;
    ShipClass shipClass = ShipClass::Frigate;
    UniversalSizeClass structuralSize = UniversalSizeClass::XS;
    std::string chassisStyle;
    float targetLengthMeters = 0.0f;
    float targetWidthMeters = 0.0f;
    float targetHeightMeters = 0.0f;
    float commandExposure = 0.25f;
    float propulsionReserve = 0.20f;
    float detailDensityBudget = 0.22f;
    std::vector<ShipRole> allowedRoles;
    std::vector<ShipRole> preferredRoles;
};

class ShipPcgRuntimeClosureSystem {
public:
    // Pass615-620: whole-ship candidate authority used before generated recipes
    // are accepted into the runtime catalog.
    static ShipGenerationCandidateReport EvaluateCandidate(const std::vector<ShipyardModuleRecord>& catalog,
                                                            const ProceduralShipVisualRecipe& recipe,
                                                            bool requireFunctionalCore = false,
                                                            bool biologicalCrew = true);
    static bool RepairSpatialCandidate(const std::vector<ShipyardModuleRecord>& catalog,
                                       ProceduralShipVisualRecipe& recipe,
                                       int maxIterations = 4);
    static bool RepairCandidate(const std::vector<ShipyardModuleRecord>& catalog,
                                ProceduralShipVisualRecipe& recipe,
                                int maxIterations = 6,
                                bool requireFunctionalCore = false,
                                bool biologicalCrew = true);

    // Pass621-624: propulsion is audited by semantic role + local axes. Unknown
    // orientation fails closed out of ordinary PCG while remaining manually usable.
    static std::vector<ExhaustClearanceVolume> BuildExhaustClearance(const std::vector<ShipyardModuleRecord>& catalog,
                                                                     const ProceduralShipVisualRecipe& recipe,
                                                                     float lengthMultiplier = 3.5f);
    static std::vector<PropulsionCatalogAuditEntry> AuditPropulsionCatalog(const std::vector<ShipyardModuleRecord>& catalog);

    // Pass625-631: material and size/class eligibility are explicit runtime gates.
    static bool MaterialPcgEligible(KitbashMaterialCertification state);
    static bool ModuleFitsClass(const ShipyardModuleRecord& module, ShipClass shipClass, bool auxiliary = true);
    static std::vector<std::size_t> FilterForClass(const std::vector<ShipyardModuleRecord>& catalog,
                                                   ShipClass shipClass,
                                                   bool auxiliary = true);

    // Pass632-638: faction hull family + role lineage is an actual generation
    // contract, not merely a display label.
    static HullFamilyRuntimeProfile BuildHullFamilyProfile(const FactionHullFamilyDefinition& family);
    static ShipRoleFitPlan BuildRoleFitPlan(const FactionHullFamilyDefinition& family, ShipRole role);
    static void ApplyLineage(ProceduralShipVisualRecipe& recipe,
                             const HullFamilyRuntimeProfile& family,
                             ShipRole role,
                             const std::string& exemplarId = {});
};

} // namespace subspace
