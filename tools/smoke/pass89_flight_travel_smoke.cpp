#include "travel/InterstellarRailTravel.h"
#include "ships/ShipPartCatalog.h"

#include <cassert>
#include <iostream>

int main()
{
    auto catalog = subspace::CreateStarterShipPartCatalog();
    auto loadout = subspace::CreateStarterShipLoadout();
    auto stats = subspace::CalculateShipPartStats(loadout, catalog);
    assert(stats.fuelCapacity > 0.0f);
    assert(stats.thrust > 0.0f);

    auto routes = subspace::CreateStarterRailTravelRoutes("HOME-SOL", "EXP", 1234u);
    assert(routes.size() >= 3);

    subspace::ShipRailTravelFit fit;
    fit.driveTier = 3;
    fit.fuelAvailable = 100.0f;
    fit.fuelCapacity = 100.0f;
    fit.cargoCapacity = stats.cargoCapacity;
    fit.defenseRating = 4;
    fit.scannerRating = 4;
    fit.thrustRating = stats.thrust / 100.0f;
    fit.mass = static_cast<float>(stats.mass);
    fit.hasRailDrive = true;

    auto report = subspace::EvaluateRailTravelFit(routes.front(), fit);
    assert(report.canLaunch);

    auto travel = subspace::StartRailTravel(routes.front(), fit);
    assert(travel.state == subspace::RailTravelState::Traveling);
    for (int i = 0; i < 200 && travel.state == subspace::RailTravelState::Traveling; ++i) {
        subspace::TickRailTravel(travel, 0.5f);
    }
    assert(travel.state == subspace::RailTravelState::Completed);
    assert(travel.cargoCollected > 0.0f);
    std::cout << "Pass89 flight/travel smoke passed\n";
    return 0;
}
