#include "migration/CppConversionBacklog.h"

#include <sstream>

namespace subspace {

const char* ConversionDispositionName(ConversionDisposition disposition) {
    switch (disposition) {
        case ConversionDisposition::NeedsPort: return "NEEDS_PORT";
        case ConversionDisposition::PartiallyPorted: return "PARTIALLY_PORTED";
        case ConversionDisposition::PortedToCpp: return "PORTED_TO_CPP";
        case ConversionDisposition::ReplacedByCppDesign: return "REPLACED_BY_CPP_DESIGN";
        case ConversionDisposition::ReferenceOnly: return "REFERENCE_ONLY";
        case ConversionDisposition::Deferred: return "DEFERRED";
        default: return "UNKNOWN";
    }
}

std::vector<ConversionBacklogItem> BuildHighLevelCppConversionBacklog() {
    return {
        {"AvorionLike/Program.cs", "runtime/client/roguelite", ConversionDisposition::NeedsPort, "split behavior into runtime/session/client; do not port monolith"},
        {"AvorionLike/Core/UI", "ui/developer/ui/client views", ConversionDisposition::NeedsPort, "port HUD concepts into C++ view models"},
        {"AvorionLike/Core/Voxel", "ships/ship_editor", ConversionDisposition::PartiallyPorted, "block/stats foundations exist; builder parity remains"},
        {"AvorionLike/Core/Modular", "ships/home builder bay", ConversionDisposition::PartiallyPorted, "module library/factory exists; hot-swap build flow is ongoing"},
        {"AvorionLike/Core/Mining", "mining/salvage/resources", ConversionDisposition::NeedsPort, "resource extraction should feed expedition/home loop"},
        {"AvorionLike/Core/Economy", "trading/stations/home logistics", ConversionDisposition::NeedsPort, "station economy and home production need parity"},
        {"AvorionLike/Core/Faction", "factions/encounters", ConversionDisposition::NeedsPort, "faction spawn/reputation and route risk"},
        {"AvorionLike/Core/Procedural", "procedural/celestial/travel", ConversionDisposition::NeedsPort, "sector, ship, derelict, route generators"},
        {"AvorionLike/Core/Persistence", "core/persistence", ConversionDisposition::NeedsPort, "home save, run save, ship save"},
        {"AvorionLike/Examples", "docs/archive/examples", ConversionDisposition::ReferenceOnly, "examples after behavior is ported"}
    };
}

std::vector<ConversionBacklogItem> FilterConversionBacklog(const std::vector<ConversionBacklogItem>& items,
                                                           ConversionDisposition disposition) {
    std::vector<ConversionBacklogItem> out;
    for (const auto& item : items) {
        if (item.disposition == disposition) { out.push_back(item); }
    }
    return out;
}

std::string CppConversionBacklogSummary(const std::vector<ConversionBacklogItem>& items) {
    int needs = 0, partial = 0, done = 0;
    for (const auto& item : items) {
        if (item.disposition == ConversionDisposition::NeedsPort) { ++needs; }
        if (item.disposition == ConversionDisposition::PartiallyPorted) { ++partial; }
        if (item.disposition == ConversionDisposition::PortedToCpp || item.disposition == ConversionDisposition::ReplacedByCppDesign) { ++done; }
    }
    std::ostringstream ss;
    ss << "conversion items=" << items.size() << " needs=" << needs << " partial=" << partial << " done=" << done;
    return ss.str();
}

} // namespace subspace
