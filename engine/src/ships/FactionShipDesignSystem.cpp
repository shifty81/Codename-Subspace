#include "ships/FactionShipDesignSystem.h"

#include <algorithm>

namespace subspace {

FactionDesignDna FactionShipDesignSystem::DefaultFactionDna(const std::string& faction){
    FactionDesignDna d;d.factionId=faction;
    const std::uint32_t seed=ProceduralVisualVariantSystem::StableSeed(faction);
    auto unit=[&](int shift){return static_cast<float>((seed>>shift)&0xFFu)/255.0f;};
    d.angularity=.35f+unit(0)*.55f;d.longitudinalBias=.35f+unit(4)*.55f;d.exposedStructure=.12f+unit(8)*.45f;d.armorDensity=.30f+unit(12)*.55f;d.symmetryPreference=.65f+unit(16)*.32f;d.enginePairPreference=.50f+unit(20)*.45f;d.sensorExposure=.20f+unit(24)*.55f;
    d.preferredStyleTags={d.angularity>.62f?"ANGULAR":"SOFT_PANEL",d.longitudinalBias>.62f?"LONGITUDINAL":"WIDE_HULL",d.exposedStructure>.35f?"EXPOSED_STRUCTURE":"SKINNED_STRUCTURE"};return d;
}

std::vector<FactionHullFamilyDefinition> FactionShipDesignSystem::BuildClassFamilies(const std::string& faction,ShipClass c,const FactionDesignDna& dna){
    auto f=ShipClassRoleSystem::BuildDefaultHullFamilies(faction,c);
    for(auto& family:f){
        if(family.designIndex==0){family.speedBias*=.90f+.20f*dna.longitudinalBias;family.armorBias*=.88f+.18f*dna.armorDensity;}
        if(family.designIndex==1){family.utilityBias*=.92f+.18f*dna.sensorExposure;}
        if(family.designIndex==2){family.armorBias*=.95f+.30f*dna.armorDensity;family.speedBias*=1.05f-.18f*dna.armorDensity;}
        if(family.designIndex==3){family.utilityBias*=1.0f+.32f*dna.exposedStructure;family.internalVolumeBias*=.95f+.22f*(1.0f-dna.longitudinalBias);}
        family.chassisStyle += dna.angularity>.62f?"_ANGULAR":"_COHERENT";
    }
    return f;
}

HullFamilyExemplarRecord FactionShipDesignSystem::CaptureExemplar(const std::string&id,const FactionHullFamilyDefinition&family,ShipRole role,const std::vector<ShipyardModuleRecord>&catalog,const ProceduralShipVisualRecipe&recipe){HullFamilyExemplarRecord r;r.exemplarId=id;r.familyId=family.familyId;r.factionId=family.factionId;r.shipClass=family.shipClass;r.role=role;r.exemplar=ShipyardDesignDnaSystem::BuildExemplar(id,ShipClassRoleSystem::RoleName(role),catalog,recipe);r.approved=true;return r;}

HullFamilyCompiledGrammar FactionShipDesignSystem::Compile(const FactionHullFamilyDefinition&family,const FactionDesignDna&dna,const std::vector<HullFamilyExemplarRecord>&exemplars){HullFamilyCompiledGrammar out;out.runtime=ShipPcgRuntimeClosureSystem::BuildHullFamilyProfile(family);out.factionDna=dna;std::vector<ShipyardDesignExemplar> valid;for(const auto&e:exemplars)if(e.approved&&e.familyId==family.familyId){valid.push_back(e.exemplar);out.exemplarIds.push_back(e.exemplarId);}out.grammar=ShipyardDesignDnaSystem::BuildGrammar(family.familyId+"_GRAMMAR",valid);if(valid.empty()){out.grammar.id=family.familyId+"_GRAMMAR";out.grammar.role="MULTIROLE";out.grammar.symmetryWeight=dna.symmetryPreference;out.grammar.propulsionAftWeight=dna.enginePairPreference;out.grammar.commandForwardWeight=.85f;}return out;}

bool FactionShipDesignSystem::VariantPreservesLineage(const ProceduralShipVisualRecipe&recipe,const HullFamilyCompiledGrammar&family,ShipRole role){if(recipe.factionId!=family.runtime.factionId||recipe.hullFamilyId!=family.runtime.familyId)return false;if(recipe.shipClassId!=ShipClassRoleSystem::ClassName(family.runtime.shipClass))return false;if(recipe.roleVariantId!=ShipClassRoleSystem::RoleName(role))return false;if(!family.runtime.allowedRoles.empty()&&std::find(family.runtime.allowedRoles.begin(),family.runtime.allowedRoles.end(),role)==family.runtime.allowedRoles.end())return false;return recipe.lineageAuthority=="FACTION_CLASS_HULL_ROLE_V1";}

} // namespace subspace
