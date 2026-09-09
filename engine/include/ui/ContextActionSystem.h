#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class ContextObjectKind { EmptySpace, Ship, HostileShip, Asteroid, Station, Planet, Moon, Wreck, Poi, MapDestination, Hex, StationModule };

struct InteractionContext {
    ContextObjectKind kind=ContextObjectKind::EmptySpace;
    bool owned=false;
    bool discovered=true;
    bool dockable=false;
    bool targetable=true;
    bool mineable=false;
    bool salvageable=false;
    bool fleetAvailable=false;
    bool capturable=false;
    bool disabled=false;
    float distance=0.0f;
};

struct ContextAction { std::string id; std::string label; bool enabled=true; std::string disabledReason; };

class ContextActionSystem {
public:
    std::vector<ContextAction> Resolve(const InteractionContext& context) const;
};

} // namespace subspace
