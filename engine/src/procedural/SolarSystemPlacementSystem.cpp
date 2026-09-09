#include "procedural/SolarSystemPlacementSystem.h"

#include "celestial/OrbitalDynamicsSystem.h"
#include "celestial/SystemSpatialScale.h"
#include "rendering/CelestialEnvironmentSystem.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

namespace subspace {
namespace {
constexpr float kPi = 3.14159265358979323846f;
constexpr float kTwoPi = kPi * 2.0f;

float Dist2D(const SectorPosition& a, const SectorPosition& b) {
    const float dx=a.x-b.x,dy=a.y-b.y;
    return std::sqrt(dx*dx+dy*dy);
}
float RadiusFrom(const SectorPosition& p,const SectorPosition& c){return Dist2D(p,c);}

std::uint32_t HashString(const std::string& text, std::uint32_t salt=2166136261u) {
    std::uint32_t h=salt;
    for(unsigned char c:text){h^=c;h*=16777619u;}
    return h ? h : 1u;
}
float Unit(std::uint32_t h){return float(h & 0x00FFFFFFu)/float(0x00FFFFFFu);}
float AngleFor(const std::string& id,std::uint32_t salt=0){return Unit(HashString(id,2166136261u^salt))*kTwoPi;}
float RangeFor(const std::string& id,float lo,float hi,std::uint32_t salt){return lo+(hi-lo)*Unit(HashString(id,2166136261u^salt));}

SectorPosition Radial(const SectorPosition& center,float radius,float angle){return {center.x+std::cos(angle)*radius,center.y+std::sin(angle)*radius,0.0f};}

std::string PlanetKey(const PlanetData& p,std::size_t i){return p.planetId.empty()?"planet_"+std::to_string(i):p.planetId;}

const PlanetData* FindPlanet(const GalaxySector& sector,const std::string& id){
    for(const auto& p:sector.planets)if(p.planetId==id)return &p;
    return nullptr;
}
PlanetData* FindPlanet(GalaxySector& sector,const std::string& id){
    for(auto& p:sector.planets)if(p.planetId==id)return &p;
    return nullptr;
}

SectorPosition BeltMarker(const GalaxySector& sector,const AsteroidBeltData& belt){
    SectorPosition center=sector.star.position;
    if(belt.beltClass==AsteroidBeltClass::PlanetaryDebris){if(const auto* p=FindPlanet(sector,belt.parentPlanetId))center=p->position;}
    return Radial(center,std::max(1.0f,belt.orbitalRadius),belt.orbitalPhaseRadians);
}

bool InsidePlanetEnvelope(const GalaxySector& sector,const SectorPosition& p,const SolarSystemPlacementSystem& sys,float extra=0.0f){
    for(const auto& planet:sector.planets)if(Dist2D(p,planet.position)<sys.PlanetEnvelopeRadiusSector(planet)+extra)return true;
    return false;
}

SectorPosition ProjectOutsideStar(const GalaxySector& sector,SectorPosition p,const SolarSystemPlacementSystem& sys,float margin=0.0f){
    if(!sector.hasStar)return p;
    const float safe=sys.StellarExclusionRadiusSector(sector.star)+margin;
    float dx=p.x-sector.star.position.x,dy=p.y-sector.star.position.y,d=std::sqrt(dx*dx+dy*dy);
    if(d>=safe)return p;
    if(d<1.0f){dx=1.0f;dy=0.0f;d=1.0f;}
    return {sector.star.position.x+dx/d*safe,sector.star.position.y+dy/d*safe,0.0f};
}

SectorPosition ProjectOutsidePlanets(const GalaxySector& sector,SectorPosition p,const SolarSystemPlacementSystem& sys,float margin=0.0f){
    for(int pass=0;pass<4;++pass){
        bool moved=false;
        for(const auto& planet:sector.planets){
            const float safe=sys.PlanetEnvelopeRadiusSector(planet)+margin;
            float dx=p.x-planet.position.x,dy=p.y-planet.position.y,d=std::sqrt(dx*dx+dy*dy);
            if(d>=safe)continue;
            if(d<1.0f){dx=1.0f;dy=0.0f;d=1.0f;}
            p={planet.position.x+dx/d*safe,planet.position.y+dy/d*safe,0.0f};moved=true;
        }
        if(!moved)break;
    }
    return p;
}

SectorPosition SafePoint(const GalaxySector& sector,SectorPosition p,const SolarSystemPlacementSystem& sys,float starMargin=12000.0f,float planetMargin=8000.0f){
    p=ProjectOutsideStar(sector,p,sys,starMargin);
    p=ProjectOutsidePlanets(sector,p,sys,planetMargin);
    return ProjectOutsideStar(sector,p,sys,starMargin);
}

const SectorSiteData* FirstSite(const GalaxySector& sector, SectorSiteType a, SectorSiteType b=SectorSiteType::MiningField, bool useB=false){
    for(const auto& s:sector.pointsOfInterest)if(s.type==a||(useB&&s.type==b))return &s;
    return nullptr;
}
}

float SolarSystemPlacementSystem::StellarExclusionRadiusSector(const StarData& star) const {
    OrbitalDynamicsSystem dynamics;
    const auto e=dynamics.StarSafety(star);
    // Use the renderer's shared macro/world scale, plus a deterministic buffer
    // so transparently rendered corona/prominence effects never intersect
    // ordinary spawned content.
    return std::max(175000.0f,e.spawnRadius*SystemSpatialScale::WorldToSector+12000.0f);
}

float SolarSystemPlacementSystem::PlanetEnvelopeRadiusSector(const PlanetData& planet) const {
    CelestialEnvironmentSystem celestial;
    // SafeLocalOrbitRadius already includes the rendered body, atmosphere and
    // dense ring presentation. Convert that real flight-space envelope back to
    // macro-sector units so generation cannot place objects inside visuals.
    const float world=celestial.SafeLocalOrbitRadius(planet,18.0f);
    return std::max(7000.0f,world*SystemSpatialScale::WorldToSector);
}

float SolarSystemPlacementSystem::OuterSystemRadius(const GalaxySector& sector) const {
    float outer=sector.hasStar?StellarExclusionRadiusSector(sector.star):0.0f;
    const SectorPosition center=sector.hasStar?sector.star.position:SectorPosition{};
    for(const auto& p:sector.planets)outer=std::max(outer,RadiusFrom(p.position,center)+PlanetEnvelopeRadiusSector(p));
    for(const auto& b:sector.asteroidBelts){
        if(b.beltClass==AsteroidBeltClass::Circumstellar)outer=std::max(outer,b.outerRadius);
        else if(const auto* p=FindPlanet(sector,b.parentPlanetId))outer=std::max(outer,RadiusFrom(p->position,center)+b.outerRadius);
    }
    return outer;
}

SectorPosition SolarSystemPlacementSystem::RepairSpawnPosition(const GalaxySector& sector, SectorPosition position,
                                                               float stellarMargin, float planetMargin) const {
    if (!sector.hasStar) return position;
    return SafePoint(sector, position, *this, std::max(0.0f, stellarMargin), std::max(0.0f, planetMargin));
}

bool SolarSystemPlacementSystem::IsSpawnPositionSafe(const GalaxySector& sector, const SectorPosition& position,
                                                       float stellarMargin, float planetMargin) const {
    if (!sector.hasStar) return true;
    if (Dist2D(position, sector.star.position) < StellarExclusionRadiusSector(sector.star) + std::max(0.0f, stellarMargin)) return false;
    for (const auto& planet : sector.planets) {
        if (Dist2D(position, planet.position) < PlanetEnvelopeRadiusSector(planet) + std::max(0.0f, planetMargin)) return false;
    }
    return true;
}

SolarSystemPlacementReport SolarSystemPlacementSystem::Normalize(GalaxySector& sector) const {
    SolarSystemPlacementReport report;
    if(!sector.hasStar)return report;
    sector.star.position={0.0f,0.0f,0.0f};
    const float stellarSafe=StellarExclusionRadiusSector(sector.star);

    // Major planets: preserve deterministic phase, but guarantee their complete
    // presentation envelopes cannot touch the star or neighboring orbital bands.
    float previousOrbit=0.0f,previousEnvelope=0.0f;
    for(std::size_t i=0;i<sector.planets.size();++i){
        auto& p=sector.planets[i];
        const float env=PlanetEnvelopeRadiusSector(p);
        float orbit=RadiusFrom(p.position,sector.star.position);
        float angle=orbit>1.0f?std::atan2(p.position.y-sector.star.position.y,p.position.x-sector.star.position.x):AngleFor(PlanetKey(p,i),11u);
        const float minStar=stellarSafe+env+40000.0f;
        const float minPrev=i?previousOrbit+previousEnvelope+env+50000.0f:minStar;
        const float safeOrbit=std::max({orbit,minStar,minPrev});
        if(std::fabs(safeOrbit-orbit)>1.0f){++report.repairs;p.position=Radial(sector.star.position,safeOrbit,angle);}else p.position.z=0.0f;
        previousOrbit=safeOrbit;previousEnvelope=env;
    }

    // First-class moons are normalized relative to their actual parent and its
    // visual/ring envelope. This prevents moon markers or future local scenes
    // from materializing inside a giant planet/ring shell.
    for(std::size_t i=0;i<sector.moons.size();++i){
        auto& m=sector.moons[i];const auto* parent=FindPlanet(sector,m.parentPlanetId);if(!parent)continue;
        const float minOrbit=PlanetEnvelopeRadiusSector(*parent)+22000.0f+float(i%3)*12000.0f;
        if(m.orbitalRadius<minOrbit){m.orbitalRadius=minOrbit;++report.repairs;}
        m.position=Radial(parent->position,m.orbitalRadius,m.orbitalPhaseRadians);
    }

    // Circumstellar belt: occupy the largest genuinely empty annular gap. If
    // no internal gap can fit safely, place it beyond the outermost planet.
    for(auto& belt:sector.asteroidBelts){
        if(belt.beltClass==AsteroidBeltClass::PlanetaryDebris){
            auto* parent=FindPlanet(sector,belt.parentPlanetId);if(!parent)continue;
            const float minInner=PlanetEnvelopeRadiusSector(*parent)+30000.0f;
            if(belt.innerRadius<minInner){belt.innerRadius=minInner;++report.repairs;}
            belt.outerRadius=std::max(belt.outerRadius,belt.innerRadius+70000.0f);
            belt.orbitalRadius=(belt.innerRadius+belt.outerRadius)*0.5f;
            continue;
        }
        struct Gap{float lo=0,hi=0;};Gap best{stellarSafe+30000.0f,stellarSafe+30000.0f};
        float cursor=stellarSafe+30000.0f;
        for(const auto& p:sector.planets){
            const float orbit=RadiusFrom(p.position,sector.star.position),env=PlanetEnvelopeRadiusSector(p);
            const float hi=orbit-env-25000.0f;
            if(hi-cursor>best.hi-best.lo)best={cursor,hi};
            cursor=std::max(cursor,orbit+env+25000.0f);
        }
        const float outerFallback=std::max(cursor,OuterSystemRadius(sector)+40000.0f);
        if(best.hi-best.lo<90000.0f)best={outerFallback,outerFallback+120000.0f};
        const float pad=std::max(10000.0f,(best.hi-best.lo)*0.12f);
        const float newInner=best.lo+pad,newOuter=best.hi-pad;
        if(std::fabs(newInner-belt.innerRadius)>1.0f||std::fabs(newOuter-belt.outerRadius)>1.0f)++report.repairs;
        belt.innerRadius=newInner;belt.outerRadius=std::max(newInner+60000.0f,newOuter);belt.orbitalRadius=(belt.innerRadius+belt.outerRadius)*0.5f;
    }

    // Primary station: explicitly parent to a planet when possible, otherwise
    // use a valid stellar orbit. The old 2.6k-5.2k origin placement is retired.
    if(sector.hasStation){
        if(sector.station.stationId.empty())sector.station.stationId="station_"+std::to_string(sector.x)+"_"+std::to_string(sector.y)+"_"+std::to_string(sector.z)+"_primary";
        const PlanetData* parent=nullptr;
        if(!sector.planets.empty()){
            parent=&sector.planets.front();
            for(const auto& p:sector.planets)if(p.supportsIndustry){parent=&p;break;}
        }
        const float angle=AngleFor(sector.station.name,73u);
        if(parent){
            sector.station.parentObjectId=parent->planetId;sector.station.placementClass="PLANET_ORBIT";
            const float orbit=PlanetEnvelopeRadiusSector(*parent)+35000.0f;
            sector.station.position=Radial(parent->position,orbit,angle);
        }else{
            sector.station.parentObjectId=sector.star.starId;sector.station.placementClass="STELLAR_ORBIT";
            sector.station.position=Radial(sector.star.position,stellarSafe+90000.0f,angle);
        }
        ++report.repairs;
    }

    // Orbital hubs follow their actual parent and remain outside the complete
    // atmosphere/ring envelope.
    for(std::size_t i=0;i<sector.orbitalHubs.size();++i){auto& h=sector.orbitalHubs[i];if(const auto* p=FindPlanet(sector,h.planetId)){h.position=Radial(p->position,PlanetEnvelopeRadiusSector(*p)+22000.0f+float(i%3)*9000.0f,AngleFor(h.hubId,91u));++report.repairs;}}

    // Semantic POI placement. Nothing is randomly sprinkled around the stellar
    // origin anymore: mining lives in a belt, orbital facilities live near a
    // planet, salvage/trade live around station traffic, and anomalies live in
    // deep space.
    float deepBase=std::max(OuterSystemRadius(sector)+90000.0f,stellarSafe+180000.0f);
    for(std::size_t i=0;i<sector.pointsOfInterest.size();++i){
        auto& s=sector.pointsOfInterest[i];const float a=AngleFor(s.siteId,101u+std::uint32_t(i));
        SectorPosition target{};std::string parent;std::string placement;
        if(s.type==SectorSiteType::MiningField && !sector.asteroidBelts.empty()){
            const auto& b=sector.asteroidBelts[i%sector.asteroidBelts.size()];SectorPosition c=sector.star.position;
            if(b.beltClass==AsteroidBeltClass::PlanetaryDebris){if(const auto* p=FindPlanet(sector,b.parentPlanetId))c=p->position;}
            const float rr=RangeFor(s.siteId,b.innerRadius,b.outerRadius,131u);target=Radial(c,rr,a);parent=b.beltId;placement="BELT_REGION";
        }else if((s.type==SectorSiteType::OrbitalRelay||s.type==SectorSiteType::ResearchOutpost) && !sector.planets.empty()){
            const auto& p=sector.planets[i%sector.planets.size()];target=Radial(p.position,PlanetEnvelopeRadiusSector(p)+45000.0f+float(i%4)*12000.0f,a);parent=p.planetId;placement="PLANET_ORBIT";
        }else if((s.type==SectorSiteType::SalvageSite||s.type==SectorSiteType::DerelictYard||s.type==SectorSiteType::DistressWreck||s.type==SectorSiteType::TradeLane||s.type==SectorSiteType::IndustrialDepot) && sector.hasStation){
            target=Radial(sector.station.position,28000.0f+float(i%5)*9000.0f,a);parent=sector.station.stationId;placement="STATION_TRAFFIC";
        }else{
            target=Radial(sector.star.position,deepBase+float(i)*55000.0f,a);parent=sector.star.starId;placement="DEEP_SPACE";
        }
        target=SafePoint(sector,target,*this);s.position=target;s.parentObjectId=parent;s.placementClass=placement;++report.repairs;
    }

    // Materialized asteroid budget is retained for existing mining gameplay,
    // but every rock belongs to a real mining region. No global rocks remain at
    // the stellar origin.
    const SectorSiteData* mining=FirstSite(sector,SectorSiteType::MiningField);
    SectorPosition rockCenter=mining?mining->position:(sector.asteroidBelts.empty()?Radial(sector.star.position,deepBase,0.4f):BeltMarker(sector,sector.asteroidBelts.front()));
    const float rockRadius=std::max(180.0f,mining?mining->radius:480.0f);
    for(std::size_t i=0;i<sector.asteroids.size();++i){auto& rock=sector.asteroids[i];const std::string id="asteroid_"+std::to_string(i);const float rr=RangeFor(id,rockRadius*.12f,rockRadius*.88f,151u),a=AngleFor(id,157u);rock.position=SafePoint(sector,Radial(rockCenter,rr,a),*this);rock.regionId=mining?mining->siteId:(!sector.asteroidBelts.empty()?sector.asteroidBelts.front().beltId:"deep_space");++report.repairs;}

    // Traffic/wrecks/debris/encounters inhabit a local activity region rather
    // than the star. These remain compatible with the existing runtime
    // regional-streaming/reanchor layer.
    SectorPosition trafficCenter=sector.hasStation?sector.station.position:(!sector.pointsOfInterest.empty()?sector.pointsOfInterest.front().position:Radial(sector.star.position,deepBase,1.1f));
    for(std::size_t i=0;i<sector.ships.size();++i){auto& s=sector.ships[i];s.position=SafePoint(sector,Radial(trafficCenter,16000.0f+float(i%5)*6000.0f,AngleFor(s.shipId,171u)),*this);s.parentObjectId=sector.hasStation?sector.station.stationId:sector.star.starId;s.placementClass="LOCAL_TRAFFIC";++report.repairs;}
    const SectorSiteData* salvage=FirstSite(sector,SectorSiteType::SalvageSite,SectorSiteType::DerelictYard,true);
    SectorPosition salvageCenter=salvage?salvage->position:trafficCenter;
    for(std::size_t i=0;i<sector.derelicts.size();++i){auto& d=sector.derelicts[i];d.position=SafePoint(sector,Radial(salvageCenter,1200.0f+float(i)*850.0f,AngleFor(d.derelictId,181u)),*this);d.parentObjectId=salvage?salvage->siteId:(sector.hasStation?sector.station.stationId:sector.star.starId);++report.repairs;}
    for(std::size_t i=0;i<sector.debrisFields.size();++i){auto& d=sector.debrisFields[i];d.position=SafePoint(sector,Radial(salvageCenter,900.0f+float(i)*1100.0f,AngleFor("debris_"+std::to_string(i),191u)),*this);d.parentObjectId=salvage?salvage->siteId:(sector.hasStation?sector.station.stationId:sector.star.starId);++report.repairs;}
    for(std::size_t i=0;i<sector.encounters.size();++i){auto& e=sector.encounters[i];e.position=SafePoint(sector,Radial(trafficCenter,22000.0f+float(i)*7000.0f,AngleFor(e.encounterId,201u)),*this);e.parentObjectId=sector.hasStation?sector.station.stationId:sector.star.starId;++report.repairs;}

    // Wormholes and anomalies are deep-space objects unless a future semantic
    // archetype explicitly parents them elsewhere.
    deepBase=std::max(OuterSystemRadius(sector)+120000.0f,deepBase);
    for(std::size_t i=0;i<sector.wormholes.size();++i){auto& w=sector.wormholes[i];w.position=Radial(sector.star.position,deepBase+float(i)*85000.0f,AngleFor(w.designation,211u));w.parentObjectId=sector.star.starId;w.placementClass="DEEP_SPACE";++report.repairs;}
    for(std::size_t i=0;i<sector.anomalies.size();++i){auto& a=sector.anomalies[i];a.position=Radial(sector.star.position,deepBase+65000.0f+float(i)*95000.0f,AngleFor(a.name,223u));a.parentObjectId=sector.star.starId;a.placementClass="DEEP_SPACE";++report.repairs;}

    auto validation=Validate(sector);validation.repairs=report.repairs;return validation;
}

SolarSystemPlacementReport SolarSystemPlacementSystem::Validate(const GalaxySector& sector) const {
    SolarSystemPlacementReport r;
    if(!sector.hasStar)return r;
    const float stellar=StellarExclusionRadiusSector(sector.star);
    auto issue=[&](const std::string& text){r.certified=false;++r.violations;r.issues.push_back(text);};
    auto starCheck=[&](const SectorPosition& p,const std::string& what){if(Dist2D(p,sector.star.position)<stellar)issue(what+" intersects stellar exclusion");};

    float prevOrbit=0.0f,prevEnv=0.0f;
    for(std::size_t i=0;i<sector.planets.size();++i){const auto& p=sector.planets[i];const float orbit=Dist2D(p.position,sector.star.position),env=PlanetEnvelopeRadiusSector(p);if(orbit<stellar+env)issue("planet "+PlanetKey(p,i)+" intersects stellar exclusion");if(i&&orbit-prevOrbit<prevEnv+env+20000.0f)issue("planet orbital envelopes overlap");prevOrbit=orbit;prevEnv=env;}
    for(const auto& m:sector.moons){const auto* p=FindPlanet(sector,m.parentPlanetId);if(!p){issue("moon has missing parent");continue;}if(m.orbitalRadius<PlanetEnvelopeRadiusSector(*p))issue("moon orbit intersects parent presentation envelope");}
    for(const auto& b:sector.asteroidBelts){if(b.outerRadius<=b.innerRadius)issue("asteroid belt has inverted bounds");if(b.beltClass==AsteroidBeltClass::Circumstellar){if(b.innerRadius<stellar)issue("circumstellar belt intersects star");for(const auto& p:sector.planets){const float orbit=Dist2D(p.position,sector.star.position),env=PlanetEnvelopeRadiusSector(p);if(b.outerRadius>orbit-env&&b.innerRadius<orbit+env)issue("circumstellar belt intersects planet envelope");}}else{const auto* p=FindPlanet(sector,b.parentPlanetId);if(!p)issue("planetary debris belt missing parent");else if(b.innerRadius<PlanetEnvelopeRadiusSector(*p))issue("planetary debris belt intersects parent/rings");}}
    if(sector.hasStation){starCheck(sector.station.position,"station");if(InsidePlanetEnvelope(sector,sector.station.position,*this))issue("station intersects planet/ring envelope");}
    for(const auto& h:sector.orbitalHubs){starCheck(h.position,"orbital hub");if(InsidePlanetEnvelope(sector,h.position,*this))issue("orbital hub intersects planet/ring envelope");}
    for(const auto& s:sector.pointsOfInterest){starCheck(s.position,"POI "+s.siteId);if(InsidePlanetEnvelope(sector,s.position,*this))issue("POI intersects planet/ring envelope");}
    for(const auto& a:sector.asteroids){starCheck(a.position,"asteroid");if(InsidePlanetEnvelope(sector,a.position,*this))issue("asteroid intersects planet/ring envelope");if(a.regionId.empty())issue("asteroid has no region authority");}
    for(const auto& s:sector.ships)starCheck(s.position,"ambient ship");
    for(const auto& d:sector.derelicts)starCheck(d.position,"derelict");
    for(const auto& d:sector.debrisFields)starCheck(d.position,"debris field");
    for(const auto& e:sector.encounters)starCheck(e.position,"encounter");
    for(const auto& w:sector.wormholes)starCheck(w.position,"wormhole");
    for(const auto& a:sector.anomalies)starCheck(a.position,"anomaly");
    return r;
}

} // namespace subspace
