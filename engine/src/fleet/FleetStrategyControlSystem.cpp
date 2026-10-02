#include "fleet/FleetStrategyControlSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {

void FleetStrategyControlSystem::NormalizeUnique(std::vector<std::uint64_t>& ids){
    ids.erase(std::remove(ids.begin(),ids.end(),0),ids.end());
    std::sort(ids.begin(),ids.end());ids.erase(std::unique(ids.begin(),ids.end()),ids.end());
}
void FleetStrategyControlSystem::ClearSelection(){_selection.selected.clear();}
bool FleetStrategyControlSystem::IsSelected(std::uint64_t id)const{return std::find(_selection.selected.begin(),_selection.selected.end(),id)!=_selection.selected.end();}
void FleetStrategyControlSystem::Select(std::uint64_t id,FleetSelectionMode mode){
    if(!id)return;
    if(mode==FleetSelectionMode::Replace){_selection.selected={id};return;}
    const auto it=std::find(_selection.selected.begin(),_selection.selected.end(),id);
    if(mode==FleetSelectionMode::Toggle){if(it!=_selection.selected.end())_selection.selected.erase(it);else _selection.selected.push_back(id);}
    else if(it==_selection.selected.end())_selection.selected.push_back(id);
    NormalizeUnique(_selection.selected);
}
void FleetStrategyControlSystem::SelectMany(const std::vector<std::uint64_t>& ids,FleetSelectionMode mode){
    if(mode==FleetSelectionMode::Replace)_selection.selected.clear();
    for(const auto id:ids){if(!id)continue;if(mode==FleetSelectionMode::Toggle)Select(id,FleetSelectionMode::Toggle);else _selection.selected.push_back(id);}
    NormalizeUnique(_selection.selected);
}
bool FleetStrategyControlSystem::AssignCommandGroup(int index){if(index<0||index>=static_cast<int>(_selection.commandGroups.size())||_selection.selected.empty())return false;_selection.commandGroups[static_cast<std::size_t>(index)]=_selection.selected;return true;}
bool FleetStrategyControlSystem::RecallCommandGroup(int index,bool add){if(index<0||index>=static_cast<int>(_selection.commandGroups.size()))return false;const auto&g=_selection.commandGroups[static_cast<std::size_t>(index)];if(g.empty())return false;SelectMany(g,add?FleetSelectionMode::Add:FleetSelectionMode::Replace);return true;}
void FleetStrategyControlSystem::TickCamera(const ControlIntent& intent,float dt){
    if(intent.domain!=ControlDomain::FleetStrategy||dt<=0.0f)return;
    const float seconds=std::min(dt,0.1f);const float yaw=_camera.yawDegrees*0.01745329251994329577f;
    const Vector3 forward{-std::sin(yaw),std::cos(yaw),0.0f};const Vector3 right{std::cos(yaw),std::sin(yaw),0.0f};
    const float scale=_camera.panSpeed*std::clamp(_camera.distance/120.0f,0.25f,6.0f)*seconds;
    _camera.focus=_camera.focus+(forward*intent.cameraForward+right*intent.cameraRight+Vector3{0,0,intent.cameraUp})*scale;
}
void FleetStrategyControlSystem::Zoom(float wheel){if(std::fabs(wheel)<0.0001f)return;const float factor=std::exp(-wheel*0.12f*_camera.zoomSpeed);_camera.distance=std::clamp(_camera.distance*factor,12.0f,5000.0f);}
void FleetStrategyControlSystem::Orbit(float yawDelta,float tiltDelta){_camera.yawDegrees+=yawDelta;_camera.tiltDegrees=std::clamp(_camera.tiltDegrees+tiltDelta,10.0f,89.0f);}
FleetOrderRequest FleetStrategyControlSystem::BuildRequest(const FleetStrategyOrderDraft& d)const{
    FleetOrderRequest r;r.type=d.type;r.targetEntityId=d.targetEntityId;r.targetX=d.targetPosition.x;r.targetY=d.targetPosition.y;r.targetZ=d.targetPosition.z;r.priority=d.priority;r.queue=d.queue;r.acceptanceRadius=d.acceptanceRadius;r.orbitRadius=d.orbitRadius;r.assignedMembers=_selection.selected;return r;
}
bool FleetStrategyControlSystem::Issue(FleetCommandComponent& fleet,const FleetStrategyOrderDraft& d)const{if(_selection.selected.empty())return false;return fleet.IssueOrder(BuildRequest(d));}
void FleetStrategyControlSystem::SetFormation(FormationType type,float spacing){_formation=type;_formationSpacing=std::clamp(spacing,2.0f,500.0f);}

} // namespace subspace
