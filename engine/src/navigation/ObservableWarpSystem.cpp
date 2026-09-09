#include "navigation/ObservableWarpSystem.h"
#include <algorithm>
namespace subspace {
void ObservableWarpSystem::EmitStageTransition(std::vector<ObservableWarpEvent>& e,std::uint64_t id,VectorTravelStage prev,VectorTravelStage next,const Vector3&o,const Vector3&d){auto add=[&](WarpEvidenceKind k,float life,float intensity){e.push_back({id,k,o,d.length()>.001f?d.normalized():Vector3{0,1,0},intensity,0.0f,life});};if(next==VectorTravelStage::Charging)add(WarpEvidenceKind::ChargeField,1.8f,.75f);if(prev==VectorTravelStage::Charging&&next==VectorTravelStage::Cruise){add(WarpEvidenceKind::DepartureStreak,2.8f,1.0f);add(WarpEvidenceKind::TransitWake,4.8f,.82f);add(WarpEvidenceKind::CollapseRing,5.8f,.68f);}if(next==VectorTravelStage::Decelerating)add(WarpEvidenceKind::ArrivalFlare,2.6f,.90f);}
void ObservableWarpSystem::Tick(std::vector<ObservableWarpEvent>& e,float dt){for(auto&v:e)v.ageSeconds+=dt;e.erase(std::remove_if(e.begin(),e.end(),[](const auto&v){return v.ageSeconds>=v.lifetimeSeconds;}),e.end());}
} // namespace subspace
