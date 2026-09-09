#pragma once

#include "content/ShipyardModuleSystem.h"
#include "ship_editor/ShipBlueprintLibrarySystem.h"

#include <string>
#include <vector>

namespace subspace {

/// Shared interchange serializer between the native in-game Shipyard and the
/// Blender SubspaceShipyard extension.  This deliberately emits the exact
/// `subspace.shipyard_design` v1 envelope used by Blender so approved player
/// designs can be inspected, re-opened, certified and promoted without a
/// second blueprint dialect.
class ShipyardDesignExchangeSystem {
public:
    static std::string Serialize(const ShipBlueprintDocument& blueprint,
                                 const std::vector<ShipyardModuleRecord>& catalog,
                                 const std::string& toolVersion = "native");
    static bool Save(const ShipBlueprintDocument& blueprint,
                     const std::vector<ShipyardModuleRecord>& catalog,
                     const std::string& path,
                     std::string* error = nullptr,
                     const std::string& toolVersion = "native");
};

} // namespace subspace
