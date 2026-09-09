#pragma once

#include "ai/AIDecisionSystem.h"
#include "ships/ShipModuleLibrary.h"

#include <cstdint>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

namespace subspace {

enum class ModularShipSize {
    Fighter,
    Corvette,
    Frigate,
    Destroyer,
    Cruiser,
    Battleship,
    Carrier
};

enum class ModularShipRole {
    Multipurpose,
    Combat,
    Mining,
    Trading,
    Exploration,
    Salvage
};

struct ModularShipConfig {
    std::string shipName = "Unnamed Ship";
    ModularShipSize size = ModularShipSize::Frigate;
    ModularShipRole role = ModularShipRole::Multipurpose;
    std::string material = "Iron";
    std::uint32_t seed = 0;
    bool addWings = true;
    bool addWeapons = true;
    bool addCargo = true;
    bool addHyperdrive = true;
    int minimumEngines = 1;
    int desiredWeaponMounts = 2;
};

struct ModularGeneratedShip {
    std::string entityId;
    std::string name;
    ModularShipConfig config;
    std::vector<ShipModulePart> modules;
    std::string coreModuleId;
    ModuleFunctionalStats compiledStats;
    float totalMass = 0.0f;
    float totalHealth = 0.0f;
    std::vector<std::string> warnings;
    std::unordered_map<std::string, int> moduleCounts;

    bool Empty() const;
    std::size_t ModuleCount() const;
};

class ModularShipFactory {
public:
    explicit ModularShipFactory(ShipModuleLibrary& library, std::uint32_t seed = 0);

    ModularGeneratedShip CreateShipForAI(AIPersonality personality,
                                         const std::string& name,
                                         const std::string& material = "Iron");
    ModularGeneratedShip CreateFighter(const std::string& name, const std::string& material = "Iron");
    ModularGeneratedShip CreateMiner(const std::string& name, const std::string& material = "Iron");
    ModularGeneratedShip CreateTrader(const std::string& name, const std::string& material = "Iron");
    ModularGeneratedShip CreateCapitalShip(const std::string& name, const std::string& material = "Titanium");
    ModularGeneratedShip CreateCustomShip(const ModularShipConfig& config);

    static ModularShipConfig CreateConfigForPersonality(AIPersonality personality,
                                                        const std::string& name,
                                                        const std::string& material,
                                                        std::uint32_t seed);

private:
    ShipModuleLibrary& m_library;
    std::mt19937 m_random;
    std::uint32_t m_nextId = 1;

    std::uint32_t NextSeed();
    std::string NextModuleId();
    ShipModulePart MakePart(const ShipModuleDefinition& definition,
                            Vector3 position,
                            const std::string& material,
                            const std::string& attachment = "");
    void AddPart(ModularGeneratedShip& ship,
                 const std::string& definitionId,
                 Vector3 position,
                 const std::string& attachment = "");
    void CompileGeneratedStats(ModularGeneratedShip& ship) const;
};

int HullSectionCountForSize(ModularShipSize size);
int EngineCountForSize(ModularShipSize size, int minimumEngines);
ModuleShipClass ToModuleShipClass(ModularShipSize size, ModularShipRole role);
std::string ModularShipSizeName(ModularShipSize size);
std::string ModularShipRoleName(ModularShipRole role);

} // namespace subspace
