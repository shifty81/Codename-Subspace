#pragma once
#include "ships/Ship.h"
namespace subspace {
struct ModuleCapabilityFractions { float mainThrust=1.0f; float maneuvering=1.0f; float power=1.0f; float shields=1.0f; float sensors=1.0f; float weapons=1.0f; float cargo=1.0f; };
class ModuleCapabilityDamageSystem { public: ModuleCapabilityFractions Evaluate(const Ship& ship) const; };
}
