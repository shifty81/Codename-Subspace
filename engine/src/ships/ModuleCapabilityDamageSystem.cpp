#include "ships/ModuleCapabilityDamageSystem.h"
#include "ships/Block.h"
#include <algorithm>
namespace subspace {
namespace { struct Acc{float cur=0,max=0;void add(float f,float w){cur+=f*w;max+=w;}float frac()const{return max>0?std::clamp(cur/max,0.0f,1.0f):1.0f;}}; }
ModuleCapabilityFractions ModuleCapabilityDamageSystem::Evaluate(const Ship& ship) const{
    Acc main,rcs,power,shield,sensor,weapon,cargo;
    for(const auto&b:ship.blocks){if(!b)continue;const float hp=b->maxHP>0?std::clamp(b->currentHP/b->maxHP,0.0f,1.0f):0.0f;const float w=std::max(.01f,b->Volume());switch(b->type){
        case BlockType::Engine:main.add(hp,w);break;case BlockType::Thruster:rcs.add(hp,w);break;case BlockType::Generator:case BlockType::Battery:power.add(hp,w);break;case BlockType::ShieldGenerator:shield.add(hp,w);break;case BlockType::Computer:case BlockType::HyperdriveCore:sensor.add(hp,w);break;case BlockType::WeaponMount:weapon.add(hp,w);break;case BlockType::Cargo:cargo.add(hp,w);break;default:break;}}
    return {main.frac(),rcs.frac(),power.frac(),shield.frac(),sensor.frac(),weapon.frac(),cargo.frac()};
}
}
