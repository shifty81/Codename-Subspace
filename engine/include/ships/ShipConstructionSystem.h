#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class ConstructionMaterial { StructuralSteel, TitaniumAlloy, CeramicComposite, RadiationComposite, Copper, Electronics, Motors, Capacitors, Optics, Superconductor, Polymer, RareMetals };
enum class ShipModuleFamily { Command, Hull, Armor, MainDrive, ManeuverThruster, Reactor, Battery, Cargo, Furnace, FieldFabricator, WeaponHardpoint, IndustrialHardpoint, DroneBay, Sensor, Shield, Crew, Utility };

struct MaterialRequirement { ConstructionMaterial material=ConstructionMaterial::StructuralSteel; double quantity=0.0; };
struct ShipModuleBlueprintNative {
    std::string id;
    ShipModuleFamily family=ShipModuleFamily::Hull;
    double mass=0.0;
    double powerDraw=0.0;
    double heat=0.0;
    std::vector<MaterialRequirement> materials;
};
struct ModularShipDesign {
    std::string name;
    std::vector<ShipModuleBlueprintNative> modules;
};
struct ShipBillOfMaterials {
    std::unordered_map<int,double> quantities;
    double totalMass=0.0;
    double totalPowerDraw=0.0;
    double totalHeat=0.0;
    double constructionMinutes=0.0;
};
struct ShipConstructionValidation { bool valid=false; std::vector<std::string> errors; std::vector<std::string> warnings; };

class ShipConstructionSystem {
public:
    ShipBillOfMaterials BuildBill(const ModularShipDesign& design) const;
    ShipConstructionValidation Validate(const ModularShipDesign& design) const;
    ModularShipDesign StarterProspector() const;
    ModularShipDesign StarterScrapper() const;
    static std::string MaterialName(ConstructionMaterial material);
};

} // namespace subspace
