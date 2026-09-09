#include "ship_editor/ShipyardEquipmentSystem.h"
#include "ship_editor/KitbashShipBuilderSystem.h"

namespace subspace {
namespace {
void Add(std::vector<ShipEquipmentSlot>& out,std::size_t module,ShipEquipmentSlotType type,ShipyardModuleSize size,int count,bool required=false){
    for(int i=0;i<count;++i){ShipEquipmentSlot s;s.moduleIndex=module;s.type=type;s.size=size;s.required=required;s.slotId=std::to_string(module)+":"+ShipyardEquipmentSystem::SlotName(type)+":"+std::to_string(i);out.push_back(s);}
}
}

std::vector<ShipEquipmentSlot> ShipyardEquipmentSystem::BuildSlots(const ProceduralShipVisualRecipe& recipe,const std::vector<ShipyardModuleRecord>& catalog){
    std::vector<ShipEquipmentSlot> out;
    for(std::size_t i=0;i<recipe.modules.size();++i){const auto* r=KitbashShipBuilderSystem::FindRecord(catalog,recipe.modules[i].moduleId);if(!r)continue;
        switch(r->partRole){
            case ShipyardPartRole::PrimaryHull:Add(out,i,ShipEquipmentSlotType::Utility,ShipyardModuleSize::S,2);Add(out,i,ShipEquipmentSlotType::Power,ShipyardModuleSize::M,1);break;
            case ShipyardPartRole::Cockpit:
            case ShipyardPartRole::Bridge:Add(out,i,ShipEquipmentSlotType::Command,r->size,1,true);Add(out,i,ShipEquipmentSlotType::Sensor,ShipyardModuleSize::S,1);Add(out,i,ShipEquipmentSlotType::Utility,ShipyardModuleSize::S,1);break;
            case ShipyardPartRole::EngineHousing:Add(out,i,ShipEquipmentSlotType::MainEngine,r->size,1,true);Add(out,i,ShipEquipmentSlotType::Rcs,ShipyardModuleSize::S,1);break;
            case ShipyardPartRole::EngineMount:Add(out,i,ShipEquipmentSlotType::Rcs,ShipyardModuleSize::S,2);break;
            case ShipyardPartRole::HardpointBase:
            case ShipyardPartRole::WeaponTurret:Add(out,i,ShipEquipmentSlotType::Weapon,r->size,1);break;
            case ShipyardPartRole::MissileMount:Add(out,i,ShipEquipmentSlotType::Missile,r->size,1);break;
            case ShipyardPartRole::SensorDish:
            case ShipyardPartRole::SensorMast:Add(out,i,ShipEquipmentSlotType::Sensor,r->size,1,true);break;
            case ShipyardPartRole::Cargo:Add(out,i,ShipEquipmentSlotType::Cargo,r->size,1,true);break;
            case ShipyardPartRole::Tank:Add(out,i,ShipEquipmentSlotType::Utility,r->size,1);break;
            default:if(r->functional)Add(out,i,ShipEquipmentSlotType::Utility,ShipyardModuleSize::S,1);break;
        }
    }
    return out;
}

bool ShipyardEquipmentSystem::Compatible(const ShipEquipmentSlot& slot,const GeneratedItem& item){
    if(item.kind!=ItemKind::Equipment&&item.kind!=ItemKind::ShipModule)return false;
    const auto has=[&](const char* token){return item.category.find(token)!=std::string::npos||item.definitionId.find(token)!=std::string::npos;};
    switch(slot.type){
        case ShipEquipmentSlotType::MainEngine:return has("PROPULSION")||item.Stat(ItemStatKind::Thrust)>0;
        case ShipEquipmentSlotType::Rcs:return item.Stat(ItemStatKind::Torque)>0;
        case ShipEquipmentSlotType::Weapon:
        case ShipEquipmentSlotType::Missile:return has("WEAPON")||item.Stat(ItemStatKind::WeaponDamage)>0;
        case ShipEquipmentSlotType::Sensor:return has("SENSOR")||item.Stat(ItemStatKind::SensorRange)>0;
        case ShipEquipmentSlotType::Cargo:return has("UTILITY")||item.Stat(ItemStatKind::Cargo)>0;
        default:return true;
    }
}

bool ShipyardEquipmentSystem::Install(ShipEquipmentSlot& slot,const GeneratedItem& item,std::string* error){
    if(!Compatible(slot,item)){if(error)*error="item is incompatible with equipment slot";return false;}
    slot.installedItemInstanceId=item.instanceId;slot.installedDefinitionId=item.definitionId;return true;
}

const char* ShipyardEquipmentSystem::SlotName(ShipEquipmentSlotType t){switch(t){case ShipEquipmentSlotType::Command:return"COMMAND";case ShipEquipmentSlotType::MainEngine:return"MAIN ENGINE";case ShipEquipmentSlotType::Rcs:return"RCS";case ShipEquipmentSlotType::Weapon:return"WEAPON";case ShipEquipmentSlotType::Missile:return"MISSILE";case ShipEquipmentSlotType::Sensor:return"SENSOR";case ShipEquipmentSlotType::Utility:return"UTILITY";case ShipEquipmentSlotType::Cargo:return"CARGO";case ShipEquipmentSlotType::Power:return"POWER";case ShipEquipmentSlotType::Mining:return"MINING";case ShipEquipmentSlotType::Salvage:return"SALVAGE";case ShipEquipmentSlotType::Drone:return"DRONE";}return"UTILITY";}

} // namespace subspace
