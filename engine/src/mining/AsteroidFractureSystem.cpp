#include "mining/AsteroidFractureSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
constexpr float kSectorToWorld = 0.0037f;
constexpr float kPi = 3.14159265358979323846f;

Vector3 ToWorld(const SectorPosition& p)
{
    return {p.x*kSectorToWorld,p.y*kSectorToWorld,0.0f};
}

float Distance2D(const Vector3& a,const Vector3& b)
{
    const float dx=a.x-b.x,dy=a.y-b.y;
    return std::sqrt(dx*dx+dy*dy);
}

std::uint32_t Hash(std::uint32_t x)
{
    x ^= x >> 16; x *= 0x7feb352du; x ^= x >> 15; x *= 0x846ca68bu; x ^= x >> 16; return x;
}
}

void AsteroidFractureSystem::Initialize(const GalaxySector& sector)
{
    Clear();
    _states.reserve(sector.asteroids.size());
    for(std::size_t i=0;i<sector.asteroids.size();++i){
        AsteroidFractureState state;
        state.asteroidIndex=i;
        state.integrity=std::max(18.0f,sector.asteroids[i].size*2.1f);
        _states.push_back(state);
    }
}

void AsteroidFractureSystem::Clear()
{
    _states.clear(); _fragments.clear(); _dustClouds.clear();
    _nextFragmentId=1; _recoveredResourceUnits=0.0f;
}

bool AsteroidFractureSystem::IsFractured(std::size_t asteroidIndex) const
{
    return asteroidIndex<_states.size() && _states[asteroidIndex].fractured;
}

float AsteroidFractureSystem::GetIntegrity(std::size_t asteroidIndex) const
{
    return asteroidIndex<_states.size() ? _states[asteroidIndex].integrity : 0.0f;
}

void AsteroidFractureSystem::SpawnFragments(std::size_t asteroidIndex,
                                            const AsteroidData& asteroid,
                                            const Vector3& impactPosition,
                                            float energy)
{
    const Vector3 center=ToWorld(asteroid.position);
    const std::uint32_t seed=Hash(static_cast<std::uint32_t>(asteroidIndex*977u + static_cast<std::size_t>(asteroid.size*31.0f)));
    const int count=std::clamp(5+static_cast<int>(energy/24.0f),5,14);
    const float totalUnits=std::max(5.0f,asteroid.size*0.85f);
    for(int i=0;i<count;++i){
        const std::uint32_t h=Hash(seed+static_cast<std::uint32_t>(i*193u));
        const float a=(static_cast<float>(h&0xffffu)/65535.0f)*2.0f*kPi;
        const float speed=0.55f+1.8f*static_cast<float>((h>>16)&0xffu)/255.0f;
        MiningFragment f;
        f.fragmentId=_nextFragmentId++;
        f.sourceAsteroidIndex=asteroidIndex;
        f.position=center + Vector3{std::cos(a)*0.22f,std::sin(a)*0.22f,0.05f+0.10f*(i%3)};
        // Bias ejecta away from impact while retaining radial breakup.
        Vector3 radial{std::cos(a),std::sin(a),0};
        Vector3 fromImpact=center-impactPosition; fromImpact.z=0;
        if(fromImpact.length()>0.01f) radial=(radial*0.68f+fromImpact.normalized()*0.32f).normalized();
        f.velocity=radial*speed;
        f.size=0.10f+0.22f*static_cast<float>((h>>24)&0xffu)/255.0f;
        f.resourceUnits=totalUnits/static_cast<float>(count);
        f.resourceType=asteroid.resourceType;
        _fragments.push_back(f);
    }
    MiningDustCloud cloud;
    cloud.position=center; cloud.baseRadius=0.75f+asteroid.size*0.012f;
    cloud.lifetime=4.5f+std::min(3.0f,energy*0.012f); cloud.density=0.55f+std::min(0.35f,energy*0.0025f); cloud.seed=seed;
    _dustClouds.push_back(cloud);
}

AsteroidFractureResult AsteroidFractureSystem::ApplyDetonation(const MissileDetonation& detonation,
                                                               const GalaxySector& sector)
{
    AsteroidFractureResult result;
    if(detonation.fractureEnergy<=0.0f || _states.size()!=sector.asteroids.size()) return result;
    float bestDistance=detonation.radius+1.25f;
    std::size_t best=sector.asteroids.size();
    for(std::size_t i=0;i<sector.asteroids.size();++i){
        if(_states[i].fractured) continue;
        const float d=Distance2D(ToWorld(sector.asteroids[i].position),detonation.position);
        if(d<bestDistance){bestDistance=d;best=i;}
    }
    if(best>=sector.asteroids.size()) return result;

    auto& state=_states[best];
    const float falloff=std::clamp(1.0f-bestDistance/std::max(0.01f,detonation.radius+1.25f),0.22f,1.0f);
    state.integrity=std::max(0.0f,state.integrity-detonation.fractureEnergy*falloff);
    result.hit=true; result.asteroidIndex=best;
    if(state.integrity<=0.0f){
        state.fractured=true; result.fractured=true;
        const std::size_t before=_fragments.size();
        SpawnFragments(best,sector.asteroids[best],detonation.position,detonation.fractureEnergy);
        result.fragmentsSpawned=static_cast<int>(_fragments.size()-before);
        for(std::size_t i=before;i<_fragments.size();++i) result.resourceUnitsReleased+=_fragments[i].resourceUnits;
    }
    return result;
}

void AsteroidFractureSystem::Update(float deltaTime)
{
    deltaTime=std::max(0.0f,deltaTime);
    for(auto& f:_fragments){
        if(f.recovered) continue;
        f.position=f.position+f.velocity*deltaTime;
        const float drag=std::exp(-0.18f*deltaTime);
        f.velocity=f.velocity*drag;
        f.position.z=std::max(0.03f,f.position.z);
    }
    for(auto& cloud:_dustClouds) cloud.age+=deltaTime;
    _dustClouds.erase(std::remove_if(_dustClouds.begin(),_dustClouds.end(),
        [](const MiningDustCloud& c){return c.age>=c.lifetime;}),_dustClouds.end());
}

float AsteroidFractureSystem::RecoverNearby(const Vector3& position,float radius,ResourceType* lastType)
{
    float recovered=0.0f;
    for(auto& f:_fragments){
        if(f.recovered || Distance2D(f.position,position)>radius) continue;
        f.recovered=true; recovered+=f.resourceUnits; if(lastType)*lastType=f.resourceType;
    }
    _recoveredResourceUnits+=recovered;
    return recovered;
}

} // namespace subspace
