#pragma once
namespace subspace {
enum class ShipyardStudioViewMode { Exterior=0, Cutaway, InteriorOnly, XRay };
struct ShipyardStudioViewSystem {
    static constexpr ShipyardStudioViewMode Next(ShipyardStudioViewMode m){
        return static_cast<ShipyardStudioViewMode>((static_cast<int>(m)+1)%4);
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
