#include "interior/InteriorInteractionSystem.h"
#include <algorithm>
#include <utility>
namespace subspace {
namespace {
InteriorInteractionOption Opt(std::string label,bool enabled,std::string reason,std::string id){return {std::move(label),enabled,std::move(reason),std::move(id)};}
}
std::vector<InteriorInteractionOption> InteriorInteractionSystem::ActionsFor(const InteriorRoom&r,bool damaged,bool powered) const {
    switch(r.type){
        case InteriorRoomType::Cockpit:return {Opt("TAKE CONTROLS",true,"","take_controls"),Opt("NAVIGATION",powered,powered?"":"NO POWER","navigation")};
        case InteriorRoomType::Engineering:return {Opt("INSPECT ENGINES",true,"","inspect_engines"),Opt("REPAIR SUBSYSTEM",damaged,damaged?"":"NO DAMAGE","repair_subsystem"),Opt("POWER ROUTING",powered,powered?"":"NO POWER","power_routing")};
        case InteriorRoomType::Reactor:return {Opt("REACTOR STATUS",true,"","reactor_status"),Opt("SCRAM REACTOR",powered,powered?"":"OFFLINE","reactor_scram")};
        case InteriorRoomType::Cargo:return {Opt("OPEN CARGO",true,"","open_cargo"),Opt("TRANSFER MANIFEST",true,"","transfer_manifest")};
        case InteriorRoomType::Airlock:return {Opt("CYCLE AIRLOCK",r.pressurized,r.pressurized?"":"DEPRESSURIZED","cycle_airlock"),Opt("SUIT CHECK",true,"","suit_check")};
        default:return {Opt("INSPECT",true,"","inspect")};
    }
}
std::vector<InteriorInteractionOption> InteriorInteractionSystem::ActionsFor(const InteriorFixtureState&f) const {return ActionsFor(f,{});}
std::vector<InteriorInteractionOption> InteriorInteractionSystem::ActionsFor(const InteriorFixtureState&f,const InteriorInteractionContext&c) const {
    const bool rangeOk=c.distanceMeters<=0.0f||c.distanceMeters<=std::max(0.25f,f.interactionRangeMeters);
    const bool accessOk=f.accessAllowed&&c.hasAccess;
    const auto ready=[&](){return rangeOk&&accessOk&&f.powered;};
    const std::string blocked=!rangeOk?"OUT OF RANGE":(!accessOk?"ACCESS DENIED":(!f.powered?"NO POWER":""));
    switch(f.kind){
        case InteriorFixtureKind::Console:return {Opt("USE CONSOLE",ready(),blocked,"use_console")};
        case InteriorFixtureKind::EngineeringPanel:return {Opt("ENGINEERING",ready(),blocked,"engineering"),Opt("REPAIR",ready(),blocked,"repair")};
        case InteriorFixtureKind::CargoTerminal:return {Opt("OPEN CARGO",ready(),blocked,"open_cargo")};
        case InteriorFixtureKind::StorageContainer:return {Opt("OPEN STORAGE",rangeOk&&accessOk,!rangeOk?"OUT OF RANGE":(!accessOk?"ACCESS DENIED":""),"open_storage")};
        case InteriorFixtureKind::FleetCommandTerminal:return {Opt("ENTER FLEET COMMAND",ready(),blocked,"fleet_command")};
        case InteriorFixtureKind::NavigationConsole:return {Opt("OPEN NAVIGATION",ready(),blocked,"navigation")};
        case InteriorFixtureKind::MiningConsole:return {Opt("MINING CONTROL",ready(),blocked,"mining")};
        case InteriorFixtureKind::SalvageConsole:return {Opt("SALVAGE CONTROL",ready(),blocked,"salvage")};
        case InteriorFixtureKind::RefineryConsole:return {Opt("REFINERY CONTROL",ready(),blocked,"refinery")};
        case InteriorFixtureKind::ManufacturingConsole:return {Opt("MANUFACTURING",ready(),blocked,"manufacturing")};
        case InteriorFixtureKind::MedicalStation:return {Opt("MEDICAL SERVICES",ready(),blocked,"medical")};
        case InteriorFixtureKind::HelmSeat:return {Opt("TAKE CONTROLS",rangeOk&&accessOk&&!f.locked,!rangeOk?"OUT OF RANGE":(!accessOk?"ACCESS DENIED":(f.locked?"LOCKED":"")),"take_controls")};
        case InteriorFixtureKind::RepairPanel:return {Opt("INSPECT",rangeOk&&accessOk,!rangeOk?"OUT OF RANGE":(!accessOk?"ACCESS DENIED":""),"inspect"),Opt("REPAIR",ready(),blocked,"repair")};
        case InteriorFixtureKind::Elevator:return {Opt("SELECT DESTINATION",ready(),blocked,"elevator_select")};
        case InteriorFixtureKind::Airlock:{
            const bool canCycle=rangeOk&&accessOk&&!f.locked&&f.powered&&!f.cycling;
            std::string why=!rangeOk?"OUT OF RANGE":(!accessOk?"ACCESS DENIED":(f.locked?"LOCKED":(!f.powered?"NO POWER":(f.cycling?"CYCLING":""))));
            return {Opt(f.open?"CLOSE AIRLOCK":"CYCLE AIRLOCK",canCycle,why,f.open?"close_airlock":"cycle_airlock"),Opt("EMERGENCY SEAL",rangeOk,rangeOk?"":"OUT OF RANGE","emergency_seal")};
        }
        case InteriorFixtureKind::Door:
        case InteriorFixtureKind::Hatch:
        default:{
            const bool canOpen=rangeOk&&accessOk&&!f.locked;
            const std::string why=!rangeOk?"OUT OF RANGE":(!accessOk?"ACCESS DENIED":(f.locked?"LOCKED":""));
            return {Opt(f.open?"CLOSE":"OPEN",canOpen,why,f.open?"close":"open"),Opt("LOCK / UNLOCK",rangeOk&&accessOk&&f.powered,!rangeOk?"OUT OF RANGE":(!accessOk?"ACCESS DENIED":(!f.powered?"NO POWER":"")),"toggle_lock")};
        }
    }
}
InteriorInteractionResult InteriorInteractionSystem::Execute(InteriorFixtureState&f,const std::string&action) const{return Execute(f,action,{});}
InteriorInteractionResult InteriorInteractionSystem::Execute(InteriorFixtureState&f,const std::string&action,const InteriorInteractionContext&c) const {
    const auto options=ActionsFor(f,c);
    const auto it=std::find_if(options.begin(),options.end(),[&](const InteriorInteractionOption&o){return o.label==action||o.actionId==action;});
    if(it==options.end())return {false,"UNKNOWN ACTION",action,f.fixtureId,f.linkedSystemId,f.destinationId};
    if(!it->enabled)return {false,it->reason.empty()?"BLOCKED":it->reason,it->actionId,f.fixtureId,f.linkedSystemId,f.destinationId};
    const auto&id=it->actionId;
    if(id=="emergency_seal"){f.open=false;f.cycling=false;return {true,"SEALED",id,f.fixtureId,f.linkedSystemId,f.destinationId};}
    if(id=="toggle_lock"){f.locked=!f.locked;if(f.locked)f.open=false;return {true,f.locked?"LOCKED":"UNLOCKED",id,f.fixtureId,f.linkedSystemId,f.destinationId};}
    if(id=="open"){f.open=true;return {true,"OPEN",id,f.fixtureId,f.linkedSystemId,f.destinationId};}
    if(id=="close"){f.open=false;return {true,"CLOSED",id,f.fixtureId,f.linkedSystemId,f.destinationId};}
    if(id=="cycle_airlock"){f.cycling=true;f.open=false;f.pressurized=!f.pressurized;f.cycling=false;return {true,f.pressurized?"PRESSURIZED":"DEPRESSURIZED",id,f.fixtureId,f.linkedSystemId,f.destinationId};}
    if(id=="close_airlock"){f.open=false;return {true,"CLOSED",id,f.fixtureId,f.linkedSystemId,f.destinationId};}
    // Terminals/elevator/workstations return a structured dispatch result. The
    // owning gameplay service performs the domain action; this system never
    // duplicates inventory/mining/fleet/refinery authority.
    return {true,"READY",id,f.fixtureId,f.linkedSystemId,f.destinationId};
}
}
