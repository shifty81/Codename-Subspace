#include "procedural/SolarSystemEcologySystem.h"
#include "procedural/SolarSystemPlacementSystem.h"
#include <algorithm>
#include <cmath>
namespace subspace {
SolarSystemEcologyReport SolarSystemEcologySystem::Audit(const GalaxySector&s) const{
    SolarSystemEcologyReport r;SolarSystemPlacementSystem placement;
    auto safe=[&](const SectorPosition&p){if(!placement.IsSpawnPositionSafe(s,p)){++r.unsafeContacts;return false;}return true;};
    if(s.hasStation)safe(s.station.position);
    for(const auto&x:s.ships){safe(x.position);++r.localTraffic;}
    for(const auto&x:s.encounters){safe(x.position);r.localTraffic+=std::max(1,x.shipCount);}
    for(const auto&x:s.derelicts){safe(x.position);++r.salvageContacts;}
    for(const auto&x:s.debrisFields){safe(x.position);++r.salvageContacts;}
    for(const auto&x:s.pointsOfInterest){safe(x.position);if(x.type==SectorSiteType::SalvageSite||x.type==SectorSiteType::DerelictYard||x.type==SectorSiteType::DistressWreck)++r.salvageContacts;}
    r.resourceRegions=static_cast<int>(s.asteroidBelts.size());
    int bands=0;if(!s.planets.empty())++bands;if(!s.asteroidBelts.empty())++bands;if(s.hasStation||!s.ships.empty())++bands;if(!s.pointsOfInterest.empty()||!s.derelicts.empty())++bands;r.occupiedBands=bands;
    const float safety=r.unsafeContacts==0?1.0f:std::max(0.0f,1.0f-r.unsafeContacts*.15f);
    const float variety=std::min(1.0f,float(bands)/4.0f);
    const float life=std::min(1.0f,float(r.localTraffic+r.salvageContacts+r.resourceRegions)/8.0f);
    r.distributionScore=safety*.55f+variety*.25f+life*.20f;
    if(r.unsafeContacts)r.issues.push_back("contacts overlap celestial exclusion envelopes");
    if(!s.planets.empty()&&bands<2)r.issues.push_back("system content collapsed into too few orbital bands");
    if(s.hasStation&&r.localTraffic==0)r.issues.push_back("inhabited station has no local traffic");
    if(r.distributionScore<.70f)r.issues.push_back("system ecology/distribution score below production threshold");
    r.certified=r.issues.empty();return r;
}
}
