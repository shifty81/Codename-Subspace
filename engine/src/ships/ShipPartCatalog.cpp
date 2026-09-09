#include "ships/ShipPartCatalog.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace subspace {

const char* ShipPartCategoryName(ShipPartCategory category)
{
    switch (category) {
    case ShipPartCategory::Hull: return "Hull";
    case ShipPartCategory::Cockpit: return "Cockpit";
    case ShipPartCategory::Reactor: return "Reactor";
    case ShipPartCategory::MainThruster: return "Main Thruster";
    case ShipPartCategory::ManeuverThruster: return "Maneuver Thruster";
    case ShipPartCategory::MiningTool: return "Mining Tool";
    case ShipPartCategory::Weapon: return "Weapon";
    case ShipPartCategory::Shield: return "Shield";
    case ShipPartCategory::Cargo: return "Cargo";
    case ShipPartCategory::Scanner: return "Scanner";
    case ShipPartCategory::Utility: return "Utility";
    }
    return "Unknown";
}

std::vector<ShipPartDefinition> CreateStarterShipPartCatalog()
{
    return {
        {"hull-light-frame", "Light Salvage Frame", ShipPartCategory::Hull, 1, 0, 6, 0, 1.08f, 1.05f, 0.96f, 0.90f, 1.0f, 0.0f},
        {"hull-industrial-frame", "Industrial Cargo Frame", ShipPartCategory::Hull, 2, 650, 16, 0, 0.92f, 0.86f, 1.08f, 1.35f, 1.0f, 0.0f},
        {"cockpit-scout", "Scout Cockpit", ShipPartCategory::Cockpit, 1, 0, 2, 1, 1.02f, 1.05f, 1.0f, 1.0f, 1.0f, 0.0f},
        {"reactor-scrapcell", "Scrapcell Reactor", ShipPartCategory::Reactor, 1, 0, 4, -8, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f},
        {"reactor-compact-fusion", "Compact Fusion Reactor", ShipPartCategory::Reactor, 2, 900, 6, -18, 1.04f, 1.02f, 0.94f, 1.0f, 1.0f, 0.0f},
        {"engine-chemical-bell", "Chemical Bell Thruster", ShipPartCategory::MainThruster, 1, 0, 5, 4, 1.00f, 1.0f, 1.00f, 1.0f, 1.0f, 0.0f},
        {"engine-ion-spool", "Ion Spool Thruster", ShipPartCategory::MainThruster, 2, 720, 4, 6, 1.18f, 1.0f, 0.82f, 1.0f, 1.0f, 0.0f},
        {"engine-heavy-hauler", "Heavy Hauler Thruster", ShipPartCategory::MainThruster, 2, 560, 9, 7, 1.35f, 0.92f, 1.18f, 1.0f, 1.0f, 0.0f},
        {"rcs-coldgas", "Cold-Gas RCS Quads", ShipPartCategory::ManeuverThruster, 1, 0, 2, 2, 1.0f, 1.00f, 1.00f, 1.0f, 1.0f, 0.0f},
        {"rcs-vector-grid", "Vector RCS Grid", ShipPartCategory::ManeuverThruster, 2, 520, 4, 5, 1.0f, 1.28f, 0.92f, 1.0f, 1.0f, 0.0f},
        {"tool-mining-laser-i", "Mining Laser I", ShipPartCategory::MiningTool, 1, 0, 3, 5, 1.0f, 1.0f, 1.0f, 1.0f, 1.00f, 0.0f},
        {"tool-salvage-cutter", "Salvage Cutter", ShipPartCategory::MiningTool, 2, 680, 5, 7, 1.0f, 1.0f, 1.02f, 1.0f, 1.25f, 0.0f},
        {"shield-patch-bubble", "Patchwork Shield Bubble", ShipPartCategory::Shield, 1, 320, 4, 8, 0.98f, 0.98f, 1.08f, 1.0f, 1.0f, 12.0f},
        {"cargo-net-rack", "Cargo Net Rack", ShipPartCategory::Cargo, 1, 0, 4, 0, 0.98f, 0.98f, 1.02f, 1.15f, 1.0f, 0.0f},
        {"cargo-compressed-bay", "Compressed Cargo Bay", ShipPartCategory::Cargo, 2, 700, 9, 2, 0.92f, 0.92f, 1.06f, 1.80f, 1.0f, 0.0f},
        {"scanner-basic-sweep", "Basic Sweep Scanner", ShipPartCategory::Scanner, 1, 0, 2, 2, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f},
        {"scanner-deep-field", "Deep Field Scanner", ShipPartCategory::Scanner, 2, 840, 4, 6, 1.0f, 1.0f, 1.04f, 1.0f, 1.0f, 0.0f},
    };
}

