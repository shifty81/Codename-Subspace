#include "economy/PlanetaryIndustrySystem.h"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace subspace {
namespace {
std::uint32_t Mix(std::uint32_t x){x^=x>>16;x*=0x7feb352dU;x^=x>>15;x*=0x846ca68bU;x^=x>>16;return x;}
float U(std::uint32_t x){return (Mix(x)&0xffff)/65535.0f;}
bool SameHex(const HexCoord&a,const HexCoord&b){return a.q==b.q&&a.r==b.r;}
bool HasInstallation(const PlanetaryIndustryState& s,HexCoord c){return std::any_of(s.installations.begin(),s.installations.end(),[&](const PiInstallation& i){return SameHex(i.hex,c);});}
}

std::string PiSectorIdentity::StableId() const {std::ostringstream out;out<<planetId<<":q"<<coord.q<<":r"<<coord.r;return out.str();}

PlanetaryIndustryState PlanetaryIndustrySystem::Generate(const PlanetData&p,int rad,std::uint32_t seed) const {
    PlanetaryIndustryState s;s.representation=p.industryRepresentation;s.planetId=!p.planetId.empty()?p.planetId:(!p.name.empty()?p.name:"planet");rad=std::clamp(rad,1,12);
    for(int q=-rad;q<=rad;++q)for(int r=std::max(-rad,-q-rad);r<=std::min(rad,-q+rad);++r){PiHexData h;h.coord={q,r};h.resource=std::clamp(p.resourceRichness*(.55f+U(seed+q*127+r*313)*.9f),0.0f,1.0f);h.hazard=std::clamp(p.hazardLevel*(.6f+U(seed+q*17+r*43)*.8f),0.0f,1.0f);h.buildability=std::clamp(1.0f-h.hazard*.65f,0.05f,1.0f);h.surveyed=(q==0&&r==0);h.claimState=h.surveyed?PiClaimState::Surveyed:PiClaimState::Unsurveyed;h.perimeter=HexDistance(h.coord,{})==rad;s.hexes[h.coord]=h;}
    s.environmentTier=p.hazardLevel>.75f?4:p.hazardLevel>.5f?3:p.hazardLevel>.3f?2:1;return s;
}

bool PlanetaryIndustrySystem::Place(PlanetaryIndustryState&s,PiInstallation i) const {auto it=s.hexes.find(i.hex);if(it==s.hexes.end()||!it->second.surveyed||it->second.buildability<.15f)return false;if(s.representation==PlanetIndustryRepresentation::AtmosphericCollectorRing&&i.kind!=PiInstallationKind::AtmosphericCollector&&i.kind!=PiInstallationKind::Storage&&i.kind!=PiInstallationKind::Power&&i.kind!=PiInstallationKind::Logistics)return false;if(HasInstallation(s,i.hex))return false;if(i.id==0)i.id=s.installations.size()+1;s.installations.push_back(i);return true;}

bool PlanetaryIndustrySystem::PlaceGoverned(PlanetaryIndustryState&s,PiInstallation i,std::uint64_t ownerId) const {if(ownerId==0)return false;auto it=s.hexes.find(i.hex);if(it==s.hexes.end()||it->second.claimState!=PiClaimState::Developed||it->second.ownerId!=ownerId)return false;return Place(s,i);}

PiValidation PlanetaryIndustrySystem::Validate(const PlanetaryIndustryState&s,float hazard) const {PiValidation v;bool hasStorage=false,hasLogistics=false;int defenses=0;for(const auto&i:s.installations){if(!i.active)continue;v.production+=std::max(0.0,i.outputPerHour-i.inputPerHour);v.powerBalance+=i.power;if(i.kind==PiInstallationKind::Storage)hasStorage=true;if(i.kind==PiInstallationKind::Logistics)hasLogistics=true;if(i.kind==PiInstallationKind::Defense)++defenses;}if(!hasStorage)v.errors.push_back("planetary industry requires storage");if(!hasLogistics)v.errors.push_back("planetary industry requires logistics");if(v.powerBalance<0)v.errors.push_back("planetary power deficit");if(hazard>.55f&&defenses==0){v.protectedEnough=false;v.errors.push_back("hostile planet requires perimeter defense");}v.valid=v.errors.empty();return v;}

double PlanetaryIndustrySystem::TransferToTether(PlanetaryIndustryState&s,double produced,double hours) const {const double moved=std::max(0.0,std::min(produced,s.tetherThroughputPerHour*std::max(0.0,hours)));s.tetherStorage+=moved;return moved;}

bool PlanetaryIndustrySystem::SurveySector(PlanetaryIndustryState& state,HexCoord coord) const {auto it=state.hexes.find(coord);if(it==state.hexes.end())return false;auto&h=it->second;if(h.claimState!=PiClaimState::Unsurveyed){h.surveyed=true;return true;}h.surveyed=true;h.claimState=PiClaimState::Surveyed;return true;}

