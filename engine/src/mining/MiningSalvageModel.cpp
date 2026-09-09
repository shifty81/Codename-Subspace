#include "mining/MiningSalvageModel.h"

#include "celestial/CelestialSystemGenerator.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace subspace {

MiningYieldResult GenerateMiningYield(const AsteroidFieldProfile& field,
                                      const MiningTargetProfile& target,
                                      const MiningToolProfile& tool,
                                      std::uint32_t seed)
{
    MiningYieldResult result;
    result.damageApplied = tool.baseDamage * std::max(0.1f, tool.efficiency) * (0.92f + CelestialSeededUnit(seed, 4) * 0.18f);
    result.fractured = target.remainingIntegrity <= result.damageApplied;

    if (!result.fractured) {
        result.message = "Mining beam scored " + target.resourceTag + " deposit.";
        return result;
    }

    const int baseUnits = std::max(1, static_cast<int>(std::round(target.radius / 24.0f)));
    const int richnessBonus = std::max(0, target.richness + field.resourceRichness - 2);
    const int units = std::max(1, baseUnits + richnessBonus / 2);
    const std::string commodity = target.resourceTag.empty() ? field.dominantResource : target.resourceTag;
    const int unitValue = 12 + field.resourceRichness * 4 + (commodity.find("rare") != std::string::npos ? 28 : 0);
    result.cargo.push_back({commodity, units, unitValue * units});

    const float salvageRoll = CelestialSeededUnit(seed, 12);
    if (tool.canSalvage || target.derelict || salvageRoll < field.salvageChance) {
        const int salvageUnits = 1 + static_cast<int>(CelestialSeededUnit(seed, 14) * 3.0f);
        result.cargo.push_back({"salvage", salvageUnits, salvageUnits * (18 + field.resourceRichness * 2)});
    }

    result.message = "Asteroid fractured: " + MiningYieldSummary(result);
    return result;
}

std::string MiningYieldSummary(const MiningYieldResult& result)
{
    std::ostringstream stream;
    for (std::size_t i = 0; i < result.cargo.size(); ++i) {
        if (i > 0) {
            stream << ", ";
        }
        stream << result.cargo[i].units << "x " << result.cargo[i].commodity;
    }
    if (result.cargo.empty()) {
        stream << "no cargo";
    }
    return stream.str();
}

int EstimateCargoCreditValue(const std::vector<CargoYieldItem>& cargo)
{
    int total = 0;
    for (const auto& item : cargo) {
        total += item.creditValue;
    }
    return total;
}

} // namespace subspace
