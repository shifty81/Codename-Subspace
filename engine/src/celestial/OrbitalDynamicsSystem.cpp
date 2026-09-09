#include "celestial/OrbitalDynamicsSystem.h"
#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
constexpr double kPi=3.14159265358979323846;
double Rad(double d){return d*kPi/180.0;}
double SolveEccentric(double mean,double e){double E=mean;for(int i=0;i<7;++i){const double f=E-e*std::sin(E)-mean;const double fp=1.0-e*std::cos(E);E-=f/std::max(1e-8,fp);}return E;}
float Dist(const Vector3&a,const Vector3&b){return (a-b).length();}
}

Vector3 OrbitalDynamicsSystem::Evaluate(const OrbitalBodyRecord& b,double t,const Vector3& parent) const {
    if(b.kind==OrbitalBodyKind::Star||b.orbit.semiMajorAxis<=0.0||b.orbit.orbitalPeriodSeconds<=0.0)return parent;
    const double M=Rad(b.orbit.meanAnomalyAtEpochDegrees)+2.0*kPi*((t-b.orbit.epochSeconds)/b.orbit.orbitalPeriodSeconds);
    const double e=std::clamp(b.orbit.eccentricity,0.0,0.92);
    const double E=SolveEccentric(M,e);
    const double x0=b.orbit.semiMajorAxis*(std::cos(E)-e);
    const double y0=b.orbit.semiMajorAxis*std::sqrt(1.0-e*e)*std::sin(E);
    const double w=Rad(b.orbit.argumentOfPeriapsisDegrees), O=Rad(b.orbit.longitudeAscendingNodeDegrees), inc=Rad(b.orbit.inclinationDegrees);
    const double cw=std::cos(w),sw=std::sin(w),cO=std::cos(O),sO=std::sin(O),ci=std::cos(inc),si=std::sin(inc);
    const double xp=x0*cw-y0*sw, yp=x0*sw+y0*cw;
    const double x=xp*cO-yp*ci*sO, y=xp*sO+yp*ci*cO, z=yp*si;
    return {parent.x+static_cast<float>(x),parent.y+static_cast<float>(y),parent.z+static_cast<float>(z)};
}

StellarSafetyEnvelope OrbitalDynamicsSystem::StarSafety(const StarData& star) const {
    StellarSafetyEnvelope e;
    e.visualRadius=std::max(90.0f,star.radius*0.22f);
    e.coronaRadius=e.visualRadius*1.55f;
    e.hazardRadius=e.visualRadius*2.2f;
    e.spawnRadius=e.visualRadius*3.6f+650.0f;
    return e;
}
bool OrbitalDynamicsSystem::IsOutsideStarSafety(const Vector3& p,const Vector3& star,const StellarSafetyEnvelope& e) const{return Dist(p,star)>=e.spawnRadius;}
OrbitalIntercept OrbitalDynamicsSystem::PredictIntercept(const OrbitalBodyRecord& b,const Vector3& parent,double now,double travel) const {OrbitalIntercept o;o.valid=travel>=0;o.arrivalTimeSeconds=now+std::max(0.0,travel);o.predictedTarget=Evaluate(b,o.arrivalTimeSeconds,parent);o.missDistance=0;return o;}

