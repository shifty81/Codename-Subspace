#include "ship_editor/ShipyardPaintZoneSystem.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace subspace {
namespace {
std::string Lower(std::string_view v){
    std::string out(v);
    std::transform(out.begin(),out.end(),out.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
    return out;
}
}

ShipyardPaintZone ShipyardPaintZoneSystem::ForMaterial(std::string_view materialName){
    const auto n=Lower(materialName);
    // The actual certified Greyoxide corpus uses Blender's generic material
    // names almost everywhere: Flat (153/156 source objects), Flat.001 on a
    // handful of secondary regions, and Glass. Treating Flat as "inherited"
    // silently preserved the neutral source gray and made large parts look
    // untextured even though the MTL had loaded successfully.
    if(n=="flat.001" || n=="flat_001" || n=="mat_seco" || n.find("secondary")!=std::string::npos || n.find("seco")!=std::string::npos)
        return ShipyardPaintZone::Secondary;
    if(n=="flat" || n=="painted" || n=="default" || n=="mat_main" ||
       n.find("primary")!=std::string::npos || n.find("paint")!=std::string::npos ||
       n.find("hull")!=std::string::npos || n.find("body")!=std::string::npos)
        return ShipyardPaintZone::Primary;
    if(n.find("trim")!=std::string::npos || n.find("accent")!=std::string::npos || n.find("stripe")!=std::string::npos)
        return ShipyardPaintZone::Trim;
    if(n=="mat_dark" || n.find("structural")!=std::string::npos || n.find("dark")!=std::string::npos ||
       n.find("vent")!=std::string::npos || n.find("grille")!=std::string::npos || n.find("cavity")!=std::string::npos)
        return ShipyardPaintZone::StructuralDark;
    if(n=="mat_metall" || n.find("metallight")!=std::string::npos || n.find("metal_l")!=std::string::npos)
        return ShipyardPaintZone::LightMetal;
    if(n=="mat_metald" || n.find("metaldark")!=std::string::npos || n.find("metal_d")!=std::string::npos)
        return ShipyardPaintZone::DarkMetal;
    if(n=="glow_whi" || n.find("emissive_white")!=std::string::npos) return ShipyardPaintZone::EmissiveWhite;
    if(n=="glow_blu" || n.find("emissive_blue")!=std::string::npos || n.find("glow_blue")!=std::string::npos) return ShipyardPaintZone::EmissiveBlue;
    if(n=="glow_ora" || n.find("emissive_orange")!=std::string::npos || n.find("glow_orange")!=std::string::npos) return ShipyardPaintZone::EmissiveOrange;
    if(n=="glass" || n.find("glass")!=std::string::npos || n.find("window")!=std::string::npos || n.find("canopy")!=std::string::npos)
        return ShipyardPaintZone::Glass;
    return ShipyardPaintZone::Inherited;
}

const char* ShipyardPaintZoneSystem::Name(ShipyardPaintZone zone){
    switch(zone){
    case ShipyardPaintZone::Primary:return "PRIMARY";
    case ShipyardPaintZone::Secondary:return "SECONDARY";
    case ShipyardPaintZone::Trim:return "TRIM";
    case ShipyardPaintZone::StructuralDark:return "STRUCTURAL DARK";
    case ShipyardPaintZone::LightMetal:return "LIGHT METAL";
    case ShipyardPaintZone::DarkMetal:return "DARK METAL";
    case ShipyardPaintZone::EmissiveWhite:return "EMISSIVE WHITE";
    case ShipyardPaintZone::EmissiveBlue:return "EMISSIVE BLUE";
    case ShipyardPaintZone::EmissiveOrange:return "EMISSIVE ORANGE";
    case ShipyardPaintZone::Glass:return "GLASS";
    case ShipyardPaintZone::Inherited:return "INHERITED";
    }
    return "INHERITED";
}

bool ShipyardPaintZoneSystem::IsPaintable(ShipyardPaintZone zone){
    return zone==ShipyardPaintZone::Primary || zone==ShipyardPaintZone::Secondary || zone==ShipyardPaintZone::Trim;
}

} // namespace subspace
