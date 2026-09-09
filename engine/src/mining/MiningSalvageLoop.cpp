#include "mining/MiningSalvageLoop.h"

#include <cmath>

namespace subspace {

void MiningSalvageLoop::Initialize(const GalaxySector& sector)
{
    _missiles.Clear(); _fracture.Initialize(sector); _telemetry={}; _lastProcessedDetonation=0; _recoveredDerelicts.clear();
}

std::uint64_t MiningSalvageLoop::LaunchMiningMissile(const Vector3& position,
                                                      const Vector3& velocity,
                                                      const Vector3& forward,
                                                      const Vector3& target)
{
    ++_telemetry.miningMissilesLaunched;
    return _missiles.Launch(position,velocity,forward,target,MissilePayloadType::MiningFracture,true);
}

void MiningSalvageLoop::Update(float deltaTime,const Vector3& playerPosition,GalaxySector& sector)
{
    _missiles.Update(deltaTime);
    for(const auto& detonation:_missiles.GetDetonations()){
        if(detonation.missileId<=_lastProcessedDetonation) continue;
        _lastProcessedDetonation=detonation.missileId;
        auto result=_fracture.ApplyDetonation(detonation,sector);
        if(result.fractured) ++_telemetry.asteroidsFractured;
    }
    _fracture.Update(deltaTime);
    ResourceType type=_telemetry.lastRecoveredType;
    const float recovered=_fracture.RecoverNearby(playerPosition,1.20f,&type);
    if(recovered>0.0f){_telemetry.oreRecovered+=recovered;_telemetry.lastRecoveredType=type;}

    constexpr float sectorToWorld=0.0037f;
    for(const auto& derelict:sector.derelicts){
        if(_recoveredDerelicts.count(derelict.derelictId)!=0) continue;
        const Vector3 p{derelict.position.x*sectorToWorld,derelict.position.y*sectorToWorld,0.0f};
        const float dx=p.x-playerPosition.x, dy=p.y-playerPosition.y;
        if(dx*dx+dy*dy<=1.55f*1.55f){
            _recoveredDerelicts.insert(derelict.derelictId);
            _telemetry.salvageRecovered+=std::max(1.0f,derelict.salvageValue*0.08f);
        }
    }
}

} // namespace subspace
