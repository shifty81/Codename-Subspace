#pragma once

#include "ships/ShipPartCatalog.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

struct ShipPartGenerationRequest {
    std::string manufacturer = "Yard Standard";
    ShipPartCategory category = ShipPartCategory::Utility;
    int minimumTier = 1;
    int maximumTier = 3;
    std::uint32_t seed = 1;
    bool includeExperimental = false;
};

std::vector<ShipPartDefinition> GenerateShipPartVariants(const ShipPartGenerationRequest& request, int count);
std::string ShipPartVariantName(const ShipPartDefinition& part);

} // namespace subspace
