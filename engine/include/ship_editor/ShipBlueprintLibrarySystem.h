#pragma once

#include "rendering/ProceduralVisualVariantSystem.h"
#include "ship_editor/ShipyardEquipmentSystem.h"

#include <string>
#include <vector>

namespace subspace {

struct ShipBlueprintDocument {
    std::string blueprintId="player_blueprint";
    std::string name="Untitled Ship";
    std::string author="PLAYER";
    int revision=1;
    ProceduralShipVisualRecipe recipe{};
    std::vector<ShipEquipmentSlot> equipmentSlots;
    ShipAppearanceState appearance{};
    std::vector<std::string> tags;
};

class ShipBlueprintLibrarySystem {
public:
    static bool Save(const ShipBlueprintDocument& blueprint,const std::string& path,std::string* error=nullptr);
    static bool Load(const std::string& path,ShipBlueprintDocument& blueprint,std::string* error=nullptr);
    static std::string CanonicalId(const ShipBlueprintDocument& blueprint);
};

} // namespace subspace
