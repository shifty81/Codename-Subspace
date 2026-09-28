#pragma once
#include "ship_editor/ShipyardTransformSpacePolicy.h"
#include <string>

namespace subspace {
struct StudioTransformStatusPolicy {
    static constexpr const char* SpaceName(ShipyardTransformSpace space) noexcept {
        return ShipyardTransformSpacePolicy::Name(space);
    }
    static constexpr const char* ToolName(ShipyardTransformTool tool) noexcept {
        switch(tool){
        case ShipyardTransformTool::Select:return "SELECT";
        case ShipyardTransformTool::Move:return "MOVE";
        case ShipyardTransformTool::Rotate:return "ROTATE";
        case ShipyardTransformTool::Scale:return "SCALE";
        }
        return "SELECT";
    }
    static constexpr ShipyardTransformSpace EffectiveSpace(ShipyardTransformTool tool,
                                                            ShipyardTransformSpace selected,
                                                            bool localConstraint=false) noexcept {
        return ShipyardTransformSpacePolicy::Effective(tool,selected,localConstraint);
    }
    static constexpr const char* EffectiveSpaceName(ShipyardTransformTool tool,
                                                     ShipyardTransformSpace effective) noexcept {
        return tool==ShipyardTransformTool::Rotate ? "EULER" : SpaceName(effective);
    }
    static constexpr bool HasEffectiveOverride(ShipyardTransformTool tool,
                                                ShipyardTransformSpace selected,
                                                bool localConstraint=false) noexcept {
        if(tool==ShipyardTransformTool::Rotate)return true;
        return EffectiveSpace(tool,selected,localConstraint)!=selected;
    }
    static std::string SelectedLabel(ShipyardTransformSpace selected){
        return std::string("SPACE ")+SpaceName(selected);
    }
    static std::string EffectiveLabel(ShipyardTransformTool tool,
                                      ShipyardTransformSpace selected,
                                      bool localConstraint=false){
        const auto effective=EffectiveSpace(tool,selected,localConstraint);
        return std::string(ToolName(tool))+" "+EffectiveSpaceName(tool,effective);
    }
};
} // namespace subspace