int PlanetaryIndustrySystem::HexDistance(HexCoord a,HexCoord b){const int aq=a.q-b.q,ar=a.r-b.r,as=(-a.q-a.r)-(-b.q-b.r);return (std::abs(aq)+std::abs(ar)+std::abs(as))/2;}
bool PlanetaryIndustrySystem::Adjacent(HexCoord a,HexCoord b){return HexDistance(a,b)==1;}

bool PlanetaryIndustrySystem::CanClaim(const PlanetaryIndustryState& state,HexCoord coord,std::uint64_t ownerId) const {if(ownerId==0)return false;auto it=state.hexes.find(coord);if(it==state.hexes.end())return false;const auto&h=it->second;if(h.claimState!=PiClaimState::Surveyed||!h.surveyed)return false;bool ownsAny=false,adjacentOwned=false;for(const auto&kv:state.hexes){const auto&other=kv.second;if(other.ownerId!=ownerId)continue;if(other.claimState!=PiClaimState::Claimed&&other.claimState!=PiClaimState::Developed)continue;ownsAny=true;adjacentOwned|=Adjacent(coord,other.coord);}return !ownsAny||adjacentOwned;}

bool PlanetaryIndustrySystem::ClaimSector(PlanetaryIndustryState& state,HexCoord coord,std::uint64_t ownerId) const {if(!CanClaim(state,coord,ownerId))return false;auto&h=state.hexes[coord];h.ownerId=ownerId;h.claimState=PiClaimState::Claimed;h.surveyed=true;return true;}

bool PlanetaryIndustrySystem::DevelopSector(PlanetaryIndustryState& state,HexCoord coord,std::uint64_t ownerId) const {auto it=state.hexes.find(coord);if(it==state.hexes.end())return false;auto&h=it->second;if(h.claimState!=PiClaimState::Claimed||h.ownerId!=ownerId||h.buildability<.15f)return false;h.claimState=PiClaimState::Developed;h.surveyed=true;return true;}

PiSectorCommandResult PlanetaryIndustrySystem::AdvanceSector(PlanetaryIndustryState& state,HexCoord coord,std::uint64_t ownerId) const {PiSectorCommandResult result;auto it=state.hexes.find(coord);if(it==state.hexes.end()){result.status="SECTOR NOT FOUND";return result;}result.before=it->second.claimState;switch(it->second.claimState){case PiClaimState::Unsurveyed:result.changed=SurveySector(state,coord);result.status=result.changed?"SECTOR SURVEYED":"SURVEY FAILED";break;case PiClaimState::Surveyed:result.changed=ClaimSector(state,coord,ownerId);result.status=result.changed?"SECTOR CLAIMED":"CLAIM REQUIRES CONTIGUOUS OWNED BORDER";break;case PiClaimState::Claimed:result.changed=DevelopSector(state,coord,ownerId);result.status=result.changed?"SECTOR DEVELOPMENT AUTHORIZED":"DEVELOPMENT BLOCKED";break;case PiClaimState::Developed:result.status="SECTOR ALREADY DEVELOPED";break;}result.after=state.hexes[coord].claimState;return result;}

std::vector<HexCoord> PlanetaryIndustrySystem::ClaimFrontier(const PlanetaryIndustryState& state,std::uint64_t ownerId) const {std::vector<HexCoord>out;for(const auto&kv:state.hexes){const auto&h=kv.second;if(h.claimState==PiClaimState::Surveyed&&CanClaim(state,h.coord,ownerId))out.push_back(h.coord);}return out;}
PiSectorIdentity PlanetaryIndustrySystem::Identity(const PlanetaryIndustryState& state,HexCoord coord) const {return {state.planetId,coord};}

const char* PlanetaryIndustrySystem::ClaimStateName(PiClaimState state){switch(state){case PiClaimState::Unsurveyed:return"UNSURVEYED";case PiClaimState::Surveyed:return"SURVEYED";case PiClaimState::Claimed:return"CLAIMED";case PiClaimState::Developed:return"DEVELOPED";}return"UNKNOWN";}
const char* PlanetaryIndustrySystem::OverlayName(PiOverlayMode mode){switch(mode){case PiOverlayMode::Resources:return"RESOURCES";case PiOverlayMode::Ownership:return"OWNERSHIP";case PiOverlayMode::Industry:return"INDUSTRY";case PiOverlayMode::Logistics:return"LOGISTICS";case PiOverlayMode::Power:return"POWER";case PiOverlayMode::Hazard:return"HAZARD";}return"RESOURCES";}
const char* PlanetaryIndustrySystem::ProjectionName(PiProjectionMode mode){return mode==PiProjectionMode::Globe?"GLOBE":"SECTOR";}

} // namespace subspace
