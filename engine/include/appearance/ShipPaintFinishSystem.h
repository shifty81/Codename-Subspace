#pragma once
#include "ship_editor/ShipyardEquipmentSystem.h"
#include <string>
#include <vector>

namespace subspace {

struct ShipPaintFinishPreset {
    ShipPaintFinish finish = ShipPaintFinish::SatinAlloy;
    std::string id;
    std::string label;
    float metallic = .65f;
    float roughness = .34f;
    float clearcoat = 0.0f;
    float clearcoatRoughness = .2f;
    float specular = .5f;
    float anisotropy = 0.0f;
    float iridescence = 0.0f;
    float iridescenceIor = 1.3f;
    float iridescenceThicknessNm = 400.0f;
};

class ShipPaintFinishSystem {
public:
    static const std::vector<ShipPaintFinishPreset>& Presets();
    static const ShipPaintFinishPreset& Describe(ShipPaintFinish finish);
    static const char* Name(ShipPaintFinish finish);
    static void Apply(ShipPaintLayer& layer, ShipPaintFinish finish);
    static ShipPaintFinish Next(ShipPaintFinish finish, int delta = 1);
};

} // namespace subspace
