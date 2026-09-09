#include "content/ShipyardPartTaxonomySystem.h"
#include "content/ShipyardNameClassification.h"

namespace subspace {
ShipyardPartCategory ShipyardPartTaxonomySystem::CategoryFor(ShipyardModuleClass c){
    return static_cast<ShipyardPartCategory>(static_cast<int>(c));
}
ShipyardPartRole ShipyardPartTaxonomySystem::RoleFor(ShipyardModuleSemantic s,const std::string& name){
    switch(s){
        case ShipyardModuleSemantic::HullBow: case ShipyardModuleSemantic::HullMid: case ShipyardModuleSemantic::HullAft: return ShipyardPartRole::PrimaryHull;
        case ShipyardModuleSemantic::StructuralFrame:{
            const auto v=ShipyardNameClassifier::CanonicalLeafName(name);
            if(v.find("strut")!=std::string::npos)return ShipyardPartRole::EngineStrut;
            if(v.find("brace")!=std::string::npos||v.find("truss")!=std::string::npos)return ShipyardPartRole::StructuralBrace;
            if(v.find("block")!=std::string::npos)return ShipyardPartRole::StructuralBlock;
            if(v.find("outrigger")!=std::string::npos)return ShipyardPartRole::Outrigger;
            return ShipyardPartRole::StructuralFrame;
        }
        case ShipyardModuleSemantic::CommandCockpit:return ShipyardPartRole::Cockpit;
        case ShipyardModuleSemantic::CommandBridge:return ShipyardPartRole::Bridge;
        case ShipyardModuleSemantic::Adapter:return ShipyardPartRole::HullAdapter;
        case ShipyardModuleSemantic::EngineHousing:return ShipyardPartRole::EngineHousing;
        case ShipyardModuleSemantic::MainEngine:return ShipyardPartRole::MainEngine;
        case ShipyardModuleSemantic::EngineNozzle:return ShipyardPartRole::EngineNozzle;
        case ShipyardModuleSemantic::RcsThruster:return ShipyardPartRole::RcsThruster;
        case ShipyardModuleSemantic::TurretHardpoint:return ShipyardPartRole::HardpointBase;
        case ShipyardModuleSemantic::WeaponMount:{
            const auto v=ShipyardNameClassifier::CanonicalLeafName(name);
            return v.find("miss")!=std::string::npos?ShipyardPartRole::MissileMount:ShipyardPartRole::WeaponTurret;
        }
        case ShipyardModuleSemantic::Wing:{
            const auto v=ShipyardNameClassifier::CanonicalLeafName(name);
            // Greyoxide uses "fin" in several lateral wing source names
            // (for example miscfinhanger/miscfinwing).  Filename text alone
            // is not sufficient authority to permit a vertical stabilizer.
            // Only explicit stabilizer/keel/dorsal/ventral naming becomes a
            // Fin; certified WING semantics remain normal lateral wings.
            const bool explicitVertical =
                v.find("stabilizer")!=std::string::npos ||
                v.find("keel")!=std::string::npos ||
                v.find("dorsalfin")!=std::string::npos || v.find("dorsal_fin")!=std::string::npos ||
                v.find("ventralfin")!=std::string::npos || v.find("ventral_fin")!=std::string::npos ||
                v.find("verticalfin")!=std::string::npos || v.find("vertical_fin")!=std::string::npos;
            return explicitVertical?ShipyardPartRole::Fin:ShipyardPartRole::Wing;
        }
        case ShipyardModuleSemantic::Sensor:{
            const auto v=ShipyardNameClassifier::CanonicalLeafName(name);
            if(v.find("telescope")!=std::string::npos)return ShipyardPartRole::Telescope;
            if(v.find("antenna")!=std::string::npos)return ShipyardPartRole::SensorAntenna;
            return v.find("dish")!=std::string::npos||v.find("radar")!=std::string::npos?ShipyardPartRole::SensorDish:ShipyardPartRole::SensorMast;
        }
        case ShipyardModuleSemantic::SurfaceDetail:return ShipyardPartRole::SurfaceDetail;
        case ShipyardModuleSemantic::Component:{
            const auto v=ShipyardNameClassifier::CanonicalLeafName(name);
            if(v.find("tank")!=std::string::npos)return ShipyardPartRole::Tank;
            if(v.find("cargo")!=std::string::npos||v.find("hold")!=std::string::npos)return ShipyardPartRole::Cargo;
            if(v.find("hangar")!=std::string::npos||v.find("hanger")!=std::string::npos)return ShipyardPartRole::Hangar;
            if(v.find("window")!=std::string::npos||v.find("canopy")!=std::string::npos)return ShipyardPartRole::WindowCanopy;
            if(v.find("ram")!=std::string::npos)return ShipyardPartRole::Ram;
            if(v.find("outrigger")!=std::string::npos)return ShipyardPartRole::Outrigger;
            if(v.find("attachment")!=std::string::npos||v.find("port")!=std::string::npos)return ShipyardPartRole::StructuralAttachment;
            return ShipyardPartRole::ReviewRequired;
        }
    }
    return ShipyardPartRole::Unknown;
}
std::string ShipyardPartTaxonomySystem::DisplayName(ShipyardPartCategory c){
    switch(c){case ShipyardPartCategory::Hull:return "Hull";case ShipyardPartCategory::Command:return "Command";case ShipyardPartCategory::Propulsion:return "Propulsion";case ShipyardPartCategory::Hardpoint:return "Hardpoint";case ShipyardPartCategory::Detail:return "Decoration";case ShipyardPartCategory::Wing:return "Wing";case ShipyardPartCategory::Adapter:return "Adapter";case ShipyardPartCategory::Component:return "Component";}return "Component";
}
std::string ShipyardPartTaxonomySystem::DisplayName(ShipyardPartRole r){
    switch(r){case ShipyardPartRole::PrimaryHull:return "Primary Hull";case ShipyardPartRole::HullAdapter:return "Hull Adapter";case ShipyardPartRole::Cockpit:return "Cockpit";case ShipyardPartRole::Bridge:return "Bridge";case ShipyardPartRole::EngineHousing:return "Engine Housing";case ShipyardPartRole::EngineMount:return "Engine Mount/Nozzle";case ShipyardPartRole::MainEngine:return "Main Engine";case ShipyardPartRole::RcsThruster:return "RCS Thruster";case ShipyardPartRole::HardpointBase:return "Hardpoint Base";case ShipyardPartRole::WeaponTurret:return "Weapon Turret";case ShipyardPartRole::MissileMount:return "Missile Mount";case ShipyardPartRole::SensorDish:return "Sensor Dish";case ShipyardPartRole::SensorMast:return "Sensor Mast";case ShipyardPartRole::Cargo:return "Cargo";case ShipyardPartRole::Tank:return "Tank";case ShipyardPartRole::StructuralFrame:return "Structural Frame";case ShipyardPartRole::StructuralBrace:return "Structural Brace";case ShipyardPartRole::StructuralBlock:return "Structural Block";case ShipyardPartRole::SurfaceDetail:return "Surface Detail";case ShipyardPartRole::Wing:return "Wing";case ShipyardPartRole::Fin:return "Fin";case ShipyardPartRole::EngineStrut:return "Engine Strut";case ShipyardPartRole::EngineNozzle:return "Engine Nozzle";case ShipyardPartRole::SensorAntenna:return "Sensor Antenna";case ShipyardPartRole::Telescope:return "Telescope";case ShipyardPartRole::Hangar:return "Hangar";case ShipyardPartRole::WindowCanopy:return "Window / Canopy";case ShipyardPartRole::StructuralAttachment:return "Structural Attachment";case ShipyardPartRole::Ram:return "Ram";case ShipyardPartRole::Outrigger:return "Outrigger";case ShipyardPartRole::ReviewRequired:return "REVIEW REQUIRED";default:return "Unknown";}
}
std::string ShipyardPartTaxonomySystem::DisplayName(ShipyardModuleClass c){
    return ShipyardModuleSystem::ClassName(c);
}
std::string ShipyardPartTaxonomySystem::DisplayName(ShipyardModuleSemantic s){
    return ShipyardModuleSystem::SemanticName(s);
}
std::string ShipyardPartTaxonomySystem::DisplayName(const ShipyardModuleRecord& record){
    if(record.partRole != ShipyardPartRole::Unknown) return DisplayName(record.partRole);
    return DisplayName(record.semantic);
}
std::string ShipyardPartTaxonomySystem::DisplayName(const VisualModuleSource& source){
    return DisplayName(ShipyardModuleSystem::SemanticClassify(source));
}
std::string ShipyardPartTaxonomySystem::DisplayName(std::string_view moduleNameOrId){
    VisualModuleSource source; source.moduleId = std::string(moduleNameOrId);
    return DisplayName(source);
}
std::string ShipyardPartTaxonomySystem::CategoryName(ShipyardPartCategory c){ return DisplayName(c); }
std::string ShipyardPartTaxonomySystem::CategoryName(ShipyardModuleClass c){ return DisplayName(CategoryFor(c)); }
}
