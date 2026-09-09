#include "ships/ShipPartGeneration.h"

#include <algorithm>
#include <sstream>

namespace subspace {

static std::uint32_t NextRand(std::uint32_t& seed) {
    seed = seed * 1103515245u + 12345u;
    return seed;
}

static int Range(std::uint32_t& seed, int low, int high) {
    if (high <= low) { return low; }
    return low + static_cast<int>(NextRand(seed) % static_cast<std::uint32_t>(high - low + 1));
}

std::vector<ShipPartDefinition> GenerateShipPartVariants(const ShipPartGenerationRequest& request, int count) {
    std::vector<ShipPartDefinition> out;
    std::uint32_t seed = request.seed == 0 ? 1 : request.seed;
    const int minTier = std::max(1, request.minimumTier);
    const int maxTier = std::max(minTier, request.maximumTier);
    for (int i = 0; i < count; ++i) {
        ShipPartDefinition part;
        part.category = request.category;
        part.tier = Range(seed, minTier, maxTier);
        part.displayName = request.manufacturer + " T" + std::to_string(part.tier) + " " + ShipPartCategoryName(request.category) + " Mk" + std::to_string(i + 1);
        part.id = "generated." + request.manufacturer + "." + ShipPartCategoryName(request.category) + "." + std::to_string(part.tier) + "." + std::to_string(i + 1);
        part.installCostCredits = 250 * part.tier + Range(seed, 0, 150 * part.tier);
        part.mass = 4 * part.tier + Range(seed, 0, 5 * part.tier);
        part.powerDraw = part.tier + Range(seed, 0, part.tier * 2);
        part.thrustMultiplier = 1.0f;
        part.turnMultiplier = 1.0f;
        part.fuelBurnMultiplier = 1.0f;
        part.cargoMultiplier = 1.0f;
        part.miningMultiplier = 1.0f;
        if (part.category == ShipPartCategory::MainThruster) { part.thrustMultiplier += 0.16f * part.tier; part.fuelBurnMultiplier += 0.08f * part.tier; }
        if (part.category == ShipPartCategory::ManeuverThruster) { part.turnMultiplier += 0.18f * part.tier; }
        if (part.category == ShipPartCategory::Cargo) { part.cargoMultiplier += 0.22f * part.tier; }
        if (part.category == ShipPartCategory::MiningTool) { part.miningMultiplier += 0.20f * part.tier; }
        if (part.category == ShipPartCategory::Shield) { part.shieldBonus += 10.0f * part.tier; }
        if (request.includeExperimental && (NextRand(seed) % 5u) == 0u) {
            part.displayName += " Experimental";
            part.installCostCredits += 350 * part.tier;
            part.powerDraw += part.tier;
        }
        out.push_back(part);
    }
    return out;
}

std::string ShipPartVariantName(const ShipPartDefinition& part) {
    std::ostringstream ss;
    ss << part.displayName << " [" << ShipPartCategoryName(part.category) << " T" << part.tier << "]";
    return ss.str();
}

} // namespace subspace
