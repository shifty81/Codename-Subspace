#pragma once

#include "content/ShipyardModuleSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"
#include "ships/ShipClassRoleSystem.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

struct ShipyardPcgOverlayState {
    bool structuralSkeleton = true;
    bool occupancy = false;
    bool sockets = true;
    bool propulsion = true;
    bool exhaust = true;
    bool commandExposure = true;
    bool detailDensity = false;
    bool functionalCore = true;
};

struct ShipyardPcgStudioRequest {
    std::string factionId = "PLAYER";
    ShipClass shipClass = ShipClass::Frigate;
    int hullFamilyIndex = 1;
    std::string role = "INDUSTRIAL";
    std::uint32_t seed = 0x51A7D007u;
    bool preserveExemplarSilhouette = true;
};

struct ShipyardPcgDecision {
    std::string targetId;
    std::string action;
    std::string reason;
    bool accepted = false;
};

struct ShipyardPcgStudioReport {
    bool valid = false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
    std::vector<ShipyardPcgDecision> decisions;
};

struct ShipyardPcgStudioState {
    ShipyardPcgStudioRequest request{};
    ShipyardPcgOverlayState overlays{};
    ShipyardPcgStudioReport lastReport{};
    std::uint32_t rerollCounter = 0;
    bool liveRegenerate = false;
    bool teachFromCurrentAssembly = false;
    std::string status = "PCG Studio ready";
};

class ShipyardPcgStudioSystem {
public:
    static ShipyardPcgStudioReport AuditCandidate(const std::vector<ShipyardModuleRecord>& catalog,
                                                  const ProceduralShipVisualRecipe& recipe,
                                                  const ShipyardPcgStudioRequest& request);
    static std::uint32_t RerollSeed(ShipyardPcgStudioState& state);
    static std::vector<std::string> EnabledOverlayNames(const ShipyardPcgOverlayState& overlays);
    static std::string ExplainRequest(const ShipyardPcgStudioRequest& request);
};

} // namespace subspace
