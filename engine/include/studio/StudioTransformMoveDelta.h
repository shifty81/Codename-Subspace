#pragma once
#include "ship_editor/ShipyardBuilderSystem.h"
#include "ship_editor/ShipyardTransformSpacePolicy.h"
#include "studio/StudioTransformViewBasis.h"
#include "studio/StudioGizmoMath.h"
#include "rendering/ForwardSpacePresentationSystem.h"
#include "editor/EditorTransformSpaceSystem.h"
#include <algorithm>
#include <cmath>

namespace subspace {
// Resolves the selected gizmo orientation into the coordinate system stored by
// the authoritative document. The caller can then apply this parent/authored
// delta without reinterpreting it through a second transform-space layer.
struct StudioTransformMoveDelta {
    static Vector3 AxisDelta(StudioAxis axis,float amount) noexcept {
        return {axis==StudioAxis::X?amount:0.0f,
                axis==StudioAxis::Y?amount:0.0f,
                axis==StudioAxis::Z?amount:0.0f};
    }
    static Vector3 ModelAuthored(const ShipyardBuilderRuntimeModel& model,
                                 const StrategicCamera& camera,
                                 StudioAxis axis,float amount) noexcept {
        if(model.modeling.recipe.primitives.empty())return {};
        const auto i=std::min(model.modeling.selectedPrimitiveIndex,model.modeling.recipe.primitives.size()-1);
        const auto effective=ShipyardTransformSpacePolicy::Effective(
            ShipyardTransformTool::Move,model.transformSpace,model.transformConstraintLocal);
        if(effective==ShipyardTransformSpace::Local){
            const auto basis=ConstructionTransformBasisSystem::ModelLocal(model.modeling.recipe.primitives[i].rotationDegrees);
            return ConstructionTransformBasisSystem::Axis(basis,static_cast<int>(axis))*amount;
        }
        if(effective==ShipyardTransformSpace::View){
            const auto basis=StudioTransformViewBasis::Build(camera);
            return ConstructionTransformBasisSystem::Axis(basis,static_cast<int>(axis))*amount;
        }
        return AxisDelta(axis,amount);
    }
    static Vector3 AssemblyAuthored(const ShipyardBuilderRuntimeModel& model,
                                    const StrategicCamera& camera,
                                    StudioAxis axis,float amount) noexcept {
        if(model.recipe.modules.empty())return {};
        const auto i=std::min(model.selectedPlacedModule,model.recipe.modules.size()-1);
        const auto& part=model.recipe.modules[i];
        const auto effective=ShipyardTransformSpacePolicy::Effective(
            ShipyardTransformTool::Move,model.transformSpace,model.transformConstraintLocal);
        if(effective==ShipyardTransformSpace::Local){
            const auto basis=ConstructionTransformBasisSystem::AssemblyLocal(part,true);
            return ConstructionTransformBasisSystem::Axis(basis,static_cast<int>(axis))*amount;
        }
        if(effective!=ShipyardTransformSpace::View)return AxisDelta(axis,amount);

        const std::string role=model.recipe.role.empty()?"INDUSTRIAL":model.recipe.role;
        const auto presentation=ForwardSpacePresentationSystem{}.ForShip(role,true,.22f);
        const Vector3 rootScale{
            .24f*presentation.widthScale*model.recipe.widthScale,
            .24f*presentation.lengthScale*model.recipe.lengthScale,
            .24f};
        const float yaw=EditorTransformSpaceSystem::RenderedRootYawRadians(
            0,model.recipe.forwardVisualYawDegrees);
        const auto view=StudioTransformViewBasis::Build(camera);
        const auto worldAxis=ConstructionTransformBasisSystem::Axis(view,static_cast<int>(axis));
        const auto unyawed=ConstructionTransformBasisSystem::RotateZ(worldAxis,-yaw);
        const float sx=std::max(1.0e-5f,std::fabs(rootScale.x));
        const float sy=std::max(1.0e-5f,std::fabs(rootScale.y));
        const float sz=std::max(1.0e-5f,std::fabs(rootScale.z));
        return {unyawed.x/sx*amount,unyawed.y/sy*amount,unyawed.z/sz*amount};
    }
};
} // namespace subspace
