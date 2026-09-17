#pragma once
#include "interior/ShipInteriorDerivedShellSystem.h"
#include <cstddef>

namespace subspace {
// Works only on a certified axis-aligned shell. Positions are ship-local metres,
// and represent the avatar's FEET. Does not invent stairs, doors or gravity.
class ShipInteriorShellTraversalSystem {
public:
    static bool CanOccupy(const ShipInteriorCarvePlan& carve,
                          const InteriorDerivedShell& shell,Vector3 feet,
                          float radius,float capsuleHeight);
    // Bounded microsteps and axis slide avoid tunnelling through narrow seams.
    // Invalid start/shell returns the unchanged position (fail closed).
    static Vector3 Move(const ShipInteriorCarvePlan& carve,
                        const InteriorDerivedShell& shell,Vector3 feet,
                        Vector3 delta,float radius,float capsuleHeight);
    static bool Spawn(const ShipInteriorCarvePlan& carve,
                      const InteriorDerivedShell& shell,float radius,
                      float capsuleHeight,Vector3& feet);
};
} // namespace subspace
