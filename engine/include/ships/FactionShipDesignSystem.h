#pragma once

#include "appearance/FactionAppearanceFamilySystem.h"
#include "ships/ShipClassRoleSystem.h"
#include "ships/ShipPcgRuntimeClosureSystem.h"
#include "ships/ShipyardDesignDnaSystem.h"

#include <string>
#include <vector>

namespace subspace {

struct FactionDesignDna {
    std::string factionId;
    float angularity = 0.5f;
    float longitudinalBias = 0.5f;
    float exposedStructure = 0.25f;
    float armorDensity = 0.5f;
    float symmetryPreference = 0.8f;
    float enginePairPreference = 0.7f;
    float sensorExposure = 0.4f;
    std::vector<std::string> preferredStyleTags;
};

struct HullFamilyExemplarRecord {
    std::string exemplarId;
    std::string familyId;
    std::string factionId;
    ShipClass shipClass = ShipClass::Frigate;
    ShipRole role = ShipRole::GeneralCombat;
    ShipyardDesignExemplar exemplar{};
    bool approved = true;
};

struct HullFamilyCompiledGrammar {
    HullFamilyRuntimeProfile runtime{};
    ShipyardDesignGrammar grammar{};
    FactionDesignDna factionDna{};
    std::vector<std::string> exemplarIds;
};

class FactionShipDesignSystem {
public:
    // Pass632-638: faction -> class -> four hull families -> role variants.
    static FactionDesignDna DefaultFactionDna(const std::string& factionId);
    static std::vector<FactionHullFamilyDefinition> BuildClassFamilies(const std::string& factionId,
                                                                       ShipClass shipClass,
                                                                       const FactionDesignDna& dna);
    static HullFamilyExemplarRecord CaptureExemplar(const std::string& exemplarId,
                                                    const FactionHullFamilyDefinition& family,
                                                    ShipRole role,
                                                    const std::vector<ShipyardModuleRecord>& catalog,
                                                    const ProceduralShipVisualRecipe& recipe);
    static HullFamilyCompiledGrammar Compile(const FactionHullFamilyDefinition& family,
                                             const FactionDesignDna& dna,
                                             const std::vector<HullFamilyExemplarRecord>& exemplars);
    static bool VariantPreservesLineage(const ProceduralShipVisualRecipe& recipe,
                                        const HullFamilyCompiledGrammar& family,
                                        ShipRole role);
};

} // namespace subspace
