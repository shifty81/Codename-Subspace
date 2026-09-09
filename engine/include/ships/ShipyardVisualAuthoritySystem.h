#pragma once
#include <string>
namespace subspace {
class ShipyardVisualAuthoritySystem {
public:
    static constexpr bool LegacyShipGeometryAllowed(){ return false; }
    static bool IsForbiddenLegacyVisualId(const std::string& id){
        return id=="hull_section"||id=="hull_section_small"||id=="cockpit_basic"||id=="cockpit_small"||
               id=="engine_main"||id=="engine_small"||id=="thruster"||id=="thruster_small"||id=="cargo_bay";
    }
    static constexpr const char* AuthorityName(){ return "SHIPYARD_ONLY"; }
};
}
