#include "ship_editor/ShipyardCapabilitySystem.h"

namespace subspace {
ShipyardCapabilityProfile ShipyardCapabilitySystem::For(ShipyardAccessMode mode){
    ShipyardCapabilityProfile p;
    if(mode==ShipyardAccessMode::PlayerDocked)return p;
    p.model=true;p.pcgStudio=true;p.world=true;p.interior=true;p.character=true;p.devWorld=true;p.sockets=true;p.rawAuthoring=true;p.publishCanonicalAsset=true;p.playFromHere=true;p.modifySourceClassification=true;return p;
}
const char* ShipyardCapabilitySystem::ModeName(ShipyardAccessMode mode){switch(mode){case ShipyardAccessMode::PlayerDocked:return "PLAYER SHIPYARD";case ShipyardAccessMode::MainMenuStudio:return "SHIPYARD DEV STUDIO";case ShipyardAccessMode::RuntimeDeveloper:return "RUNTIME DEV SHIPYARD";}return "SHIPYARD";}
}
