#pragma once

#include <string>
#include <vector>

namespace subspace {

struct CombatComponent;

struct ShipCommandModuleState {
    std::string label;
    std::string shortcut;
    float cycle = 0.0f;
    bool active = false;
    bool offline = false;
    bool requiresTarget = true;
};

struct ShipCommandHudModel {
    float shield = 1.0f;
    float armor = 1.0f;
    float hull = 1.0f;
    float power = 1.0f;
    float speed = 0.0f;
    float maximumSpeed = 30.0f;
    bool dampeners = true;
    bool boost = false;
    bool shieldOnline = true;
    bool critical = false;
    std::string integrityStatus = "NOMINAL";
    std::vector<ShipCommandModuleState> modules;
};

class ShipCommandHudSystem {
public:
    static ShipCommandHudModel Build(float speed,bool dampeners,bool boost,const std::vector<std::string>& moduleLabels);
    static ShipCommandHudModel Build(float speed,bool dampeners,bool boost,
                                     const std::vector<std::string>& moduleLabels,
                                     const CombatComponent* combat);
};

} // namespace subspace
