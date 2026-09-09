#pragma once

#include "content/ShipyardModuleSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"

#include <string>
#include <vector>

namespace subspace {

enum class ShipFunctionalCapability {
    Command,
    Power,
    MainPropulsion,
    Maneuvering,
    Navigation,
    Sensors,
    Communications,
    Thermal,
    Structure,
    FuelEnergyStorage,
    CrewControl,
    LifeSupport,
    UtilityAccess
};


struct ShipFunctionalAutofitChoice {
    ShipFunctionalCapability capability = ShipFunctionalCapability::Structure;
    std::string moduleId;
};

struct ShipFunctionalAutofitPlan {
    bool complete = false;
    std::vector<ShipFunctionalAutofitChoice> additions;
    std::vector<ShipFunctionalCapability> unresolved;
};

struct ShipFunctionalCoreReport {
    bool valid = false;
    std::vector<ShipFunctionalCapability> satisfied;
    std::vector<ShipFunctionalCapability> missing;
    std::vector<std::string> messages;
};

class ShipFunctionalCoreSystem {
public:
    static const char* Name(ShipFunctionalCapability capability);
    static std::vector<ShipFunctionalCapability> Required(bool biologicalCrew = true);
    static std::vector<ShipFunctionalCapability> CapabilitiesFor(const ShipyardModuleRecord& record);
    static ShipFunctionalCoreReport Validate(const std::vector<ShipyardModuleRecord>& catalog,
                                             const ProceduralShipVisualRecipe& recipe,
                                             bool biologicalCrew = true);
    // Pass635: PCG must plan at least one implementation of every required
    // functional capability before the hull can be certified.
    static ShipFunctionalAutofitPlan BuildAutofitPlan(const std::vector<ShipyardModuleRecord>& catalog,
                                                       const ProceduralShipVisualRecipe& recipe,
                                                       bool biologicalCrew = true);
};

} // namespace subspace
