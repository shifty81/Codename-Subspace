#pragma once

#include "ships/ModuleDef.h"
#include "factions/FactionProfile.h"

#include <string>
#include <vector>

namespace subspace {

enum class ArchetypeClass { Interceptor, Frigate, Freighter, Cruiser, Battleship };

struct ShipArchetype {
    std::string    id;
    ArchetypeClass archClass;
    int            minModules;
    int            maxModules;
    int            minWeapons;
    int            maxWeapons;
    int            minEngines;
    float          targetMass;
    float          aggressiveness; // 0.0 = peaceful hauler, 1.0 = full combat
};

class ShipArchetypes {
public:
    static ShipArchetype Interceptor();
    static ShipArchetype FrigateArchetype();
    static ShipArchetype Freighter();
    static ShipArchetype CruiserArchetype();
    static ShipArchetype BattleshipArchetype();

    static std::vector<ShipArchetype> GetAll();
};

class ModularShipGenerator {
public:
    ModularShipGenerator(const ShipArchetype& archetype,
                         const FactionProfile& faction,
                         int seed);

    ModularShip Generate();

private:
    void PlaceCore(ModularShip& ship);
    void GrowStructure(ModularShip& ship);
    void AddEngines(ModularShip& ship);
    void AddWeapons(ModularShip& ship);
    void FillRemaining(ModularShip& ship);
    bool ValidateShip(const ModularShip& ship) const;

    const ModuleDef* PickModuleForSlot(ModuleType preferred) const;
    Vector3 NextHardpointPosition(const ModularShip& ship, int moduleIndex) const;

    int   NextRandom();
    float NextRandomFloat();

    ShipArchetype  m_archetype;
    FactionProfile m_faction;
    int            m_seed;
    int            m_rngState;
};

} // namespace subspace
