#include "home/HomeShipPrep.h"
#include "ships/ShipPartCatalog.h"

#include <sstream>

namespace subspace {

HomeShipPrepState CreateStarterHomeShipPrepState() {
    HomeShipPrepState prep;
    prep.slots.push_back({"core", "starter-core", "Reactor Core", true});
    prep.slots.push_back({"engine", "starter-engine", "Main Engine", true});
    prep.slots.push_back({"rcs", "starter-rcs", "Directional Thrusters", true});
    prep.slots.push_back({"scanner", "starter-scanner", "Scanner", false});
    prep.fuelLoaded = 100;
    prep.cargoReserved = 8;
    prep.defenseRating = 1;
    prep.scannerRating = 1;
    prep.availableParts = CreateStarterShipPartCatalog();
    return prep;
}

HomeShipPrepValidation ValidateHomeShipPrepForAdventure(const HomeShipPrepState& prep,
                                                        const HomeFactoryNetworkState& inventory) {
    HomeShipPrepValidation validation;
    for (const auto& slot : prep.slots) {
        if (slot.required && slot.installedPartId.empty()) {
            validation.missingRequiredSlots.push_back(slot.slotId);
        }
    }
    if (prep.fuelLoaded < 40) validation.warnings.push_back("Fuel is below recommended expedition minimum.");
    if (prep.cargoReserved < 4) validation.warnings.push_back("Cargo reserve is low for salvage recovery.");
    if (GetHomeInventoryUnits(inventory, "hull-plate") < 1) validation.warnings.push_back("No spare hull plate packed for field repair.");
    validation.estimatedFuelRange = prep.fuelLoaded * 2;
    validation.canLaunch = validation.missingRequiredSlots.empty() && prep.fuelLoaded > 0;
    return validation;
}

std::string HomeShipPrepSummary(const HomeShipPrepState& prep) {
    std::ostringstream out;
    out << "shipPrep slots=" << prep.slots.size()
        << " parts=" << prep.availableParts.size()
        << " fuel=" << prep.fuelLoaded
        << " cargoReserve=" << prep.cargoReserved
        << " defense=" << prep.defenseRating
        << " scanner=" << prep.scannerRating;
    return out.str();
}

std::string HomeShipPrepValidationSummary(const HomeShipPrepValidation& validation) {
    std::ostringstream out;
    out << "shipPrepValidation canLaunch=" << (validation.canLaunch ? "yes" : "no")
        << " range=" << validation.estimatedFuelRange
        << " warnings=" << validation.warnings.size()
        << " missing=" << validation.missingRequiredSlots.size();
    return out.str();
}

} // namespace subspace