ShipLoadout CreateStarterShipLoadout()
{
    ShipLoadout loadout;
    loadout.slots = {
        {ShipPartCategory::Hull, "hull-light-frame"},
        {ShipPartCategory::Cockpit, "cockpit-scout"},
        {ShipPartCategory::Reactor, "reactor-scrapcell"},
        {ShipPartCategory::MainThruster, "engine-chemical-bell"},
        {ShipPartCategory::ManeuverThruster, "rcs-coldgas"},
        {ShipPartCategory::MiningTool, "tool-mining-laser-i"},
        {ShipPartCategory::Cargo, "cargo-net-rack"},
        {ShipPartCategory::Scanner, "scanner-basic-sweep"},
    };
    return loadout;
}

const ShipPartDefinition* FindShipPart(const std::vector<ShipPartDefinition>& catalog, const std::string& partId)
{
    auto it = std::find_if(catalog.begin(), catalog.end(), [&](const ShipPartDefinition& part) { return part.id == partId; });
    return it == catalog.end() ? nullptr : &(*it);
}

std::vector<ShipPartDefinition> FilterShipPartsByCategory(const std::vector<ShipPartDefinition>& catalog, ShipPartCategory category)
{
    std::vector<ShipPartDefinition> result;
    for (const auto& part : catalog) {
        if (part.category == category) {
            result.push_back(part);
        }
    }
    return result;
}

ShipPartStats CalculateShipPartStats(const ShipLoadout& loadout, const std::vector<ShipPartDefinition>& catalog)
{
    ShipPartStats stats;
    float thrustMult = 1.0f;
    float turnMult = 1.0f;
    float fuelMult = 1.0f;
    float cargoMult = 1.0f;
    float miningMult = 1.0f;
    for (const auto& slot : loadout.slots) {
        if (const auto* part = FindShipPart(catalog, slot.partId)) {
            thrustMult *= part->thrustMultiplier;
            turnMult *= part->turnMultiplier;
            fuelMult *= part->fuelBurnMultiplier;
            cargoMult *= part->cargoMultiplier;
            miningMult *= part->miningMultiplier;
            stats.shieldBonus += part->shieldBonus;
            if (part->category == ShipPartCategory::Shield) {
                stats.shieldCapacity += part->shieldBonus;
            }
            if (part->category == ShipPartCategory::Scanner) {
                stats.scannerRange = std::max(stats.scannerRange, 1.0f + static_cast<float>(part->tier));
            }
            stats.mass += part->mass;
            stats.powerDraw += part->powerDraw;
        }
    }
    const float massPenalty = std::max(0.72f, 1.0f - static_cast<float>(std::max(0, stats.mass - 25)) * 0.008f);
    stats.thrust *= thrustMult * massPenalty;
    stats.reverseThrust *= thrustMult * massPenalty;
    stats.strafeThrust *= std::sqrt(std::max(0.25f, thrustMult)) * massPenalty;
    stats.turnSpeed *= turnMult * massPenalty;
    stats.maxSpeed *= std::max(0.75f, thrustMult * 0.95f);
    stats.fuelBurnPerSecond *= std::max(0.25f, fuelMult);
    stats.cargoCapacity = std::max(8, static_cast<int>(stats.cargoCapacity * cargoMult));
    stats.miningEfficiency *= miningMult;
    return stats;
}

ShipPartInstallResult InstallShipPart(ShipLoadout& loadout,
                                      const ShipPartDefinition& part,
                                      int availableCredits,
                                      bool atHome,
                                      bool expeditionActive)
{
    ShipPartInstallResult result;
    result.costCredits = part.installCostCredits;
    if (!atHome) {
        result.message = "Ship parts can only be hot-swapped at the Home Solar System shipyard.";
        return result;
    }
    if (expeditionActive && part.expeditionLocked) {
        result.message = "Cannot install this ship part while an expedition is active.";
        return result;
    }
    if (!part.hotSwappableAtHome) {
        result.message = "Part is not hot-swappable in the current shipyard tier.";
        return result;
    }
    if (availableCredits < part.installCostCredits) {
        result.message = "Not enough credits for " + part.displayName + ".";
        return result;
    }
    auto it = std::find_if(loadout.slots.begin(), loadout.slots.end(), [&](const ShipLoadoutSlot& slot) {
        return slot.category == part.category;
    });
    if (it == loadout.slots.end()) {
        loadout.slots.push_back({part.category, part.id});
    }
    else {
        it->partId = part.id;
    }
    result.success = true;
    result.message = "Installed " + part.displayName + ".";
    return result;
}

std::string ShipLoadoutSummary(const ShipLoadout& loadout, const std::vector<ShipPartDefinition>& catalog)
{
    std::ostringstream out;
    out << loadout.displayName << ": ";
    bool first = true;
    for (const auto& slot : loadout.slots) {
        if (!first) {
            out << " | ";
        }
        first = false;
        out << ShipPartCategoryName(slot.category) << "=";
        if (const auto* part = FindShipPart(catalog, slot.partId)) {
            out << part->displayName;
        }
        else {
            out << slot.partId;
        }
    }
    return out.str();
}

} // namespace subspace
