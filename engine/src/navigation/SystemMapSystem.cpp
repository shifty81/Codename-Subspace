#include "navigation/SystemMapSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
std::uint64_t HashNode(std::uint64_t seed, std::uint64_t value) {
    seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
    return seed ? seed : 1;
}
}

SystemMapSnapshot SystemMapSystem::Build(const GalaxySector& sector) const {
    SystemMapSnapshot map;
    map.systemName = "System " + std::to_string(sector.x) + ":" + std::to_string(sector.y);
    std::uint64_t seed = HashNode(static_cast<std::uint64_t>(static_cast<std::uint32_t>(sector.x)), static_cast<std::uint32_t>(sector.y));
    if (sector.hasStar) map.nodes.push_back({HashNode(seed,1),SystemMapNodeKind::Star,sector.star.name,sector.star.position,true,false,0.0f,sector.star.radius});
    for (std::size_t i=0;i<sector.planets.size();++i) {
        const auto& p=sector.planets[i];
        map.nodes.push_back({HashNode(seed,100+i),SystemMapNodeKind::Planet,p.name,p.position,true,true,p.hazardLevel,p.radius});
    }
    for (std::size_t i=0;i<sector.moons.size();++i) {
        const auto& m=sector.moons[i];
        map.nodes.push_back({HashNode(seed,150+i),SystemMapNodeKind::Moon,m.name,m.position,true,true,m.hazardLevel,m.radius});
    }
    for (std::size_t i=0;i<sector.asteroidBelts.size();++i) {
        const auto& belt=sector.asteroidBelts[i];
        SectorPosition marker=sector.star.position;
        if (belt.beltClass==AsteroidBeltClass::PlanetaryDebris) {
            for (const auto& p:sector.planets) if (p.planetId==belt.parentPlanetId) { marker=p.position; break; }
        }
        marker.x += std::cos(belt.orbitalPhaseRadians)*belt.orbitalRadius;
        marker.y += std::sin(belt.orbitalPhaseRadians)*belt.orbitalRadius;
        map.nodes.push_back({HashNode(seed,200+i),SystemMapNodeKind::Belt,belt.name,marker,true,true,
                             0.12f+(1.0f-belt.resourceRichness)*0.18f,
                             std::max(900.0f,belt.outerRadius-belt.innerRadius)});
    }
    if (sector.hasStation) map.nodes.push_back({HashNode(seed,300),SystemMapNodeKind::Station,sector.station.name,sector.station.position,true,true,0.1f,800});
    for (std::size_t i=0;i<sector.orbitalHubs.size();++i) {
        const auto& h=sector.orbitalHubs[i];map.nodes.push_back({HashNode(seed,400+i),SystemMapNodeKind::OrbitalHub,h.hubId,h.position,true,true,0.05f,500});
    }
    for (std::size_t i=0;i<sector.pointsOfInterest.size();++i) {
        const auto& s=sector.pointsOfInterest[i];
        SystemMapNodeKind kind=SystemMapNodeKind::DeepSpace;
        if(s.type==SectorSiteType::MiningField)kind=SystemMapNodeKind::Belt;
        else if(s.type==SectorSiteType::SalvageSite||s.type==SectorSiteType::DerelictYard||s.type==SectorSiteType::DistressWreck)kind=SystemMapNodeKind::Salvage;
        else if(s.type==SectorSiteType::AnomalyBeacon||s.type==SectorSiteType::ResearchOutpost)kind=SystemMapNodeKind::Signature;
        else if(s.type==SectorSiteType::TradeLane)kind=SystemMapNodeKind::TradeLane;
        map.nodes.push_back({HashNode(seed,500+i),kind,s.name,s.position,s.discovered||kind==SystemMapNodeKind::Belt,true,s.danger,s.radius});
    }
    for(auto& n:map.nodes){n.distanceFromOrigin=std::sqrt(n.position.x*n.position.x+n.position.y*n.position.y);switch(n.kind){case SystemMapNodeKind::Planet:n.regionLabel="PLANETARY ORBIT";break;case SystemMapNodeKind::Moon:n.regionLabel="LUNAR ORBIT";break;case SystemMapNodeKind::Station:n.regionLabel="STATION APPROACH";break;case SystemMapNodeKind::OrbitalHub:n.regionLabel="ORBITAL INFRASTRUCTURE";break;case SystemMapNodeKind::Belt:n.regionLabel="BELT REGION";break;case SystemMapNodeKind::Salvage:n.regionLabel="SALVAGE REGION";break;case SystemMapNodeKind::Signature:n.regionLabel="SIGNATURE REGION";break;case SystemMapNodeKind::TradeLane:n.regionLabel="TRADE LANE";break;default:n.regionLabel="SYSTEM SPACE";break;}}
    return map;
}

bool SystemMapSystem::SelectById(SystemMapSnapshot& map,std::uint64_t id) const {
    for(std::size_t i=0;i<map.nodes.size();++i)if(map.nodes[i].id==id){map.selected=i;return true;}return false;
}
const SystemMapNode* SystemMapSystem::Selected(const SystemMapSnapshot& map) const { return map.selected<map.nodes.size()?&map.nodes[map.selected]:nullptr; }
std::vector<SystemMapNode> SystemMapSystem::WarpableKnown(const SystemMapSnapshot& map) const { std::vector<SystemMapNode> out;for(const auto&n:map.nodes)if(n.known&&n.warpable)out.push_back(n);return out; }
float SystemMapSystem::Extent(const SystemMapSnapshot& map) const { float e=1;for(const auto&n:map.nodes)e=std::max(e,std::sqrt(n.position.x*n.position.x+n.position.y*n.position.y)+n.strategicRadius);return e; }
std::vector<std::string> SystemMapSystem::SelectedSummary(const SystemMapSnapshot& map) const {std::vector<std::string> out;const auto*n=Selected(map);if(!n)return out;out.push_back(n->label);out.push_back(std::string("TYPE ")+KindName(n->kind)+" / "+n->regionLabel);out.push_back(std::string(n->known?"KNOWN":"UNRESOLVED")+(n->warpable?" / VECTOR CAPABLE":" / LOCAL ONLY"));out.push_back("HAZARD "+std::to_string(static_cast<int>(n->hazard*100))+"%");return out;}

const char* SystemMapSystem::KindName(SystemMapNodeKind kind){
    switch(kind){
        case SystemMapNodeKind::Star:return "STAR";
        case SystemMapNodeKind::Planet:return "PLANET";
        case SystemMapNodeKind::Moon:return "MOON";
        case SystemMapNodeKind::Station:return "STATION";
        case SystemMapNodeKind::OrbitalHub:return "ORBITAL HUB";
        case SystemMapNodeKind::Belt:return "ASTEROID BELT";
        case SystemMapNodeKind::Salvage:return "SALVAGE SITE";
        case SystemMapNodeKind::Signature:return "SIGNATURE";
        case SystemMapNodeKind::TradeLane:return "TRADE LANE";
        default:return "DEEP SPACE";
    }
}

} // namespace subspace

