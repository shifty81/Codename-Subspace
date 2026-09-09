#include "station/StationServiceSystem.h"

#include <algorithm>

namespace subspace {

static bool Enabled(const StationServiceProfile& p, StationServiceType t) {
    switch (t) {
    case StationServiceType::Refuel: return p.refuel;
    case StationServiceType::RepairHull: return p.repairHull;
    case StationServiceType::RepairModules: return p.repairModules;
    case StationServiceType::Refit: return p.refit;
    case StationServiceType::Storage: return p.storage;
    case StationServiceType::Market: return p.market;
    case StationServiceType::Shipyard: return p.shipyard;
    }
    return false;
}

bool StationServiceSystem::IsAvailable(const StationServiceProfile& profile, StationServiceType type, double factionStanding) const {
    return Enabled(profile, type) && factionStanding >= profile.minimumStanding;
}

double StationServiceSystem::BaseUnitPrice(StationServiceType type) {
    switch (type) {
    case StationServiceType::Refuel: return 2.0;
    case StationServiceType::RepairHull: return 7.5;
    case StationServiceType::RepairModules: return 12.0;
    case StationServiceType::Refit: return 25.0;
    case StationServiceType::Storage: return 1.0;
    case StationServiceType::Market: return 0.0;
    case StationServiceType::Shipyard: return 50.0;
    }
    return 0.0;
}

StationServiceQuote StationServiceSystem::Quote(const StationServiceProfile& profile, StationServiceType type,
                                                 double amount, double factionStanding) const {
    StationServiceQuote q; q.type = type; q.amount = std::max(0.0, amount);
    if (!Enabled(profile, type)) { q.reason = "service disabled"; return q; }
    if (factionStanding < profile.minimumStanding) { q.reason = "standing too low"; return q; }
    q.available = true;
    const double standingDiscount = std::clamp(1.0 - std::max(0.0, factionStanding) * 0.12, 0.75, 1.0);
    q.unitPrice = BaseUnitPrice(type) * std::max(0.1, profile.priceMultiplier) * standingDiscount;
    q.totalPrice = q.unitPrice * q.amount;
    return q;
}

} // namespace subspace
