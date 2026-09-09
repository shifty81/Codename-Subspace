#pragma once

#include "core/Math.h"
#include "core/resources/ObjAssetLoader.h"

namespace subspace {

struct ShipyardSurfaceContact {
    Vector3 point{};
    Vector3 normal{};
    float supportingArea = 0.0f;
    float confidence = 0.0f;
    bool valid = false;
};

struct ShipyardSurfaceContactSet {
    ShipyardSurfaceContact forward;
    ShipyardSurfaceContact aft;
    ShipyardSurfaceContact port;
    ShipyardSurfaceContact starboard;
    ShipyardSurfaceContact dorsal;
    ShipyardSurfaceContact ventral;
};

/// Headless geometry analysis used by both certification and runtime assembly.
/// Contacts are derived from substantial terminal triangle bands instead of
/// raw bounding-box extrema, preventing a tiny protrusion or disconnected
/// detail island from becoming the authoritative module socket surface.
class ShipyardGeometryAnalysisSystem {
public:
    static ShipyardSurfaceContactSet Analyze(const ObjMeshData& mesh);
};

} // namespace subspace
