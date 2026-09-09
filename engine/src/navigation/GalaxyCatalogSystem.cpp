#include "navigation/GalaxyCatalogSystem.h"
#include <algorithm>
#include <cmath>
namespace subspace {
namespace {std::uint64_t Mix(std::uint64_t x){x+=0x9e3779b97f4a7c15ULL;x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;x=(x^(x>>27))*0x94d049bb133111ebULL;return x^(x>>31);}float U(std::uint64_t x){return float(Mix(x)&0xffffff)/float(0xffffff);}}
std::vector<GalaxySystemRecord> GalaxyCatalogSystem::Generate(std::uint64_t seed,std::size_t count) const {count=std::clamp<std::size_t>(count,1,10000);std::vector<GalaxySystemRecord> out;out.reserve(count);for(std::size_t i=0;i<count;++i){const float angle=U(seed+i*17)*6.2831853f+float(i%4)*1.5708f;const float radius=120.0f+U(seed+i*29)*4880.0f;const float armWave=std::sin(angle*2.0f+radius*.003f)*120.0f;GalaxySystemRecord r;r.id=std::uint32_t(i+1);r.name="SYS-"+std::to_string(r.id);r.x=std::cos(angle)*radius+std::cos(angle*2.7f)*armWave;r.y=std::sin(angle)*radius+std::sin(angle*2.7f)*armWave;r.z=(U(seed+i*43)-.5f)*220.0f;r.star=static_cast<GalaxyStarKind>(int(U(seed+i*59)*5)%5);r.security=U(seed+i*71);r.economy=U(seed+i*83);r.resourceRichness=U(seed+i*97);r.populationTier=int(U(seed+i*101)*6);r.discovered=i==0;r.hasShipyard=r.populationTier>=4&&r.economy>.55f;r.hasAnomaly=U(seed+i*127)>.965f;out.push_back(r);}return out;}
const GalaxySystemRecord* GalaxyCatalogSystem::Find(const std::vector<GalaxySystemRecord>&c,std::uint32_t id) const {for(const auto&r:c)if(r.id==id)return &r;return nullptr;}
} // namespace subspace
