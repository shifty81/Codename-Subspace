#include "integration/NavigationMiningIntegration.h"
#include <algorithm>
#include <cmath>

namespace subspace {

SystemMapSnapshot NavigationMiningIntegration::BuildMapAndNavigation(const GalaxySector& sector,SystemNavigationSystem& navigation) const {
    SystemMapSystem maps;auto map=maps.Build(sector);AstronomicalScaleSystem scale;
    for(const auto& n:map.nodes){SystemDestination d;d.id=n.id;d.name=n.label;d.discovered=n.known;d.warpable=n.warpable;d.hazardRating=n.hazard;d.position.localX=static_cast<double>(n.position.x)*1000000.0;d.position.localY=static_cast<double>(n.position.y)*1000000.0;d.position=scale.Normalize(d.position);switch(n.kind){case SystemMapNodeKind::Planet:d.type=SystemDestinationType::Planet;break;case SystemMapNodeKind::Moon:d.type=SystemDestinationType::Moon;break;case SystemMapNodeKind::Station:case SystemMapNodeKind::OrbitalHub:d.type=SystemDestinationType::Station;break;case SystemMapNodeKind::Belt:d.type=SystemDestinationType::BeltRegion;break;case SystemMapNodeKind::Salvage:d.type=SystemDestinationType::SalvageSite;break;case SystemMapNodeKind::Signature:d.type=SystemDestinationType::Signature;break;default:d.type=SystemDestinationType::DeepSpace;break;}navigation.RegisterDestination(d);}return map;
}

bool NavigationMiningIntegration::AddResolvedDiscovery(SystemMapSnapshot& map,SystemNavigationSystem& nav,std::uint64_t id,const std::string& label,const AstronomicalPosition& pos,double hazard,bool bookmark) const {
    if(id==0||label.empty())return false;SystemDestination d;d.id=id;d.name=label;d.type=SystemDestinationType::Signature;d.position=pos;d.hazardRating=hazard;if(!nav.RegisterDestination(d))return false;SystemMapNode n;n.id=id;n.kind=SystemMapNodeKind::Signature;n.label=label;n.known=true;n.warpable=true;n.hazard=static_cast<float>(hazard);n.position.x=static_cast<float>(pos.localX/1000000.0);n.position.y=static_cast<float>(pos.localY/1000000.0);map.nodes.push_back(n);if(bookmark)nav.AddBookmark(id);return true;
}

bool NavigationMiningIntegration::AddTemporaryTear(SystemMapSnapshot& map,SystemNavigationSystem& nav,std::uint64_t id,const std::string& label,const AstronomicalPosition& pos,double hazard) const {if(id==0)return false;SystemDestination d;d.id=id;d.name=label;d.type=SystemDestinationType::DeepSpace;d.position=pos;d.hazardRating=hazard;if(!nav.RegisterDestination(d))return false;SystemMapNode n;n.id=id;n.kind=SystemMapNodeKind::Signature;n.label=label;n.known=true;n.warpable=true;n.hazard=static_cast<float>(hazard);n.position.x=static_cast<float>(pos.localX/1000000.0);n.position.y=static_cast<float>(pos.localY/1000000.0);map.nodes.push_back(n);return true;}

WarpPlan NavigationMiningIntegration::PlanVector(const SystemNavigationSystem& n,const AstronomicalPosition& from,std::uint64_t destination,double speed,double fuel) const {return n.PlanWarp(from,destination,speed,fuel);}

LocalCelestialScene NavigationMiningIntegration::EvaluateLocalCelestials(const GalaxySector& sector,const Vector3& playerWorld) const {
    LocalCelestialScene out;if(sector.planets.empty())return out;CelestialEnvironmentSystem env;float best=1e30f;for(std::size_t i=0;i<sector.planets.size();++i){Vector3 w{sector.planets[i].position.x*0.0120f,sector.planets[i].position.y*0.0120f,0.0f};float dx=w.x-playerWorld.x,dy=w.y-playerWorld.y;float d=std::sqrt(dx*dx+dy*dy);if(d<best){best=d;out.dominantPlanet=i;}}
    if(out.dominantPlanet<sector.planets.size()){out.majorDiscs.push_back(out.dominantPlanet);const auto& p=sector.planets[out.dominantPlanet];Vector3 w{p.position.x*0.0120f,p.position.y*0.0120f,0.0f};auto context=env.EvaluateLocalContext(playerWorld,w,p,true);out.dominantScreenScale=std::min<double>(env.ProfileFor(p).maximumScreenFraction,std::max(0.0f,context.radiusWorld/std::max(1.0f,context.distanceWorld)));}return out;
}

BeltOperationState NavigationMiningIntegration::EnterBelt(const BeltMacroRegion& belt,RegionCellKey cell,RegionStreamingSystem& streaming) const {BeltOperationState s;s.belt=belt;s.cell=cell;streaming.MarkSurveyed(belt.id,cell);s.materialized=streaming.Materialize(belt,cell);s.materialized.surveyed=true;return s;}

bool NavigationMiningIntegration::MineNode(BeltOperationState& state,std::uint64_t nodeId,RegionStreamingSystem& streaming) const {auto it=std::find_if(state.materialized.asteroids.begin(),state.materialized.asteroids.end(),[&](const auto& a){return a.id==nodeId;});if(it==state.materialized.asteroids.end()||streaming.IsDepleted(nodeId))return false;state.recoveredRichness+=it->richness;state.minedNodes++;streaming.MarkDepleted(nodeId);return true;}

RegionCellKey NavigationMiningIntegration::NextUnsurveyed(const BeltOperationState& state,RegionStreamingSystem& streaming,std::uint64_t salt) const {return streaming.FindUnsurveyedWarpCell(state.belt,state.cell,salt);}

} // namespace subspace
