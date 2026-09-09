#include "interior/ShipInteriorLayoutSystem.h"
#include <algorithm>
namespace subspace {
InteriorLayoutPlan ShipInteriorLayoutSystem::Plan(std::uint64_t shipId,const std::vector<ShipyardModuleRecord>& catalog,const ProceduralShipVisualRecipe& recipe) const{
    InteriorLayoutPlan p;p.shipId=shipId;p.roomTypes={InteriorRoomType::Cockpit,InteriorRoomType::Engineering,InteriorRoomType::Reactor,InteriorRoomType::Cargo,InteriorRoomType::Airlock};
    int hull=0,cargo=0,hangar=0,command=0,propulsion=0;
    for(const auto&m:recipe.modules){auto it=std::find_if(catalog.begin(),catalog.end(),[&](const auto&r){return r.source.moduleId==m.moduleId;});if(it==catalog.end())continue;
        if(it->partRole==ShipyardPartRole::PrimaryHull||it->partRole==ShipyardPartRole::StructuralFrame||it->partRole==ShipyardPartRole::StructuralBlock)++hull;
        if(it->partRole==ShipyardPartRole::Cargo||it->partRole==ShipyardPartRole::Tank)++cargo;
        if(it->partRole==ShipyardPartRole::Hangar)++hangar;
        if(it->partRole==ShipyardPartRole::Bridge||it->partRole==ShipyardPartRole::Cockpit)++command;
        if(it->partRole==ShipyardPartRole::MainEngine||it->partRole==ShipyardPartRole::EngineHousing)++propulsion;
    }
    if(cargo>1)p.roomTypes.push_back(InteriorRoomType::Cargo);
    if(hull>=6)p.roomTypes.push_back(InteriorRoomType::CrewQuarters);
    if(hull>=10)p.roomTypes.push_back(InteriorRoomType::Medbay);
    if(propulsion>=3)p.roomTypes.push_back(InteriorRoomType::Workshop);
    if(hangar>0)p.roomTypes.push_back(InteriorRoomType::Airlock);
    p.corridors=std::max(1,static_cast<int>(p.roomTypes.size())-1);for(int i=0;i<p.corridors;++i)p.roomTypes.push_back(InteriorRoomType::Corridor);
    p.airlocks=static_cast<int>(std::count(p.roomTypes.begin(),p.roomTypes.end(),InteriorRoomType::Airlock));
    p.decks=std::clamp(1+hull/8,1,3);p.rooms=static_cast<int>(p.roomTypes.size());return p;
}
InteriorLayoutPlan ShipInteriorLayoutSystem::Materialize(std::uint64_t shipId,const std::vector<ShipyardModuleRecord>& catalog,const ProceduralShipVisualRecipe& recipe,ShipInteriorSystem& interiors) const{
    auto p=Plan(shipId,catalog,recipe);int i=0;for(auto type:p.roomTypes){const int deck=i%p.decks;int crew=type==InteriorRoomType::CrewQuarters?4:(type==InteriorRoomType::Cockpit?2:0);double power=(type==InteriorRoomType::Engineering||type==InteriorRoomType::Reactor)?12.0:(type==InteriorRoomType::Medbay?6.0:2.0);interiors.AddRoom(shipId,type,deck,crew,power);++i;}return p;
}
}
