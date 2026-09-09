#include "weapons/MissileSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
float Distance2D(const Vector3& a, const Vector3& b)
{
    const float dx = a.x-b.x;
    const float dy = a.y-b.y;
    return std::sqrt(dx*dx+dy*dy);
}

Vector3 PlanarNormalized(const Vector3& v, const Vector3& fallback)
{
    Vector3 p{v.x,v.y,0.0f};
    const float len = p.length();
    if (len <= 1.0e-5f) return fallback;
    return p*(1.0f/len);
}
}

MissileDefinition MissileSystem::DefinitionFor(MissilePayloadType payload)
{
    MissileDefinition d;
    d.payload = payload;
    switch (payload) {
        case MissilePayloadType::HighExplosive:
            d.launchSpeed=13.0f; d.acceleration=12.0f; d.maxSpeed=36.0f;
            d.guidanceStrength=5.5f; d.proximityFuse=0.72f; d.lifetime=8.0f;
            d.detonationRadius=1.55f; d.impulse=3600.0f; d.damage=110.0f;
            d.fractureEnergy=18.0f; d.particleBudget=120; break;
        case MissilePayloadType::MiningFracture:
            d.launchSpeed=8.5f; d.acceleration=5.5f; d.maxSpeed=20.0f;
            d.guidanceStrength=2.8f; d.proximityFuse=0.62f; d.lifetime=10.0f;
            d.detonationRadius=2.25f; d.impulse=1800.0f; d.damage=18.0f;
            d.fractureEnergy=125.0f; d.particleBudget=180; break;
        case MissilePayloadType::SalvageBreacher:
            d.launchSpeed=7.0f; d.acceleration=4.0f; d.maxSpeed=17.0f;
            d.guidanceStrength=3.2f; d.proximityFuse=0.48f; d.lifetime=10.0f;
            d.detonationRadius=1.25f; d.impulse=850.0f; d.damage=10.0f;
            d.fractureEnergy=55.0f; d.particleBudget=100; break;
    }
    return d;
}

std::uint64_t MissileSystem::Launch(const Vector3& position,
                                    const Vector3& launcherVelocity,
                                    const Vector3& forward,
                                    const Vector3& targetPosition,
                                    MissilePayloadType payload,
                                    bool guided)
{
    const MissileDefinition d = DefinitionFor(payload);
    MissileProjectile m;
    m.id = _nextId++;
    m.payload = payload;
    m.position = {position.x,position.y,0.0f};
    m.previousPosition = m.position;
    const Vector3 dir = PlanarNormalized(forward,{0,1,0});
    m.velocity = {launcherVelocity.x + dir.x*d.launchSpeed,
                  launcherVelocity.y + dir.y*d.launchSpeed,0.0f};
    m.targetPosition = {targetPosition.x,targetPosition.y,0.0f};
    m.lifetime=d.lifetime; m.acceleration=d.acceleration; m.maxSpeed=d.maxSpeed;
    m.guidanceStrength=d.guidanceStrength; m.proximityFuse=d.proximityFuse;
    m.detonationRadius=d.detonationRadius; m.impulse=d.impulse; m.damage=d.damage;
    m.fractureEnergy=d.fractureEnergy; m.particleBudget=d.particleBudget;
    m.guided=guided;
    _projectiles.push_back(m);
    return m.id;
}

void MissileSystem::EmitDetonation(const MissileProjectile& missile)
{
    MissileDetonation d;
    d.missileId=missile.id; d.payload=missile.payload; d.position=missile.position;
    d.radius=missile.detonationRadius; d.impulse=missile.impulse; d.damage=missile.damage;
    d.fractureEnergy=missile.fractureEnergy; d.particleBudget=missile.particleBudget;
    d.lifetime = missile.payload==MissilePayloadType::MiningFracture ? 1.9f : 1.25f;
    _detonations.push_back(d);
}

bool MissileSystem::Detonate(std::uint64_t missileId)
{
    for (auto& missile : _projectiles) {
        if (missile.id != missileId || !missile.alive) continue;
        missile.alive=false;
        EmitDetonation(missile);
        return true;
    }
    return false;
}

void MissileSystem::Update(float deltaTime)
{
    if (deltaTime < 0.0f) deltaTime = 0.0f;
    for (auto& m : _projectiles) {
        if (!m.alive) continue;
        m.previousPosition=m.position;
        m.age += deltaTime;

        if (m.guided) {
            const Vector3 desired = PlanarNormalized(m.targetPosition-m.position,{0,1,0});
            const Vector3 velocityDir = PlanarNormalized(m.velocity,desired);
            // Guidance changes velocity through bounded lateral acceleration;
            // the projectile never teleports or rotates its velocity instantly.
            Vector3 steer = desired-velocityDir;
            if (steer.length()>1.0f) steer=steer.normalized();
            m.velocity = m.velocity + steer*(m.guidanceStrength*m.acceleration*deltaTime);
        }

        const Vector3 accelDir = PlanarNormalized(m.velocity,{0,1,0});
        m.velocity = m.velocity + accelDir*(m.acceleration*deltaTime);
        const float speed=std::sqrt(m.velocity.x*m.velocity.x+m.velocity.y*m.velocity.y);
        if (speed>m.maxSpeed && speed>0.001f) {
            const float scale=m.maxSpeed/speed;
            m.velocity.x*=scale; m.velocity.y*=scale;
        }
        m.velocity.z=0.0f;
        m.position = m.position + m.velocity*deltaTime;
        m.position.z=0.0f;

        if ((m.guided && Distance2D(m.position,m.targetPosition)<=m.proximityFuse) || m.age>=m.lifetime) {
            m.alive=false;
            EmitDetonation(m);
        }
    }

    _projectiles.erase(std::remove_if(_projectiles.begin(),_projectiles.end(),
        [](const MissileProjectile& m){return !m.alive;}),_projectiles.end());

    for (auto& d : _detonations) d.age += deltaTime;
    _detonations.erase(std::remove_if(_detonations.begin(),_detonations.end(),
        [](const MissileDetonation& d){return d.age>=d.lifetime;}),_detonations.end());
}

void MissileSystem::Clear()
{
    _projectiles.clear();
    _detonations.clear();
    _nextId=1;
}

} // namespace subspace
