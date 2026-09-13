#include "interior/ShipInteriorLayoutSystem.h"

#include <algorithm>

namespace subspace {
namespace {
int CrewFor(InteriorRoomType t){return t==InteriorRoomType::CrewQuarters?4:(t==InteriorRoomType::Cockpit?2:0);}
double PowerFor(InteriorRoomType t){return (t==InteriorRoomType::Engineering||t==InteriorRoomType::Reactor)?12.0:(t==InteriorRoomType::Medbay?6.0:2.0);}
}

InteriorLayoutPlan ShipInteriorLayoutSystem::Plan(std::uint64_t shipId,
                                                   const std::vector<ShipyardModuleRecord>& catalog,
                                                   const ProceduralShipVisualRecipe& recipe) const
{
    InteriorLayoutPlan p;p.shipId=shipId;
    p.carve=ShipInteriorCarvingSystem::Carve(catalog,recipe);
    p.decks=std::max(1,p.carve.deckCount);
    p.warnings=p.carve.warnings;

    // The room list is now a projection of the actual carved pressure hull.
    // One transformed walkable module produces one primary room/cell; actual
    // stitched portals produce corridor connectivity. No unrelated generic
    // room list is fabricated from module counts anymore.
    for(const auto& v:p.carve.volumes){p.roomTypes.push_back(v.roomType);if(v.roomType==InteriorRoomType::Airlock)++p.airlocks;}
    for(const auto& portal:p.carve.portals)if(portal.walkable){p.roomTypes.push_back(InteriorRoomType::Corridor);++p.corridors;}
    p.rooms=static_cast<int>(p.roomTypes.size());
    return p;
}

InteriorLayoutPlan ShipInteriorLayoutSystem::Materialize(std::uint64_t shipId,
                                                           const std::vector<ShipyardModuleRecord>& catalog,
                                                           const ProceduralShipVisualRecipe& recipe,
                                                           ShipInteriorSystem& interiors) const
{
    auto p=Plan(shipId,catalog,recipe);
    // Preserve runtime damage/crew state when callers intentionally own that
    // lifecycle; current refit/capture callers clear explicitly before rebuild.
    for(const auto& v:p.carve.volumes){interiors.AddRoom(shipId,v.roomType,v.deck,CrewFor(v.roomType),PowerFor(v.roomType));}
    for(const auto& portal:p.carve.portals)if(portal.walkable){
        int deck=0;
        for(const auto& v:p.carve.volumes)if(v.moduleIndex==portal.moduleA){deck=v.deck;break;}
        interiors.AddRoom(shipId,InteriorRoomType::Corridor,deck,0,1.0);
    }
    return p;
}
}
