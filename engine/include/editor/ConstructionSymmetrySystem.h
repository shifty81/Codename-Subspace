#pragma once

#include "content/ShipyardModuleSystem.h"
#include "rendering/ProceduralVisualVariantSystem.h"

#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

enum class ConstructionSymmetryAxis {
    PortStarboard = 0, // reflect ship-local X
    ForeAft,           // reflect ship-local Y
    DorsalVentral      // reflect ship-local Z
};

struct ConstructionSymmetryFrame {
    Vector3 origin{};
    ConstructionSymmetryAxis axis = ConstructionSymmetryAxis::PortStarboard;
    bool live = true;
};

struct ConstructionSymmetryPair {
    std::size_t first = 0;
    std::size_t second = 0;
    ConstructionSymmetryAxis axis = ConstructionSymmetryAxis::PortStarboard;
    bool linked = true;
};

struct ConstructionSymmetryValidation {
    bool valid = false;
    float positionError = 0.0f;
    std::string message;
};

/// Universal construction symmetry authority used by Shipyard, stations,
/// planetary construction and weapon/turret authoring. Reflection is geometric
/// handedness, not duplicate+180-degree rotation.
class ConstructionSymmetrySystem {
public:
    static const char* AxisName(ConstructionSymmetryAxis axis);
    static Vector3 ReflectPoint(const Vector3& point, const ConstructionSymmetryFrame& frame);
    static Vector3 ReflectDirection(const Vector3& direction, ConstructionSymmetryAxis axis);
    static VisualModulePlacement ReflectPlacement(const VisualModulePlacement& placement,
                                                   const ConstructionSymmetryFrame& frame);
    static ShipyardAssemblySocket ReflectSocket(const ShipyardAssemblySocket& socket,
                                                 ConstructionSymmetryAxis axis);
    static std::string ReflectSocketName(const std::string& name, ConstructionSymmetryAxis axis);
    static ConstructionSymmetryValidation ValidatePair(const VisualModulePlacement& first,
                                                        const VisualModulePlacement& second,
                                                        const ConstructionSymmetryFrame& frame,
                                                        float tolerance = 0.02f);
};

} // namespace subspace
