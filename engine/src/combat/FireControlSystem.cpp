#include "combat/FireControlSystem.h"

#include <algorithm>

namespace subspace {

WeaponFirePolicy FireControlSystem::PolicyFor(WeaponType type){
    WeaponFirePolicy p;
    switch(type){
        case WeaponType::SpinalRailgun:p.requiresTargetLock=true;p.allowsManualAim=true;p.allowsFreeFire=true;p.convergenceRange=260.0f;break;
        case WeaponType::BroadsideCannon:p.requiresTargetLock=true;p.allowsManualAim=true;p.allowsFreeFire=true;p.convergenceRange=160.0f;break;
        case WeaponType::InwardFlak:p.requiresTargetLock=false;p.allowsManualAim=true;p.allowsFreeFire=true;p.convergenceRange=75.0f;break;
        case WeaponType::BurstLancer:p.requiresTargetLock=true;p.allowsManualAim=true;p.allowsFreeFire=false;p.convergenceRange=220.0f;break;
        case WeaponType::BeamArray:p.requiresTargetLock=true;p.allowsManualAim=true;p.allowsFreeFire=false;p.convergenceRange=180.0f;break;
    }
    return p;
}

FireControlSolution FireControlSystem::Solve(WeaponType type,const FireControlRequest& r){
    FireControlSolution s;const auto policy=PolicyFor(type);s.lockRequired=policy.requiresTargetLock;
    if(policy.requiresTargetLock&&!r.targetLocked&&r.mode!=FireControlMode::FreeFire){s.status="TARGET LOCK REQUIRED";return s;}
    if(r.mode==FireControlMode::FreeFire&&!policy.allowsFreeFire){s.status="FREE FIRE NOT SUPPORTED";return s;}
    Vector3 aim=r.lockedTargetWorld;
    if(r.mode==FireControlMode::LockedManualAim&&policy.allowsManualAim){
        // Manual pointer aim is a precision bias around the locked target rather
        // than an arcade bypass of the target-lock/fitting model.
        const Vector3 bias=r.pointerWorld-r.lockedTargetWorld;
        const float maxBias=std::max(2.0f,policy.convergenceRange*.10f);
        const float len=bias.length();aim=r.lockedTargetWorld+(len>maxBias?bias.normalized()*maxBias:bias);
    } else if(r.mode==FireControlMode::FreeFire){aim=r.pointerWorld;}
    if(r.projectileSpeed>1.0f&&r.mode!=FireControlMode::FreeFire){
        const float distance=(aim-r.muzzleWorld).length();s.leadSeconds=std::clamp(distance/r.projectileSpeed,0.0f,3.5f);aim=aim+r.targetVelocity*s.leadSeconds;
    }
    s.aimPoint=aim;Vector3 d=aim-r.muzzleWorld;s.aimDirection=d.length()>.0001f?d.normalized():Vector3{0,1,0};s.valid=true;s.mayFire=r.triggerHeld;s.status=r.mode==FireControlMode::LockedManualAim?"MANUAL PRECISION":"FIRE CONTROL READY";return s;
}

} // namespace subspace
