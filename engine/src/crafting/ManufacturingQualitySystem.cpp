#include "crafting/ManufacturingQualitySystem.h"

#include <algorithm>
#include <cmath>

namespace subspace {
namespace {
float NextSigned(std::uint32_t& seed) {
    seed = seed * 1664525u + 1013904223u;
    const float v = static_cast<float>((seed >> 8) & 0x00FFFFFFu) / 16777215.0f;
    return v * 2.0f - 1.0f;
}
}

int ManufacturingSkillProfile::LevelFor(ManufacturingDiscipline d) const {
    switch (d) {
        case ManufacturingDiscipline::Structural:return structural;
        case ManufacturingDiscipline::Propulsion:return propulsion;
        case ManufacturingDiscipline::Weapons:return weapons;
        case ManufacturingDiscipline::Electronics:return electronics;
        case ManufacturingDiscipline::Utility:return utility;
        case ManufacturingDiscipline::Shipbuilding:return shipbuilding;
    }
    return 0;
}

ManufacturingDiscipline ManufacturingQualitySystem::DisciplineFor(const ItemDefinition& d) {
    switch (d.shipyardRole) {
        case ShipyardPartRole::MainEngine:
        case ShipyardPartRole::RcsThruster:
        case ShipyardPartRole::EngineHousing:
        case ShipyardPartRole::EngineMount: return ManufacturingDiscipline::Propulsion;
        case ShipyardPartRole::WeaponTurret:
        case ShipyardPartRole::MissileMount:
        case ShipyardPartRole::HardpointBase: return ManufacturingDiscipline::Weapons;
        case ShipyardPartRole::SensorDish:
        case ShipyardPartRole::SensorMast:
        case ShipyardPartRole::Cockpit:
        case ShipyardPartRole::Bridge: return ManufacturingDiscipline::Electronics;
        case ShipyardPartRole::Cargo:
        case ShipyardPartRole::Tank: return ManufacturingDiscipline::Utility;
        case ShipyardPartRole::PrimaryHull:
        case ShipyardPartRole::HullAdapter:
        case ShipyardPartRole::StructuralBrace:
        case ShipyardPartRole::StructuralBlock:
        case ShipyardPartRole::Wing:
        case ShipyardPartRole::Fin: return ManufacturingDiscipline::Structural;
        default: return ManufacturingDiscipline::Shipbuilding;
    }
}

float ManufacturingQualitySystem::Craftsmanship(const ManufacturingContext& c,
                                                  float* skillContribution,
                                                  float* materialContribution,
                                                  float* stationContribution) {
    const float skill = std::clamp(c.skills.LevelFor(c.discipline),0,100) / 100.0f;
    const float skillPart = skill * 0.16f;
    const float materialPart = std::clamp(c.materialPurity - 1.0f, -0.25f, 0.15f) * 0.22f;
    const float stationPart = static_cast<float>(std::clamp(c.stationTier,1,5)-1) * 0.018f;
    const float masteryPart = std::clamp(c.blueprintMastery,0.0f,1.0f) * 0.07f;
    std::uint32_t seed=c.seed;
    const float workmanshipVariance = NextSigned(seed) * 0.025f;
    if(skillContribution)*skillContribution=skillPart;
    if(materialContribution)*materialContribution=materialPart;
    if(stationContribution)*stationContribution=stationPart;
    return std::clamp(0.91f + skillPart + materialPart + stationPart + masteryPart + workmanshipVariance,0.78f,1.28f);
}

ManufacturingResult ManufacturingQualitySystem::Manufacture(const ItemDefinition& definition,
                                                              const ManufacturingContext& input) {
    ManufacturingContext c=input;
    c.discipline=DisciplineFor(definition);
    ManufacturingResult result;
    result.craftsmanship=Craftsmanship(c,&result.skillContribution,&result.materialContribution,&result.stationContribution);
    // Skill shifts the rarity roll but cannot exceed the blueprint/material cap.
    const float qualityBias=std::clamp((result.craftsmanship-0.94f)*0.55f,0.0f,0.18f);
    const ItemRarity rarity=ItemizationSystem::RollRarity(c.seed^0xA5F00D1u,qualityBias,c.rarityCap);
    ItemProvenance provenance;
    provenance.source="MANUFACTURED";
    provenance.manufacturer=c.manufacturer;
    provenance.crafter=c.crafter;
    provenance.manufacturingSkill=c.skills.LevelFor(c.discipline);
    provenance.stationTier=c.stationTier;
    provenance.seed=c.seed;
    result.item=ItemizationSystem::Generate(definition,c.seed,rarity,result.craftsmanship,1.0f,provenance);
    result.exceptional=static_cast<int>(rarity)>=static_cast<int>(ItemRarity::Epic) || result.craftsmanship>=1.17f;
    return result;
}

} // namespace subspace
