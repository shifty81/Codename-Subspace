#include "home/HomeClientViewModel.h"
#include "roguelite/RogueliteDirector.h"

#include <iostream>
#include <stdexcept>

int main()
{
    auto director = subspace::CreateRogueliteDirector(0x7600u);
    auto view = subspace::BuildHomeClientViewModel(director, 12.0f);
    if (view.bodies.empty()) {
        throw std::runtime_error("expected home celestial bodies");
    }
    if (view.buildZones.empty()) {
        throw std::runtime_error("expected home build zones");
    }
    if (view.structures.empty()) {
        throw std::runtime_error("expected starter home structures");
    }
    if (view.runOffers.empty()) {
        throw std::runtime_error("expected expedition run offers");
    }
    if (subspace::HomeClientViewSummary(view).empty()) {
        throw std::runtime_error("expected non-empty summary");
    }
    std::cout << "Pass76 home client view smoke passed\n";
    return 0;
}
