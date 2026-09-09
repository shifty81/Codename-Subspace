#pragma once
#include "content/ShipyardModuleSystem.h"
#include "interior/ShipInteriorSystem.h"
#include <cstdint>
#include <vector>
namespace subspace {
struct InteriorLayoutPlan { std::uint64_t shipId=0; int decks=1; int corridors=0; int airlocks=0; int rooms=0; std::vector<InteriorRoomType> roomTypes; };
class ShipInteriorLayoutSystem {
public:
    InteriorLayoutPlan Plan(std::uint64_t shipId,const std::vector<ShipyardModuleRecord>& catalog,const ProceduralShipVisualRecipe& recipe) const;
    InteriorLayoutPlan Materialize(std::uint64_t shipId,const std::vector<ShipyardModuleRecord>& catalog,const ProceduralShipVisualRecipe& recipe,ShipInteriorSystem& interiors) const;
};
}
