#include "procedural/GalaxyGenerator.h"
#include "procedural/SolarSystemPlacementSystem.h"
#include "navigation/SystemMapSystem.h"
#include "celestial/OrbitalDynamicsSystem.h"

#include <cmath>
#include <iostream>

using namespace subspace;

namespace {
int failures=0, assertions=0;
void Check(bool ok,const char* name){++assertions;if(!ok){++failures;std::cerr<<"[FAIL] "<<name<<"\n";}else std::cout<<"[PASS] "<<name<<"\n";}
float Dist(const SectorPosition&a,const SectorPosition&b){const float dx=a.x-b.x,dy=a.y-b.y;return std::sqrt(dx*dx+dy*dy);}
}

int main(){
    SolarSystemPlacementSystem placement;
    GalaxyGenerator gen(496496);
    gen.planetProbability=1.0f;
    gen.eventStageProbability=0.0f;
    gen.stationProbability=1.0f;
    gen.wormholeProbability=1.0f;
    gen.anomalyProbability=1.0f;
    gen.asteroidBeltProbability=1.0f;

    int certified=0,moonSystems=0,gasGiantSystems=0;
    for(int y=-3;y<=3;++y){
        for(int x=-3;x<=3;++x){
            auto s=gen.GenerateSector(x,y,0);
            const auto report=placement.Validate(s);
            if(report.certified)++certified;
            Check(report.certified,"generated system passes canonical placement certification");
            const float stellar=placement.StellarExclusionRadiusSector(s.star);
            for(const auto&a:s.asteroids){Check(Dist(a.position,s.star.position)>=stellar,"asteroid outside stellar exclusion");Check(!a.regionId.empty(),"asteroid has belt/site region authority");}
            for(const auto&p:s.pointsOfInterest){Check(Dist(p.position,s.star.position)>=stellar,"POI outside stellar exclusion");Check(!p.parentObjectId.empty()&&!p.placementClass.empty(),"POI carries semantic placement authority");}
            for(const auto&w:s.wormholes){Check(Dist(w.position,s.star.position)>=stellar,"wormhole is deep-space safe");Check(w.placementClass=="DEEP_SPACE","wormhole classified deep space");}
            for(const auto&a:s.anomalies){Check(Dist(a.position,s.star.position)>=stellar,"anomaly is deep-space safe");Check(a.placementClass=="DEEP_SPACE","anomaly classified deep space");}
            if(s.hasStation){Check(Dist(s.station.position,s.star.position)>=stellar,"station outside stellar exclusion");Check(!s.station.parentObjectId.empty(),"station has explicit orbital parent");}
            if(!s.moons.empty())++moonSystems;
            for(const auto&p:s.planets)if(p.type==PlanetType::GasGiant)++gasGiantSystems;
            for(const auto&m:s.moons){
                bool parentFound=false;float parentEnvelope=0.0f;
                for(const auto&p:s.planets)if(p.planetId==m.parentPlanetId){parentFound=true;parentEnvelope=placement.PlanetEnvelopeRadiusSector(p);break;}
                Check(parentFound,"moon has first-class planet parent");
                Check(m.orbitalRadius>=parentEnvelope,"moon orbit clears parent atmosphere/rings");
            }
        }
    }
    Check(certified==49,"all 49 deterministic certification fixtures pass");
    Check(moonSystems>0&&gasGiantSystems>0,"generated sample includes persistent moons and gas giants");

    auto sample=gen.GenerateSector(2,-1,0);
    SystemMapSystem maps;const auto map=maps.Build(sample);
    bool mapMoon=sample.moons.empty();
    for(const auto&n:map.nodes)if(n.kind==SystemMapNodeKind::Moon)mapMoon=true;
    Check(mapMoon,"System Map consumes first-class moon records");

    OrbitalDynamicsSystem dynamics;const auto orbital=dynamics.DeriveSystemOrbits(sample);
    bool orbitalMoon=sample.moons.empty();
    for(const auto&b:orbital)if(b.kind==OrbitalBodyKind::Moon)orbitalMoon=true;
    Check(orbitalMoon,"orbital simulation consumes first-class moon records");

    GalaxyGenerator legacyContract(42);legacyContract.stationProbability=1.0f;legacyContract.wormholeProbability=1.0f;legacyContract.minAsteroids=1;legacyContract.maxAsteroids=1;
    const auto one=legacyContract.GenerateSector(0,0,0);
    Check(one.asteroids.size()==1,"legacy materialized asteroid budget remains configurable");
    Check(!one.asteroids.empty()&&!one.asteroids.front().regionId.empty(),"legacy asteroid budget is reparented rather than origin-spawned");

    std::cout<<"Pass496 assertions: "<<assertions-failures<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
