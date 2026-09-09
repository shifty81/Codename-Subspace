#include "home/HomeFactoryNetwork.h"
#include "home/HomeSolarSystem.h"
#include "home/HomeSurfaceBuilder.h"

#include <cassert>
#include <iostream>

int main() {
    subspace::HomeSolarSystemState home = subspace::CreateDefaultHomeSolarSystem(0x77u);
    subspace::HomeFactoryNetworkState factory = subspace::CreateStarterHomeFactoryNetwork(home);
    subspace::AddHomeInventory(factory, "hull-plate", 20);
    subspace::AddHomeInventory(factory, "ingot", 20);
    subspace::AddHomeInventory(factory, "recovered-parts", 10);
    subspace::AddHomeInventory(factory, "module-component", 10);

    const auto* zone = subspace::FindHomeBuildZone(home, "home-world-surface-a");
    assert(zone != nullptr);
    const auto palette = subspace::CreateHomeBuildPalette(zone->type);
    assert(!palette.empty());

    subspace::HomeBuildPlacementRequest request;
    request.zoneId = zone->id;
    request.type = subspace::HomeStructureType::ConveyorHub;
    request.x = 28;
    request.y = 30;
    const auto placed = subspace::PlaceHomeStructure(home, factory, request);
    assert(placed.success);
    assert(subspace::FindHomeStructureAt(home, zone->id, 28, 30) != nullptr);

    const auto occupied = subspace::PlaceHomeStructure(home, factory, request);
    assert(!occupied.success);
    assert(occupied.status == subspace::HomePlacementStatus::Occupied);

    const auto removed = subspace::RemoveHomeStructure(home, factory, zone->id, 28, 30);
    assert(removed.success);
    assert(subspace::FindHomeStructureAt(home, zone->id, 28, 30) == nullptr);

    std::cout << "Pass77 home surface builder smoke passed\n";
    return 0;
}
