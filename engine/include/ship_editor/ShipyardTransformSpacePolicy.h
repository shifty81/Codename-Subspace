#pragma once
#include "ship_editor/ShipyardTransformSystem.h"

namespace subspace {
struct ShipyardTransformSpacePolicy {
    static constexpr ShipyardTransformSpace Default() noexcept { return ShipyardTransformSpace::Ship; }
    static constexpr const char* Name(ShipyardTransformSpace space) noexcept {
        switch(space){
        case ShipyardTransformSpace::View:return "VIEW";
        case ShipyardTransformSpace::Ship:return "PARENT";
        case ShipyardTransformSpace::Local:return "OBJECT";
        }
        return "PARENT";
    }
    static constexpr ShipyardTransformSpace Next(ShipyardTransformSpace space) noexcept {
        switch(space){
        case ShipyardTransformSpace::Ship:return ShipyardTransformSpace::Local;
        case ShipyardTransformSpace::Local:return ShipyardTransformSpace::View;
        case ShipyardTransformSpace::View:return ShipyardTransformSpace::Ship;
        }
        return ShipyardTransformSpace::Ship;
    }
    static constexpr ShipyardTransformSpace Effective(ShipyardTransformTool tool,
                                                       ShipyardTransformSpace selected,
                                                       bool localConstraint=false) noexcept {
        // Existing rotation storage is Euler-component based. Until a true
        // axis-angle/quaternion transaction lands, Rotate must not advertise a
        // spatial OBJECT/VIEW frame it cannot faithfully compose.
        if(tool==ShipyardTransformTool::Rotate)return ShipyardTransformSpace::Ship;
        if(tool==ShipyardTransformTool::Scale)return ShipyardTransformSpace::Local;
        if(tool==ShipyardTransformTool::Move&&localConstraint)return ShipyardTransformSpace::Local;
        return selected;
    }
    static constexpr bool FollowsObject(ShipyardTransformTool tool,
                                        ShipyardTransformSpace selected,
                                        bool localConstraint=false) noexcept {
        return Effective(tool,selected,localConstraint)==ShipyardTransformSpace::Local;
    }
};
} // namespace subspace
