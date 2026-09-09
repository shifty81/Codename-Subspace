#include "ships/ShipyardAuthoredShipSystem.h"
namespace subspace {
const std::vector<ShipyardAuthoredShipDefinition>& ShipyardAuthoredShipSystem::Definitions(){
    static const std::vector<ShipyardAuthoredShipDefinition> v={
        {"SBS_SCOUT","Shipyard Scout","shipyard_sbs_scoutship","EXPLORATION",7.4776f,33.8902f,15.2209f,true},
        {"SBS_BATTLESHIP","Shipyard Battleship","shipyard_sbs_battleship","COMBAT",13.7072f,32.9679f,10.2643f,true},
        {"SBS_CRUISER","Shipyard Cruiser","shipyard_sbs_cruisership","HAULER",15.4893f,31.6708f,10.4426f,true}
    };return v;
}
const ShipyardAuthoredShipDefinition* ShipyardAuthoredShipSystem::Find(const std::string& id){for(const auto& d:Definitions())if(d.id==id)return &d;return nullptr;}
ProceduralShipVisualRecipe ShipyardAuthoredShipSystem::BuildRecipe(const ShipyardAuthoredShipDefinition& d){
    ProceduralShipVisualRecipe r;
    r.recipeId="AUTHORED_"+d.id;
    r.role=d.role;
    r.seed=1;
    r.sourceFamily="SHIPYARD_STRIKES_BACK_AUTHORED";
    r.acceptedByArtDirector=true;
    r.qualityScore=100.0f;

    // Pass474: the authored Scout/Battleship/Cruiser composites use the same
    // Greyoxide source-axis convention as the loose Shipyard corpus. Capturing
    // one of these ships must not silently remove the +180 visual normalization
    // that already makes generated/starter ships agree with native +Y thrust.
    r.forwardVisualYawDegrees=180.0f;
    r.forwardAuthority="AUTHORED_COCKPIT";
    r.cockpitModuleIndex=0;

    VisualModulePlacement p;
    p.moduleId=d.moduleId;
    p.scaleX=p.scaleY=p.scaleZ=0.72f;
    p.material=SpaceMaterialKind::IndustrialHull;
    r.modules.push_back(p);
    return r;
}
}
