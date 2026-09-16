#pragma once
#include "core/Math.h"
#include "editor/AuthoringStandardsSystem.h"
#include "interior/ShipInteriorAuthoringSystem.h"
#include <cstddef>
#include <string>
#include <vector>

namespace subspace {

enum class ShipInteriorElementKind {
    Floor,
    Wall,
    Ceiling,
    Door,
    Hatch,
    Airlock,
    Window,
    Ramp,
    Stair,
    Ladder,
    Elevator,
    Corridor,
    Bulkhead,
    Custom
};

enum class ShipInteriorElementShape { Box, Wedge, Cylinder, CustomModel };

struct ShipInteriorStructuralElement {
    std::string id;
    ShipInteriorElementKind kind = ShipInteriorElementKind::Floor;
    ShipInteriorElementShape shape = ShipInteriorElementShape::Box;
    std::size_t moduleIndex = 0;
    int deck = 0;
    Vector3 position{};
    Vector3 rotationDegrees{};
    Vector3 sizeMeters{2.0f,2.0f,.12f};
    std::string materialId = "INTERIOR_FLOOR_METAL";
    std::string modelAssetId;
    std::string portalId;
    bool pressureBoundary = false;
    bool openable = false;
    bool removable = true;
};

struct ShipInteriorStructuralModel {
    std::vector<ShipInteriorStructuralElement> elements;
    std::size_t selected = 0;
    std::uint64_t revision = 1;
    bool dirty = false;
    std::string status = "Interior structure ready";
};

struct ShipInteriorStructureValidation {
    bool valid = false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

class ShipInteriorStructureAuthoringSystem {
public:
    static const char* KindName(ShipInteriorElementKind kind);
    static ShipInteriorStructuralElement DefaultElement(ShipInteriorElementKind kind,
                                                        std::size_t moduleIndex,
                                                        int deck,
                                                        std::size_t ordinal);
    static ShipInteriorStructuralModel GenerateDefaults(const GeneratedShipInteriorProgram& program);
    static std::size_t Add(ShipInteriorStructuralModel& model,ShipInteriorElementKind kind,
                           std::size_t moduleIndex,int deck);
    static bool RemoveSelected(ShipInteriorStructuralModel& model);
    static bool MoveSelected(ShipInteriorStructuralModel& model,const Vector3& delta);
    static bool RotateSelected(ShipInteriorStructuralModel& model,const Vector3& deltaDegrees);
    static bool ResizeSelected(ShipInteriorStructuralModel& model,const Vector3& delta);
    static bool AssignMaterial(ShipInteriorStructuralModel& model,const std::string& materialId);
    static ShipInteriorStructureValidation Validate(const ShipInteriorStructuralModel& model,
                                                    const WorldScaleProfile& scale=WorldScaleAuthoritySystem::DefaultProfile());
};

} // namespace subspace
