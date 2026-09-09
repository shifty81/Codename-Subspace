#pragma once

#include <string>
#include <vector>

namespace subspace {

enum class HotbarActionKind { Weapon, Mining, Scanner, Repair, Drone, Ewar, Utility };
struct HotbarSlot { int slot=0; std::string id; std::string label; HotbarActionKind kind=HotbarActionKind::Utility; float cooldown=0; float heat=0; float powerCost=0; int ammo=-1; bool enabled=true; bool requiresTarget=false; };
class ShipActionHotbarSystem {
public:
    std::vector<HotbarSlot> BuildDefault(const std::vector<std::string>& fittedCapabilities) const;
    bool CanActivate(const HotbarSlot& slot,bool hasTarget,float availablePower) const;
};

} // namespace subspace
