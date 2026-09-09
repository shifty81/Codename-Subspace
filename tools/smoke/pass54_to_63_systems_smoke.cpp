#include "celestial/CelestialSystemGenerator.h"
#include "celestial/SectorResourceModel.h"
#include "scanning/SectorScanner.h"
#include "mining/MiningSalvageModel.h"
#include "combat/CombatSimulation.h"
#include "trading/StationEconomy.h"
#include "factions/FactionEncounterModel.h"
#include "procedural/ProceduralEncounterGenerator.h"
#include "core/persistence/RuntimeSaveGame.h"
#include "ui/ClientHudModel.h"
#include "ship_editor/ShipBuilderVisualModel.h"
#include "developer/assets/ContentDataNormalization.h"

#include <cassert>
#include <iostream>

int main() {
    subspace::CelestialSystemGeneratorOptions options;
    options.allowBlackHolePrimary = true;
    subspace::CelestialSystemGenerator generator(options);
    auto system = generator.GenerateSystem("SMOKE-54-63", 5463u);
    auto survey = subspace::BuildSectorResourceSurvey(system);
    assert(!survey.resources.empty());
    assert(!survey.asteroidField.dominantResource.empty());

    subspace::SectorScanner scanner;
    auto report = scanner.ScanSystem(system, survey, 2);
    assert(!report.contacts.empty());

    subspace::MiningToolProfile tool;
    tool.canSalvage = true;
    subspace::MiningTargetProfile target;
    target.resourceTag = survey.asteroidField.dominantResource;
    target.remainingIntegrity = 10.0f;
    target.radius = 42.0f;
    target.richness = survey.asteroidField.resourceRichness;
    auto yield = subspace::GenerateMiningYield(survey.asteroidField, target, tool, 77u);
    assert(yield.fractured);
    assert(!yield.cargo.empty());

    auto station = subspace::BuildStationEconomyFromSurvey(survey);
    auto quote = subspace::QuoteCargoSale(station, yield.cargo);
    assert(quote.totalCredits > 0);

    subspace::ShieldProfile shield;
    shield.shield = 10.0f;
    shield.armor = 5.0f;
    shield.hull = 20.0f;
    subspace::WeaponProfile weapon;
    weapon.damage = 18.0f;
    auto hit = subspace::ResolveWeaponHit(weapon, shield, 1u);
    assert(hit.shieldDamage > 0.0f || hit.hullDamage > 0.0f);

    auto encounters = subspace::BuildEncounterSpawnTable(survey, 99u);
    subspace::ProceduralEncounterGenerator encounterGenerator;
    auto encounter = encounterGenerator.GenerateEncounter(encounters, 2, 111u);
    assert(!encounter.id.empty());

    subspace::RuntimeSaveGameSnapshot save;
    save.sectorId = system.id;
    save.sectorSeed = system.seed;
    save.credits = quote.totalCredits;
    save.cargo = yield.cargo;
    auto text = subspace::SerializeRuntimeSaveGameSnapshot(save);
    auto loaded = subspace::DeserializeRuntimeSaveGameSnapshot(text);
    assert(loaded.sectorId == save.sectorId);

    auto hud = subspace::BuildClientHudPanel(system.id, quote.totalCredits, 3, 88.0f, 120.0f, subspace::SectorResourceSummary(survey));
    assert(!hud.lines.empty());

    auto builder = subspace::BuildDefaultShipBuilderVisualState();
    assert(!builder.palette.empty());

    auto content = subspace::BuildDefaultContentNormalizationPlan();
    assert(!content.actions.empty());

    std::cout << "Pass54-63 systems smoke passed: " << subspace::SectorResourceSummary(survey) << "\n";
    return 0;
}
