#include "station/StationEditorPlacementSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
const ShipyardModuleRecord* Find(const std::vector<ShipyardModuleRecord>&c,const std::string&id){auto it=std::find_if(c.begin(),c.end(),[&](const auto&r){return r.source.moduleId==id;});return it==c.end()?nullptr:&*it;}
float Dist(const VisualModulePlacement&p,const Vector3&q){const float x=p.x-q.x,y=p.y-q.y,z=p.z-q.z;return std::sqrt(x*x+y*y+z*z);}
}
EditorPlacementResolution StationEditorPlacementSystem::Resolve(const StationKitbashVisualRecipe&recipe,const std::vector<ShipyardModuleRecord>&catalog,const StationKitbashPiece&dragged,const Vector3&pointer){std::vector<EditorPlacementCandidate>candidates;if(!dragged.generatorEligible||dragged.sourceModuleId.empty()){EditorPlacementResolution o;o.state=EditorPlacementState::Invalid;o.message="Station piece has no certified source geometry";return o;}for(std::size_t i=0;i<recipe.modules.size();++i){const auto&target=recipe.modules[i];const auto*record=Find(catalog,target.moduleId);if(!record)continue;EditorPlacementCandidate c;c.targetId="station.module."+std::to_string(i);c.targetSocket="AUTO_OUT";c.childSocket="AUTO_IN";c.distanceScore=Dist(target,pointer);c.compatibilityScore=dragged.structural?1.0f:.92f;c.orientationScore=dragged.radialFriendly?.95f:.82f;c.designScore=recipe.connectedGraph?1.0f:.5f;if(!record->generatorEligible){c.invalidReason="Target module is not PCG-certified";c.clearanceBlocked=true;}if(dragged.role==StationKitbashPieceRole::DockCollar&&recipe.usedDedicatedDock)c.warning="Station already has a primary dock; placement will create an auxiliary berth";candidates.push_back(std::move(c));}return EditorPlacementResolver::Resolve(std::move(candidates));}
} // namespace subspace
