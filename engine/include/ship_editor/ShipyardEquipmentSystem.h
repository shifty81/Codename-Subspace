#pragma once

#include "content/ShipyardModuleSystem.h"
#include "inventory/ItemizationSystem.h"

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

enum class ShipEquipmentSlotType {
    Command,
    MainEngine,
    Rcs,
    Weapon,
    Missile,
    Sensor,
    Utility,
    Cargo,
    Power,
    Mining,
    Salvage,
    Drone
};

struct ShipEquipmentSlot {
    std::string slotId;
    std::size_t moduleIndex = 0;
    ShipEquipmentSlotType type = ShipEquipmentSlotType::Utility;
    ShipyardModuleSize size = ShipyardModuleSize::S;
    std::string installedItemInstanceId;
    std::string installedDefinitionId;
    bool required = false;
};

struct ShipPaintLayer {
    std::string id;
    float r = 0.34f, g = 0.39f, b = 0.43f, a = 1.0f;
    float metallic = 0.65f;
    float roughness = 0.34f;
};

struct ShipDecalLayer {
    std::string id;
    std::string decalAsset;
    std::size_t moduleIndex = 0;
    float u = 0.5f, v = 0.5f;
    float scale = 1.0f;
    float rotationDegrees = 0.0f;
    float opacity = 1.0f;
    bool mirror = false;
};

struct ShipAppearanceState {
    ShipPaintLayer primary{"PRIMARY",.34f,.39f,.43f,1.0f,.65f,.34f};
    ShipPaintLayer secondary{"SECONDARY",.18f,.22f,.25f,1.0f,.58f,.40f};
    ShipPaintLayer trim{"TRIM",.72f,.42f,.12f,1.0f,.72f,.28f};
    std::vector<ShipDecalLayer> decals;
    float factoryWear = 0.0f;
};

class ShipyardEquipmentSystem {
public:
    static std::vector<ShipEquipmentSlot> BuildSlots(const ProceduralShipVisualRecipe& recipe,
                                                     const std::vector<ShipyardModuleRecord>& catalog);
    static bool Compatible(const ShipEquipmentSlot& slot,const GeneratedItem& item);
    static bool Install(ShipEquipmentSlot& slot,const GeneratedItem& item,std::string* error=nullptr);
    static const char* SlotName(ShipEquipmentSlotType type);
};

} // namespace subspace
