#include "procedural/SystemTopologySystem.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace subspace {

SystemTopology SystemTopologySystem::Generate(std::uint64_t galaxySeed, int sectorX, int sectorY, double securityRating) const {
    SystemTopology t; t.sectorX=sectorX; t.sectorY=sectorY;
    const std::uint64_t mixed = galaxySeed ^ (static_cast<std::uint64_t>(static_cast<std::uint32_t>(sectorX))*0x9E3779B185EBCA87ULL)
                                    ^ (static_cast<std::uint64_t>(static_cast<std::uint32_t>(sectorY))*0xC2B2AE3D27D4EB4FULL);
    t.seed=mixed; std::mt19937_64 rng(mixed); std::uniform_real_distribution<double> pos(-80.0,80.0); std::uniform_real_distribution<double> u(0.0,1.0);
    const double sec=std::clamp(securityRating,0.0,1.0);
    t.gateCount = sec > 0.65 ? 3 : (sec > 0.3 ? 2 : 1);
    for(int i=0;i<t.gateCount;++i){ double a=(6.283185307179586*i)/t.gateCount; t.sites.push_back({TopologySiteType::Stargate,std::cos(a)*70.0,std::sin(a)*70.0,"","gate"}); }
    const int stations = sec > 0.7 ? 2 : (sec > 0.25 ? 1 : 0);
    for(int i=0;i<stations;++i)t.sites.push_back({TopologySiteType::Station,pos(rng),pos(rng),sec>0.55?"settled":"frontier","station"});
    const int asteroids=2+static_cast<int>((1.0-sec)*4.0);
    for(int i=0;i<asteroids;++i)t.sites.push_back({TopologySiteType::LargeAsteroid,pos(rng),pos(rng),"","claimable_asteroid"});
    if(u(rng)>sec)t.sites.push_back({TopologySiteType::DerelictCluster,pos(rng),pos(rng),"","salvage"});
    if(sec<0.45)t.sites.push_back({TopologySiteType::FactionAnchor,pos(rng),pos(rng),u(rng)>0.5?"pirate":"unknown","hidden_presence"});
    t.sites.push_back({TopologySiteType::ResourceField,pos(rng),pos(rng),"","resource_field"});
    return t;
}

} // namespace subspace
