#include "interior/ShipInteriorAuthoringSystem.h"

#include <algorithm>

namespace subspace {

int ShipInteriorAuthoringSystem::TargetDeckCount(ShipClass c){
    switch(c){
    case ShipClass::Shuttle:return 1;
    case ShipClass::Frigate:return 1;
    case ShipClass::Destroyer:return 1;
    case ShipClass::Cruiser:return 2;
    case ShipClass::Battlecruiser:return 2;
    case ShipClass::Battleship:return 3;
    case ShipClass::Carrier:return 4;
    case ShipClass::Dreadnought:return 5;
    case ShipClass::IndustrialCapital:return 5;
    case ShipClass::Capital:return 6;
    }
    return 1;
}

GeneratedShipInteriorProgram ShipInteriorAuthoringSystem::Generate(const ShipInteriorConnectionPlan& plan,
                                                                   ShipClass shipClass,
                                                                   float cellSizeMeters,
                                                                   float deckHeightMeters){
    GeneratedShipInteriorProgram out;out.shipClass=shipClass;out.targetDeckCount=TargetDeckCount(shipClass);
    cellSizeMeters=std::max(.25f,cellSizeMeters);deckHeightMeters=std::max(2.0f,deckHeightMeters);
    for(std::size_t i=0;i<plan.bindings.size();++i){
        const auto& b=plan.bindings[i];if(!b.walkable)continue;
        const int decks=std::max(1,b.deckCount);
        for(int deck=0;deck<decks;++deck){
            ShipInteriorRoomVolume room;room.moduleIndex=i;room.moduleId=b.moduleId;room.capability=b.capability;room.deck=deck;
            room.widthCells=std::max(1,b.footprintWidthCells);room.lengthCells=std::max(1,b.footprintLengthCells);room.cellSizeMeters=cellSizeMeters;room.deckHeightMeters=deckHeightMeters;room.walkable=true;out.rooms.push_back(std::move(room));
        }
    }
    for(const auto& e:plan.edges){ShipInteriorPortalLink p;p.moduleA=e.moduleA;p.moduleB=e.moduleB;p.portalA=e.portalA;p.portalB=e.portalB;p.kind=e.kind;p.walkable=e.walkable;out.portals.push_back(std::move(p));}
    if(out.rooms.empty())out.warnings.push_back("No walkable module volume is available for this exterior assembly");
    bool walkablePortal=false;for(const auto&p:out.portals)walkablePortal|=p.walkable;
    if(out.rooms.size()>1&&!walkablePortal)out.warnings.push_back("Interior rooms exist but no walkable exterior attachment portal connects them");
    if(static_cast<int>(out.rooms.size())<out.targetDeckCount&&shipClass>=ShipClass::Cruiser)out.warnings.push_back("Generated class expects more interior deck/room coverage than current modules provide");
    out.valid=out.errors.empty()&&!out.rooms.empty();return out;
}

} // namespace subspace
