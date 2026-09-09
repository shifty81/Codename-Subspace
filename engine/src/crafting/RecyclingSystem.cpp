#include "crafting/RecyclingSystem.h"

#include <algorithm>

namespace subspace {

RecyclingResult RecyclingSystem::Recycle(const GeneratedItem& item,const RecyclingContext& c){
    RecyclingResult r;
    const float skill=std::clamp(c.recyclingSkill,0,100)/100.0f;
    const float facility=.48f+static_cast<float>(std::clamp(c.facilityTier,1,5)-1)*.075f;
    r.efficiency=std::clamp(facility+skill*.20f,0.45f,0.92f);
    if(c.reverseEngineer){r.blueprintResearch=(12.0f+28.0f*skill)*(0.55f+0.45f*item.condition);r.efficiency*=.55f;}
    const float mass=std::max(.1f,item.mass)*r.efficiency;
    r.materials.push_back({"structural_alloy",mass*.62f});
    if(item.category=="PROPULSION"||item.category=="WEAPONS"||item.category=="SENSORS"||item.category=="COMMAND")r.materials.push_back({"electronics",mass*.14f});
    if(item.category=="PROPULSION"||item.category=="WEAPONS")r.materials.push_back({"conductive_metal",mass*.12f});
    if(static_cast<int>(item.rarity)>=static_cast<int>(ItemRarity::Rare))r.materials.push_back({"rare_alloy",mass*(.025f+.01f*static_cast<int>(item.rarity))});
    return r;
}

} // namespace subspace
