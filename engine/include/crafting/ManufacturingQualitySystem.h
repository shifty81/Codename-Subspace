#pragma once

#include "inventory/ItemizationSystem.h"

#include <cstdint>
#include <string>

namespace subspace {

enum class ManufacturingDiscipline {
    Structural,
    Propulsion,
    Weapons,
    Electronics,
    Utility,
    Shipbuilding
};

struct ManufacturingSkillProfile {
    int structural = 0;
    int propulsion = 0;
    int weapons = 0;
    int electronics = 0;
    int utility = 0;
    int shipbuilding = 0;

    int LevelFor(ManufacturingDiscipline discipline) const;
};

struct ManufacturingContext {
    ManufacturingSkillProfile skills{};
    ManufacturingDiscipline discipline = ManufacturingDiscipline::Structural;
    int stationTier = 1;             // 1..5
    float materialPurity = 1.0f;     // 0.75..1.15
    float blueprintMastery = 0.0f;   // 0..1
    ItemRarity rarityCap = ItemRarity::Legendary;
    std::string manufacturer = "PLAYER";
    std::string crafter = "PLAYER";
    std::uint32_t seed = 1;
};

struct ManufacturingResult {
    GeneratedItem item{};
    float craftsmanship = 1.0f;
    float skillContribution = 0.0f;
    float materialContribution = 0.0f;
    float stationContribution = 0.0f;
    bool exceptional = false;
};

/// Converts skill/material/station/mastery into deterministic craftsmanship.
/// Skill scales performance values and efficiency, not the authored visual
/// dimensions of a kitbash module.
class ManufacturingQualitySystem {
public:
    static ManufacturingDiscipline DisciplineFor(const ItemDefinition& definition);
    static ManufacturingResult Manufacture(const ItemDefinition& definition,
                                            const ManufacturingContext& context);
    static float Craftsmanship(const ManufacturingContext& context,
                               float* skillContribution = nullptr,
                               float* materialContribution = nullptr,
                               float* stationContribution = nullptr);
};

} // namespace subspace
