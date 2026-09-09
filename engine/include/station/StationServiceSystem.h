#pragma once

#include <string>

namespace subspace {

enum class StationServiceType { Refuel, RepairHull, RepairModules, Refit, Storage, Market, Shipyard };

struct StationServiceProfile {
    bool refuel = true;
    bool repairHull = true;
    bool repairModules = true;
    bool refit = true;
    bool storage = true;
    bool market = true;
    bool shipyard = false;
    double priceMultiplier = 1.0;
    double minimumStanding = -0.5;
};

struct StationServiceQuote {
    StationServiceType type = StationServiceType::Refuel;
    bool available = false;
    double amount = 0.0;
    double unitPrice = 0.0;
    double totalPrice = 0.0;
    std::string reason;
};

class StationServiceSystem {
public:
    StationServiceQuote Quote(const StationServiceProfile& profile, StationServiceType type,
                              double amount, double factionStanding) const;
    bool IsAvailable(const StationServiceProfile& profile, StationServiceType type, double factionStanding) const;
    static double BaseUnitPrice(StationServiceType type);
};

} // namespace subspace