std::vector<OrbitalBodyRecord> OrbitalDynamicsSystem::DeriveSystemOrbits(const GalaxySector& sector) const {
    std::vector<OrbitalBodyRecord> out;
    const std::uint64_t starId=1;
    if(sector.hasStar)out.push_back({starId,0,sector.star.name,OrbitalBodyKind::Star,{},sector.star.radius,false});

    // Major planets use compressed Kepler-like periods. The ratios follow
    // a^(3/2), while the absolute time scale is intentionally game-readable.
    // This keeps inner worlds moving faster than outer worlds without making
    // players wait real-world months to notice orbital change.
    for(std::size_t i=0;i<sector.planets.size();++i){
        const auto&p=sector.planets[i];
        const double a=std::max(2500.0,std::sqrt(double(p.position.x*p.position.x+p.position.y*p.position.y)));
        OrbitalElements e;e.semiMajorAxis=a;e.eccentricity=0.006+0.010*(i%5);
        e.inclinationDegrees=(int(i)%7-3)*1.35;
        e.longitudeAscendingNodeDegrees=std::fmod(31.0*double(i)+17.0,360.0);
        e.argumentOfPeriapsisDegrees=std::fmod(47.0*double(i)+9.0,360.0);
        e.meanAnomalyAtEpochDegrees=std::fmod(std::atan2(p.position.y,p.position.x)*180.0/kPi+360.0,360.0);
        e.orbitalPeriodSeconds=std::max(1800.0,std::pow(a/2600.0,1.5)*2100.0);
        const std::uint64_t planetId=100+i;
        out.push_back({planetId,starId,p.name,OrbitalBodyKind::Planet,e,p.radius,false});

        // Pass496: consume persistent first-class moon records when present.
        // Historical hand-authored/test sectors that predate GalaxySector::moons
        // retain the deterministic fallback below.
        bool hasFirstClassMoons=false;
        for(const auto& moon:sector.moons){
            if(moon.parentPlanetId!=p.planetId)continue;
            hasFirstClassMoons=true;
            OrbitalElements me;
            me.semiMajorAxis=std::max(480.0,double(moon.orbitalRadius));
            const std::uint32_t mh=static_cast<std::uint32_t>(moon.surfaceSeed)*1664525u+1013904223u;
            me.eccentricity=.004+.006*((mh>>3)&3u);
            me.inclinationDegrees=-4.0+double((mh>>5)%9u);
            me.longitudeAscendingNodeDegrees=std::fmod(double((mh>>8)%360u),360.0);
            me.argumentOfPeriapsisDegrees=std::fmod(double((mh>>12)%360u),360.0);
            me.meanAnomalyAtEpochDegrees=std::fmod(double(moon.orbitalPhaseRadians)*180.0/kPi+360.0,360.0);
            me.orbitalPeriodSeconds=std::max(220.0,320.0*std::pow(me.semiMajorAxis/std::max(480.0,double(p.radius)*2.5),1.5));
            const std::uint64_t moonId=1000+out.size();
            out.push_back({moonId,planetId,moon.name,OrbitalBodyKind::Moon,me,moon.radius,false});
        }
        if(!hasFirstClassMoons){
            const std::uint32_t moonHash=static_cast<std::uint32_t>((p.surfaceSeed*1664525u)+1013904223u+static_cast<unsigned>(i)*97u);
            const int moonCount=(p.type==PlanetType::GasGiant)?(1+int(moonHash%3u)):int((moonHash>>3)%3u);
            for(int m=0;m<moonCount;++m){
                OrbitalElements me;
                me.semiMajorAxis=std::max(480.0,double(p.radius)*(2.5+1.35*m));
                me.eccentricity=.004+.006*((moonHash>>(m*3))&3u);
                me.inclinationDegrees=-4.0+double((moonHash>>(m*5))%9u);
                me.longitudeAscendingNodeDegrees=std::fmod(63.0*m+double(moonHash%71u),360.0);
                me.argumentOfPeriapsisDegrees=std::fmod(29.0*m+double((moonHash>>8)%83u),360.0);
                me.meanAnomalyAtEpochDegrees=std::fmod(73.0*m+double((moonHash>>12)%360u),360.0);
                me.orbitalPeriodSeconds=std::max(220.0,320.0*std::pow(me.semiMajorAxis/std::max(480.0,double(p.radius)*2.5),1.5));
                const std::uint64_t moonId=1000+i*8+static_cast<std::size_t>(m);
                out.push_back({moonId,planetId,p.name+" Moon "+std::to_string(m+1),OrbitalBodyKind::Moon,me,std::max(24.0f,p.radius*(.10f+.035f*m)),false});
            }
        }
    }

    // Pass410R3 macro asteroid belts are first-class orbital entities. The
    // orbit track represents the belt's mean radius; local region streaming
    // expands that track into a wide, dense mineable band on demand.
    for(std::size_t i=0;i<sector.asteroidBelts.size();++i){
        const auto& belt=sector.asteroidBelts[i];
        std::uint64_t parent=starId;
        double parentScale=std::max(1.0f,belt.orbitalRadius);
        if(belt.beltClass==AsteroidBeltClass::PlanetaryDebris){
            for(std::size_t p=0;p<sector.planets.size();++p){
                if(sector.planets[p].planetId==belt.parentPlanetId){parent=100+p;parentScale=std::max(1.0f,belt.orbitalRadius);break;}
            }
        }
        OrbitalElements e;
        e.semiMajorAxis=std::max(800.0,parentScale);
        e.eccentricity=belt.beltClass==AsteroidBeltClass::Circumstellar?.018:.006;
        e.inclinationDegrees=belt.beltClass==AsteroidBeltClass::Circumstellar?2.4:1.2;
        e.longitudeAscendingNodeDegrees=std::fmod(19.0*double(i)+13.0,360.0);
        e.argumentOfPeriapsisDegrees=std::fmod(37.0*double(i)+7.0,360.0);
        e.meanAnomalyAtEpochDegrees=std::fmod(double(belt.orbitalPhaseRadians)*180.0/kPi+360.0,360.0);
        if(belt.beltClass==AsteroidBeltClass::Circumstellar)
            e.orbitalPeriodSeconds=std::max(2200.0,std::pow(e.semiMajorAxis/2600.0,1.5)*2100.0);
        else
            e.orbitalPeriodSeconds=std::max(420.0,420.0*std::pow(e.semiMajorAxis/1200.0,1.5));
        out.push_back({6000+i,parent,belt.name,OrbitalBodyKind::BeltObject,e,
                       std::max(80.0f,(belt.outerRadius-belt.innerRadius)*.5f),false});
    }

    if(sector.hasStation){
        std::uint64_t parent=starId;
        const PlanetData* parentPlanet=nullptr;
        for(std::size_t p=0;p<sector.planets.size();++p)if(sector.planets[p].planetId==sector.station.parentObjectId){parent=100+p;parentPlanet=&sector.planets[p];break;}
        const SectorPosition center=parentPlanet?parentPlanet->position:sector.star.position;
        const float dx=sector.station.position.x-center.x,dy=sector.station.position.y-center.y;
        OrbitalElements e;e.semiMajorAxis=std::max(900.0,double(std::sqrt(dx*dx+dy*dy)));
        e.eccentricity=0.002;e.inclinationDegrees=1.8;e.orbitalPeriodSeconds=parentPlanet?980.0:7600.0;
        e.meanAnomalyAtEpochDegrees=std::fmod(std::atan2(dy,dx)*180.0/kPi+360.0,360.0);
        out.push_back({300,parent,sector.station.name,OrbitalBodyKind::Station,e,140.0f,true});
    }
    for(std::size_t i=0;i<sector.orbitalHubs.size();++i){
        const auto& hub=sector.orbitalHubs[i];std::uint64_t parent=starId;float parentRadius=260.0f;
        for(std::size_t p=0;p<sector.planets.size();++p)if(sector.planets[p].planetId==hub.planetId){parent=100+p;parentRadius=sector.planets[p].radius;break;}
        OrbitalElements e;e.semiMajorAxis=std::max(760.0,double(parentRadius)*4.2+240.0*i);e.eccentricity=.0015;e.inclinationDegrees=(int(i)%3-1)*2.0;e.meanAnomalyAtEpochDegrees=115.0+61.0*i;e.orbitalPeriodSeconds=1180.0+160.0*i;
        out.push_back({400+i,parent,hub.hubId,OrbitalBodyKind::Station,e,90.0f,true});
    }
    return out;
}

} // namespace subspace
