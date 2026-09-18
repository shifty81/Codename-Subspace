#pragma once
namespace subspace {
enum class ShipyardStudioViewMode { Exterior=0, Cutaway, InteriorOnly, XRay };
struct ShipyardStudioViewSystem {
    static constexpr ShipyardStudioViewMode Next(ShipyardStudioViewMode m){
        return static_cast<ShipyardStudioViewMode>((static_cast<int>(m)+1)%4);
    }
    // Interior-only is an inspection view; exterior workspaces must show hulls.
    static constexpr ShipyardStudioViewMode ForWorkspace(ShipyardStudioViewMode current,bool interior){
        if(interior)return current==ShipyardStudioViewMode::Exterior?ShipyardStudioViewMode::InteriorOnly:current;
        return current==ShipyardStudioViewMode::InteriorOnly?ShipyardStudioViewMode::Exterior:current;
    }
    // Placement requires an exterior hull or ghost, even when the previous
    // view was set to Interior-only. Explicit Cutaway/X-Ray remain unchanged.
    static constexpr ShipyardStudioViewMode ForPlacement(ShipyardStudioViewMode current){
        return current==ShipyardStudioViewMode::InteriorOnly?ShipyardStudioViewMode::Exterior:current;
    }
    // Native game and standalone Studio select the catalog card on pointer
    // press BEFORE crossing the drag threshold. Reveal the ghost at this point,
    // without forcibly changing the dedicated Interior workspace's view.
    static constexpr ShipyardStudioViewMode ForCatalogPress(ShipyardStudioViewMode current,bool interiorWorkspace){
        return interiorWorkspace?current:ForPlacement(current);
    }
    static constexpr const char* Name(ShipyardStudioViewMode m){
        switch(m){
        case ShipyardStudioViewMode::Exterior:return "EXTERIOR";
        case ShipyardStudioViewMode::Cutaway:return "CUTAWAY";
        case ShipyardStudioViewMode::InteriorOnly:return "INTERIOR";
        case ShipyardStudioViewMode::XRay:return "X-RAY";
        }
        return "EXTERIOR";
    }
};
} // namespace subspace
