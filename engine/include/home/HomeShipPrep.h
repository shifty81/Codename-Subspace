#pragma once

#include "home/HomeFactoryNetwork.h"
#include "ships/ShipPartGeneration.h"

#include <string>
#include <vector>

namespace subspace {

struct HomeShipPrepSlot {
    std::string slotId;
    std::string installedPartId;
    std::string displayName;
    bool required = false;
};

struct HomeShipPrepState {
    std::vector<HomeShipPrepSlot> slots;
    std::vector<ShipPartDefinition> availableParts;
    int fuelLoaded = 0;
    int cargoReserved = 0;
    int defenseRating = 0;
    int scannerRating = 0;
};

struct HomeShipPrepValidation {
    bool canLaunch = false;
    std::vector<std::string> warnings;
    std::vector<std::string> missingRequiredSlots;
    int estimatedFuelRange = 0;
};

HomeShipPrepState CreateStarterHomeShipPrepState();
HomeShipPrepValidation ValidateHomeShipPrepForAdventure(const HomeShipPrepState& prep,
                                                        const HomeFactoryNetworkState& inventory);
std::string HomeShipPrepSummary(const HomeShipPrepState& prep);
std::string HomeShipPrepValidationSummary(const HomeShipPrepValidation& validation);

} // namespace subspace
