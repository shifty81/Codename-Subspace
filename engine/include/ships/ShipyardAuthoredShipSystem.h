#pragma once
#include "rendering/ProceduralVisualVariantSystem.h"
#include <string>
#include <vector>
namespace subspace {
struct ShipyardAuthoredShipDefinition {
    std::string id;
    std::string displayName;
    std::string moduleId;
    std::string role;
    float width=1.0f,length=1.0f,height=1.0f;
    bool capturable=true;
};
class ShipyardAuthoredShipSystem {
public:
    static const std::vector<ShipyardAuthoredShipDefinition>& Definitions();
    static const ShipyardAuthoredShipDefinition* Find(const std::string& id);
    static ProceduralShipVisualRecipe BuildRecipe(const ShipyardAuthoredShipDefinition& d);
};
}
