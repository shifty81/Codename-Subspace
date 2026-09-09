#pragma once
#include "hangar/DockingExperienceSystem.h"
#include "interior/ShipEmbodimentSystem.h"
#include <string>
#include <vector>
namespace subspace {
enum class ProductionContextKind { None, Ship, Planet, Station, Derelict, OrbitalHub, Asteroid, Site };
struct ProductionContextAction { std::string label; bool enabled=true; std::string disabledReason; };
struct ProductionHudModel {
    std::string modeLabel;
    std::string contextTitle;
    std::vector<std::string> contextActions;
    std::vector<ProductionContextAction> actions;
    std::vector<std::string> statusLines;
    std::vector<std::string> topBar;
    std::vector<std::string> bottomHints;
    bool showModuleRack = true;
    bool showScanner = true;
    bool compactDeveloperTelemetry = false;
};
class ProductionInterfaceSystem {
public:
    ProductionHudModel Build(ProductionContextKind contact, const std::string& id,
                             ShipEmbodimentMode embodiment,
                             DockingExperienceStage docking,
                             bool vectorTransit) const;
};
} // namespace subspace
