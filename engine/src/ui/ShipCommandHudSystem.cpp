#include "ui/ShipCommandHudSystem.h"
#include "combat/CombatSystem.h"
#include <algorithm>

namespace subspace {
namespace {
void PopulateModules(ShipCommandHudModel& m,const std::vector<std::string>& labels){
    int i=0;
    for(const auto& label:labels){
        ShipCommandModuleState s;
        s.label=label;
        s.shortcut="F"+std::to_string(++i);
        s.requiresTarget=label!="SCAN"&&label!="BOOST"&&label!="REPAIR";
        m.modules.push_back(s);
    }
}
}

ShipCommandHudModel ShipCommandHudSystem::Build(float speed,bool dampeners,bool boost,const std::vector<std::string>& labels){
    return Build(speed,dampeners,boost,labels,nullptr);
}

ShipCommandHudModel ShipCommandHudSystem::Build(float speed,bool dampeners,bool boost,
                                                 const std::vector<std::string>& labels,
                                                 const CombatComponent* combat){
    ShipCommandHudModel m;
    m.speed=std::max(0.0f,speed);
    m.dampeners=dampeners;
    m.boost=boost;
    if(combat){
        m.shield=combat->ShieldFraction();
        m.armor=combat->ArmorFraction();
        m.hull=combat->HullFraction();
        m.power=combat->PowerFraction();
        m.shieldOnline=combat->shields.isShieldActive && m.shield>0.001f;
        m.critical=m.hull<0.25f;
        if(m.hull<=0.001f)m.integrityStatus="DESTROYED";
        else if(m.critical)m.integrityStatus="HULL CRITICAL";
        else if(m.armor<0.25f)m.integrityStatus="ARMOR CRITICAL";
        else if(m.shield<0.10f)m.integrityStatus="SHIELDS DOWN";
        else if(m.power<0.15f)m.integrityStatus="LOW POWER";
        else m.integrityStatus="NOMINAL";
    }
    PopulateModules(m,labels);
    return m;
}
} // namespace subspace
