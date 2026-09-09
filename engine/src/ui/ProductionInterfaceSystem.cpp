#include "ui/ProductionInterfaceSystem.h"
namespace subspace {
ProductionHudModel ProductionInterfaceSystem::Build(ProductionContextKind contact,const std::string&id,ShipEmbodimentMode e,DockingExperienceStage d,bool transit) const {
    ProductionHudModel m;
    if(transit)m.modeLabel="VECTOR TRANSIT";else if(e==ShipEmbodimentMode::InteriorOnFoot)m.modeLabel="SHIP INTERIOR";else if(e==ShipEmbodimentMode::DockedHangar||d==DockingExperienceStage::Docked)m.modeLabel="STATION HANGAR";else if(e==ShipEmbodimentMode::CutawayInspection)m.modeLabel="SHIP INSPECTION";else m.modeLabel="FLIGHT";
    m.showScanner=e==ShipEmbodimentMode::CockpitControl||e==ShipEmbodimentMode::CutawayInspection;m.showModuleRack=m.showScanner;
    switch(contact){
        case ProductionContextKind::Planet:m.contextTitle=id.empty()?"PLANET":id;m.contextActions={"VECTOR TO ORBIT","SURVEY","BOOKMARK","INFORMATION"};break;
        case ProductionContextKind::Station:m.contextTitle=id.empty()?"STATION":id;m.contextActions={"APPROACH","REQUEST DOCK","VECTOR TO","MARKET / SERVICES"};break;
        case ProductionContextKind::Site:m.contextTitle=id.empty()?"POINT OF INTEREST":id;m.contextActions={"VECTOR TO SITE","DIRECTIONAL SCAN","APPROACH","BOOKMARK"};break;
        case ProductionContextKind::Asteroid:m.contextTitle="ASTEROID";m.contextActions={"APPROACH","LOCK","MINE","BOOKMARK"};break;
        case ProductionContextKind::Ship:m.contextTitle=id.empty()?"SHIP":id;m.contextActions={"APPROACH","LOCK / INSPECT","FLEET","FOLLOW"};break;
        case ProductionContextKind::Derelict:m.contextTitle=id.empty()?"DERELICT":id;m.contextActions={"APPROACH","SCAN","SALVAGE","BOOKMARK"};break;
        default:break;
    }
    for(const auto& action:m.contextActions)m.actions.push_back({action,true,{}});
    if(transit){for(auto& a:m.actions)if(a.label.find("VECTOR")!=std::string::npos||a.label.find("DOCK")!=std::string::npos){a.enabled=false;a.disabledReason="Unavailable during Vector transit";}}
    m.topBar={m.modeLabel, id.empty()?"NO TARGET":id};
    m.bottomHints={"TAB CONTEXT","M SYSTEM MAP","I COCKPIT / INTERIOR"};
    if(e==ShipEmbodimentMode::InteriorOnFoot)m.statusLines={"WASD MOVE","I TAKE CONTROLS AT COCKPIT","RMB ROTATE CAMERA","WHEEL LIMITED ZOOM"};
    else if(d==DockingExperienceStage::Docked)m.statusLines={"I BOARD SHIP INTERIOR","ENTER UNDOCK","H HANGAR / FITTING"};
    else m.statusLines={"I EXIT COCKPIT","I SHIP INSPECTION","M SYSTEM MAP","RMB ORBIT / WHEEL ZOOM"};
    return m;
}
} // namespace subspace
