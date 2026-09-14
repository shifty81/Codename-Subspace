#include "ship_editor/ShipyardActivityDockSystem.h"

#include <algorithm>

namespace subspace {
std::vector<ShipyardActivityTabDescriptor> ShipyardActivityDockSystem::Tabs(){
    return {{ShipyardActivityTab::Activity,"activity","Activity",false},
            {ShipyardActivityTab::Validation,"validation","Validation",false},
            {ShipyardActivityTab::History,"history","History",false},
            {ShipyardActivityTab::Console,"console","Console",true},
            {ShipyardActivityTab::Search,"search","Search",false},
            {ShipyardActivityTab::Build,"build","Build",true},
            {ShipyardActivityTab::Pcc,"pcc","PCC",true}};
}
ShipyardActivityDockState ShipyardActivityDockSystem::DefaultState(){return {};}
bool ShipyardActivityDockSystem::Activate(ShipyardActivityDockState& state,ShipyardActivityTab tab,bool expand){state.active=tab;if(expand)state.collapsed=false;return true;}
void ShipyardActivityDockSystem::ToggleCollapsed(ShipyardActivityDockState& state){state.collapsed=!state.collapsed;}
void ShipyardActivityDockSystem::RevealValidationFailure(ShipyardActivityDockState& state){if(state.autoRevealErrors){state.active=ShipyardActivityTab::Validation;state.collapsed=false;}}
float ShipyardActivityDockSystem::ClampExpandedHeight(float value){return std::clamp(value,120.0f,640.0f);}
}
