#include "ships/ShipFunctionalCoreSystem.h"

#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace subspace {
namespace {std::string Lower(std::string s){for(char&c:s)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));return s;}bool Has(const std::string&s,const char*t){return s.find(t)!=std::string::npos;}}
const char* ShipFunctionalCoreSystem::Name(ShipFunctionalCapability c){switch(c){case ShipFunctionalCapability::Command:return "COMMAND";case ShipFunctionalCapability::Power:return "POWER";case ShipFunctionalCapability::MainPropulsion:return "MAIN PROPULSION";case ShipFunctionalCapability::Maneuvering:return "MANEUVERING";case ShipFunctionalCapability::Navigation:return "NAVIGATION";case ShipFunctionalCapability::Sensors:return "SENSORS";case ShipFunctionalCapability::Communications:return "COMMUNICATIONS";case ShipFunctionalCapability::Thermal:return "THERMAL";case ShipFunctionalCapability::Structure:return "STRUCTURE";case ShipFunctionalCapability::FuelEnergyStorage:return "FUEL / ENERGY STORAGE";case ShipFunctionalCapability::CrewControl:return "CREW / CONTROL";case ShipFunctionalCapability::LifeSupport:return "LIFE SUPPORT";case ShipFunctionalCapability::UtilityAccess:return "UTILITY ACCESS";}return "UNKNOWN";}
std::vector<ShipFunctionalCapability> ShipFunctionalCoreSystem::Required(bool biologicalCrew){std::vector<ShipFunctionalCapability> r={ShipFunctionalCapability::Command,ShipFunctionalCapability::Power,ShipFunctionalCapability::MainPropulsion,ShipFunctionalCapability::Maneuvering,ShipFunctionalCapability::Navigation,ShipFunctionalCapability::Sensors,ShipFunctionalCapability::Communications,ShipFunctionalCapability::Thermal,ShipFunctionalCapability::Structure,ShipFunctionalCapability::FuelEnergyStorage,ShipFunctionalCapability::CrewControl,ShipFunctionalCapability::UtilityAccess};if(biologicalCrew)r.push_back(ShipFunctionalCapability::LifeSupport);return r;}
std::vector<ShipFunctionalCapability> ShipFunctionalCoreSystem::CapabilitiesFor(const ShipyardModuleRecord&r){
    std::vector<ShipFunctionalCapability> o;auto add=[&](ShipFunctionalCapability c){if(std::find(o.begin(),o.end(),c)==o.end())o.push_back(c);};
    const std::string n=Lower(r.source.moduleId+" "+r.placementRole);
    switch(r.semantic){
    case ShipyardModuleSemantic::CommandCockpit:case ShipyardModuleSemantic::CommandBridge:add(ShipFunctionalCapability::Command);add(ShipFunctionalCapability::Navigation);add(ShipFunctionalCapability::CrewControl);add(ShipFunctionalCapability::Communications);break;
    case ShipyardModuleSemantic::MainEngine:case ShipyardModuleSemantic::EngineNozzle:add(ShipFunctionalCapability::MainPropulsion);break;
    case ShipyardModuleSemantic::RcsThruster:add(ShipFunctionalCapability::Maneuvering);break;
    case ShipyardModuleSemantic::Sensor:add(ShipFunctionalCapability::Sensors);add(ShipFunctionalCapability::Communications);break;
    case ShipyardModuleSemantic::HullBow:case ShipyardModuleSemantic::HullMid:case ShipyardModuleSemantic::HullAft:case ShipyardModuleSemantic::StructuralFrame:case ShipyardModuleSemantic::Adapter:add(ShipFunctionalCapability::Structure);break;
    default:break;
    }
    if(Has(n,"reactor")||Has(n,"power")||Has(n,"generator"))add(ShipFunctionalCapability::Power);
    if(Has(n,"radiator")||Has(n,"thermal")||Has(n,"cool"))add(ShipFunctionalCapability::Thermal);
    if(Has(n,"fuel")||Has(n,"battery")||Has(n,"tank")||Has(n,"energy_storage"))add(ShipFunctionalCapability::FuelEnergyStorage);
    if(Has(n,"life")||Has(n,"hab")||Has(n,"crew")||Has(n,"bridge")||Has(n,"cockpit"))add(ShipFunctionalCapability::LifeSupport);
    if(Has(n,"utility")||Has(n,"service")||Has(n,"maintenance")||Has(n,"access"))add(ShipFunctionalCapability::UtilityAccess);
    if(Has(n,"nav"))add(ShipFunctionalCapability::Navigation);
    if(Has(n,"comm")||Has(n,"antenna")||Has(n,"radio"))add(ShipFunctionalCapability::Communications);
    return o;
}
ShipFunctionalCoreReport ShipFunctionalCoreSystem::Validate(const std::vector<ShipyardModuleRecord>&catalog,const ProceduralShipVisualRecipe&recipe,bool biologicalCrew){
    ShipFunctionalCoreReport report;std::unordered_set<int> sat;
    auto find=[&](const std::string&id)->const ShipyardModuleRecord*{for(const auto&r:catalog)if(r.source.moduleId==id)return &r;return nullptr;};
    for(const auto&p:recipe.modules)if(const auto*r=find(p.moduleId))for(const auto c:CapabilitiesFor(*r))sat.insert(static_cast<int>(c));
    for(const auto c:Required(biologicalCrew)){if(sat.count(static_cast<int>(c))){report.satisfied.push_back(c);}else{report.missing.push_back(c);report.messages.push_back(std::string("Missing required functional capability: ")+Name(c));}}
    report.valid=report.missing.empty();return report;
}
ShipFunctionalAutofitPlan ShipFunctionalCoreSystem::BuildAutofitPlan(const std::vector<ShipyardModuleRecord>&catalog,const ProceduralShipVisualRecipe&recipe,bool biologicalCrew){
    ShipFunctionalAutofitPlan plan;const auto report=Validate(catalog,recipe,biologicalCrew);
    for(const auto missing:report.missing){
        const ShipyardModuleRecord* best=nullptr;
        for(const auto& rec:catalog){
            const auto caps=CapabilitiesFor(rec);
            if(std::find(caps.begin(),caps.end(),missing)==caps.end())continue;
            if(!rec.generatorEligible)continue;
            if(!best||static_cast<int>(rec.size)<static_cast<int>(best->size))best=&rec;
        }
        if(best)plan.additions.push_back({missing,best->source.moduleId});else plan.unresolved.push_back(missing);
    }
    plan.complete=plan.unresolved.empty();return plan;
}
} // namespace subspace
