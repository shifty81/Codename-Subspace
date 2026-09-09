#include "interior/InteriorInteractionSystem.h"
namespace subspace {
std::vector<InteriorInteractionOption> InteriorInteractionSystem::ActionsFor(const InteriorRoom&r,bool damaged,bool powered) const {std::vector<InteriorInteractionOption> o;switch(r.type){case InteriorRoomType::Cockpit:o={{"TAKE CONTROLS",true,""},{"NAVIGATION",powered,powered?"":"NO POWER"}};break;case InteriorRoomType::Engineering:o={{"INSPECT ENGINES",true,""},{"REPAIR SUBSYSTEM",damaged,damaged?"":"NO DAMAGE"},{"POWER ROUTING",powered,powered?"":"NO POWER"}};break;case InteriorRoomType::Reactor:o={{"REACTOR STATUS",true,""},{"SCRAM REACTOR",powered,powered?"":"OFFLINE"}};break;case InteriorRoomType::Cargo:o={{"OPEN CARGO",true,""},{"TRANSFER MANIFEST",true,""}};break;case InteriorRoomType::Airlock:o={{"CYCLE AIRLOCK",r.pressurized,r.pressurized?"":"DEPRESSURIZED"},{"SUIT CHECK",true,""}};break;default:o={{"INSPECT",true,""}};break;}return o;}
std::vector<InteriorInteractionOption> InteriorInteractionSystem::ActionsFor(const InteriorFixtureState& f) const {
    if(f.kind==InteriorFixtureKind::Console||f.kind==InteriorFixtureKind::EngineeringPanel||f.kind==InteriorFixtureKind::CargoTerminal)return {{"USE CONSOLE",f.powered,f.powered?"":"NO POWER"}};
    if(f.kind==InteriorFixtureKind::Airlock)return {{f.open?"CLOSE AIRLOCK":"CYCLE AIRLOCK",!f.locked&&f.powered&&!f.cycling,f.locked?"LOCKED":(!f.powered?"NO POWER":(f.cycling?"CYCLING":""))},{"EMERGENCY SEAL",true,""}};
    return {{f.open?"CLOSE":"OPEN",!f.locked,f.locked?"LOCKED":""},{"LOCK / UNLOCK",f.powered,f.powered?"":"NO POWER"}};
}
InteriorInteractionResult InteriorInteractionSystem::Execute(InteriorFixtureState& f,const std::string& action) const {
    if(action=="EMERGENCY SEAL"){f.open=false;f.cycling=false;return {true,"SEALED"};}
    if(action=="USE CONSOLE")return {f.powered,f.powered?"CONSOLE READY":"NO POWER"};
    if(action=="LOCK / UNLOCK"){if(!f.powered)return {false,"NO POWER"};f.locked=!f.locked;if(f.locked)f.open=false;return {true,f.locked?"LOCKED":"UNLOCKED"};}
    if(action=="OPEN"||action=="CLOSE"){if(f.locked)return {false,"LOCKED"};f.open=action=="OPEN";return {true,f.open?"OPEN":"CLOSED"};}
    if(action=="CYCLE AIRLOCK"||action=="CLOSE AIRLOCK"){if(f.locked)return {false,"LOCKED"};if(!f.powered)return {false,"NO POWER"};f.cycling=true;f.open=false;f.pressurized=!f.pressurized;f.cycling=false;return {true,f.pressurized?"PRESSURIZED":"DEPRESSURIZED"};}
    return {false,"UNKNOWN ACTION"};
}
}
