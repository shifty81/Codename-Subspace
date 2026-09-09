#include "ship_editor/ShipyardDefinitionOverrideSystem.h"

#include <cmath>
#include <fstream>
#include <iomanip>
#include <unordered_map>

namespace subspace {
namespace {
constexpr const char* kHeader="SUBSPACE_SHIPYARD_DEFINITION_OVERRIDES_V1";
bool Near(float a,float b,float e){return std::fabs(a-b)<=e;}
}

bool ShipyardDefinitionOverrideSystem::DefinitionEqual(const ShipyardModuleRecord& a,
                                                        const ShipyardModuleRecord& b,
                                                        float epsilon){
    return a.moduleClass==b.moduleClass && a.semantic==b.semantic && a.size==b.size &&
           a.preferredRoles==b.preferredRoles && a.preserveAspectRatio==b.preserveAspectRatio &&
           a.builderCategory==b.builderCategory && a.partRole==b.partRole && a.primaryHull==b.primaryHull &&
           a.generatorEligible==b.generatorEligible && a.surfaceOnly==b.surfaceOnly && a.functional==b.functional &&
           a.mirrorPreferred==b.mirrorPreferred && a.placementRole==b.placementRole &&
           a.pairedPlacement==b.pairedPlacement && a.preferredMountFace==b.preferredMountFace &&
           Near(a.mountFaceConfidence,b.mountFaceConfidence,epsilon);
}

bool ShipyardDefinitionOverrideSystem::Save(const std::vector<ShipyardModuleRecord>& baseline,
                                             const std::vector<ShipyardModuleRecord>& edited,
                                             const std::string& path,
                                             std::string* error,
                                             std::size_t* changedModules){
    std::unordered_map<std::string,const ShipyardModuleRecord*> base;
    for(const auto& record:baseline)base[record.source.moduleId]=&record;
    std::ofstream out(path,std::ios::trunc);
    if(!out){if(error)*error="could not open definition override path for writing";return false;}
    out<<kHeader<<"\n"<<std::setprecision(9);
    std::size_t changed=0;
    for(const auto& record:edited){
        const auto it=base.find(record.source.moduleId);
        if(it!=base.end()&&DefinitionEqual(*it->second,record))continue;
        ++changed;
        out<<"MODULE "<<std::quoted(record.source.moduleId)<<" "
           <<static_cast<int>(record.moduleClass)<<" "<<static_cast<int>(record.semantic)<<" "<<static_cast<int>(record.size)<<" "
           <<record.generatorEligible<<" "<<record.pairedPlacement<<" "<<record.functional<<" "<<record.surfaceOnly<<" "<<record.mirrorPreferred<<" "
           <<std::quoted(record.placementRole)<<" "<<std::quoted(record.preferredMountFace)<<" "<<record.mountFaceConfidence<<"\n";
    }
    if(!out.good()){if(error)*error="failed while writing definition overrides";return false;}
    if(changedModules)*changedModules=changed;return true;
}

bool ShipyardDefinitionOverrideSystem::LoadAndApply(std::vector<ShipyardModuleRecord>& catalog,
                                                     const std::string& path,
                                                     std::string* error,
                                                     std::size_t* appliedModules){
    std::ifstream in(path);if(!in){if(error)*error="definition override file not found";return false;}
    std::string header;std::getline(in,header);if(header!=kHeader){if(error)*error="unsupported definition override document";return false;}
    std::unordered_map<std::string,ShipyardModuleRecord*> byId;for(auto& r:catalog)byId[r.source.moduleId]=&r;
    std::size_t applied=0;std::string token;
    while(in>>token){
        if(token!="MODULE"){if(error)*error="malformed definition override document";return false;}
        std::string id,placementRole,mountFace;int moduleClass=0,semantic=0,size=0;bool generator=false,paired=false,functional=false,surface=false,mirror=false;float confidence=0.0f;
        if(!(in>>std::quoted(id)>>moduleClass>>semantic>>size>>generator>>paired>>functional>>surface>>mirror>>std::quoted(placementRole)>>std::quoted(mountFace)>>confidence)){
            if(error)*error="malformed MODULE definition override";return false;
        }
        const auto it=byId.find(id);if(it==byId.end())continue;
        auto& r=*it->second;
        r.moduleClass=static_cast<ShipyardModuleClass>(moduleClass);
        r.semantic=static_cast<ShipyardModuleSemantic>(semantic);
        r.size=static_cast<ShipyardModuleSize>(size);
        r.builderCategory=ShipyardPartTaxonomySystem::CategoryFor(r.moduleClass);
        r.partRole=ShipyardPartTaxonomySystem::RoleFor(r.semantic,r.source.moduleId);
        r.primaryHull=r.partRole==ShipyardPartRole::PrimaryHull;
        r.generatorEligible=generator;r.pairedPlacement=paired;r.functional=functional;r.surfaceOnly=surface;r.mirrorPreferred=mirror;
        r.placementRole=std::move(placementRole);r.preferredMountFace=std::move(mountFace);r.mountFaceConfidence=confidence;
        ++applied;
    }
    if(appliedModules)*appliedModules=applied;return true;
}

} // namespace subspace
