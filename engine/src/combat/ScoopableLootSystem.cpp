#include "combat/ScoopableLootSystem.h"
#include "ship_editor/KitbashShipBuilderSystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
float Next01(std::uint32_t& s){s=s*1664525u+1013904223u;return static_cast<float>((s>>8)&0x00FFFFFFu)/16777215.0f;}
Vector3 RandomImpulse(std::uint32_t& s){return {(Next01(s)-.5f)*1.3f,(Next01(s)-.5f)*1.3f,(Next01(s)-.5f)*.25f};}
float Length(const Vector3& v){return std::sqrt(v.x*v.x+v.y*v.y+v.z*v.z);}
}

ScoopableItemDrop ScoopableLootSystem::CreateDrop(const GeneratedItem& item,const Vector3& p,const Vector3& impulse,std::uint64_t id){
    ScoopableItemDrop d;d.dropId=id;d.item=item;d.position=p;d.velocity=impulse;
    // All module drops are compact cargo-scale tokens; the source module is
    // shown as a miniature/holographic identity, not full construction scale.
    d.compactScale=(item.kind==ItemKind::Blueprint||item.kind==ItemKind::BlueprintFragment) ? .14f : .18f;
    d.pickupRadius=.55f;d.attractionRange=4.5f;return d;
}

std::vector<ScoopableItemDrop> ScoopableLootSystem::BuildDestroyedShipDrops(
    const ProceduralShipVisualRecipe& ship,const std::vector<ShipyardModuleRecord>& catalog,
    const Vector3& position,std::uint32_t seed,float moduleChance,float blueprintChance){
    std::vector<ScoopableItemDrop> out;std::uint64_t id=1;
    for(const auto& placement:ship.modules){
        if(Next01(seed)>std::clamp(moduleChance,0.0f,1.0f))continue;
        const auto* record=KitbashShipBuilderSystem::FindRecord(catalog,placement.moduleId);if(!record)continue;
        auto def=ItemizationSystem::BuildShipyardDefinition(*record);
        const float condition=.28f+Next01(seed)*.60f;
        const auto rarity=ItemizationSystem::RollRarity(seed^0x91BADC0u,.02f);
        ItemProvenance provenance;provenance.source="DESTROYED_SHIP";provenance.originEntity=ship.recipeId;provenance.manufacturer=ship.manufacturerFamily;provenance.seed=seed;
        auto item=ItemizationSystem::Generate(def,seed,rarity,.92f+Next01(seed)*.12f,condition,provenance);
        out.push_back(CreateDrop(item,position,RandomImpulse(seed),id++));
    }
    if(Next01(seed)<blueprintChance){
        ItemDefinition def;def.definitionId="blueprint_fragment:"+ship.recipeId;def.displayName=(ship.recipeId.empty()?"Recovered Ship":ship.recipeId)+" Blueprint Fragment";
        def.kind=ItemKind::BlueprintFragment;def.category="BLUEPRINT";def.baseMass=.05f;def.baseValue=450.0f;
        if(!ship.modules.empty())def.sourceModuleId=ship.modules.front().moduleId;
        ItemProvenance provenance;provenance.source="DESTROYED_SHIP";provenance.originEntity=ship.recipeId;provenance.manufacturer=ship.manufacturerFamily;provenance.seed=seed;
        auto item=ItemizationSystem::Generate(def,seed,ItemRarity::Rare,1.0f,1.0f,provenance);
        item.blueprintId=ship.recipeId;item.blueprintFragmentsRequired=5;item.blueprintFragmentIndex=1+static_cast<int>(seed%5u);item.iconKey=ItemizationSystem::BuildIconKey(item);
        out.push_back(CreateDrop(item,position,RandomImpulse(seed),id++));
    }
    // Complete blueprints are much rarer than fragments.
    if(Next01(seed)<.035f){
        ItemDefinition def;def.definitionId="blueprint:"+ship.recipeId;def.displayName=(ship.recipeId.empty()?"Recovered Ship":ship.recipeId)+" Complete Blueprint";
        def.kind=ItemKind::Blueprint;def.category="BLUEPRINT";def.baseMass=.05f;def.baseValue=2200.0f;
        if(!ship.modules.empty())def.sourceModuleId=ship.modules.front().moduleId;
        ItemProvenance provenance;provenance.source="DESTROYED_SHIP";provenance.originEntity=ship.recipeId;provenance.manufacturer=ship.manufacturerFamily;provenance.seed=seed;
        auto item=ItemizationSystem::Generate(def,seed,ItemRarity::Epic,1.0f,1.0f,provenance);
        item.blueprintId=ship.recipeId;item.iconKey=ItemizationSystem::BuildIconKey(item);
        out.push_back(CreateDrop(item,position,RandomImpulse(seed),id++));
    }
    return out;
}

void ScoopableLootSystem::Update(std::vector<ScoopableItemDrop>& drops,const Vector3& collector,float collectorRange,float dt,std::vector<GeneratedItem>* collected){
    for(auto& d:drops){if(d.collected)continue;d.spinRadians+=dt*.9f;Vector3 to{collector.x-d.position.x,collector.y-d.position.y,collector.z-d.position.z};const float dist=Length(to);
        const float range=std::max(collectorRange,d.attractionRange);if(dist<range&&dist>.001f){const float accel=4.5f*(1.0f-dist/range)+1.0f;d.velocity.x+=to.x/dist*accel*dt;d.velocity.y+=to.y/dist*accel*dt;d.velocity.z+=to.z/dist*accel*dt;}
        d.position.x+=d.velocity.x*dt;d.position.y+=d.velocity.y*dt;d.position.z+=d.velocity.z*dt;d.velocity.x*=std::exp(-1.6f*dt);d.velocity.y*=std::exp(-1.6f*dt);d.velocity.z*=std::exp(-1.6f*dt);
        if(dist<=d.pickupRadius){d.collected=true;if(collected)collected->push_back(d.item);}
    }
    drops.erase(std::remove_if(drops.begin(),drops.end(),[](const auto&d){return d.collected;}),drops.end());
}

} // namespace subspace
