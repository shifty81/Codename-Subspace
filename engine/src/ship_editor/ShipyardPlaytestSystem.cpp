#include "ship_editor/ShipyardPlaytestSystem.h"

namespace subspace {
bool ShipyardPlaytestSystem::BeginInterior(std::uint64_t id,const InteriorTraversalBounds&bounds){
    if(id==0)return false;
    embodiment_=ShipEmbodimentSystem{};embodiment_.SetTraversalBounds(bounds);
    if(!embodiment_.ExitCockpit(id))return false;
    state_={ShipyardPlaytestMode::InteriorWalk,id,true,false,"PLAY / INTERIOR FPS"};return true;
}
bool ShipyardPlaytestSystem::TakeCockpit(){if(!state_.active)return false;if(!embodiment_.TakeControls())return false;state_.mode=ShipyardPlaytestMode::CockpitFlight;state_.status="PLAY / COCKPIT 6DOF";return true;}
bool ShipyardPlaytestSystem::OpenRemoteFleetCommand(){if(!state_.active)return false;state_.mode=ShipyardPlaytestMode::RemoteFleetCommand;state_.status="PLAY / REMOTE FLEET COMMAND";return true;}
void ShipyardPlaytestSystem::ReturnToEditing(){state_={};embodiment_=ShipEmbodimentSystem{};}
void ShipyardPlaytestSystem::MoveOnFoot(float f,float s,double dt){if(state_.mode==ShipyardPlaytestMode::InteriorWalk)embodiment_.Move(f,s,dt);}
void ShipyardPlaytestSystem::LookOnFoot(float y,float p){if(state_.mode==ShipyardPlaytestMode::InteriorWalk)embodiment_.Look(y,p);}
FirstPersonViewPose ShipyardPlaytestSystem::CurrentOnFootView() const{return FirstPersonViewSystem::BuildOnFootLocal(embodiment_.Avatar());}
} // namespace subspace
