#include "studio/StudioAxisGizmo.h"
#include "studio/StudioGizmoProjectionPolicy.h"
#include "application/NativeBattlefieldRenderer.h"
#include "editor/ConstructionTransformBasisSystem.h"
#include "editor/EditorTransformSpaceSystem.h"
#include "rendering/ForwardSpacePresentationSystem.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace subspace {
namespace {
void PopulateHandles(StudioGizmoSnapshot& snapshot,const StrategicCamera& camera,
                     int width,int height,const Vector3& origin,
                     const ConstructionTransformBasis& axisWorldUnits,float probeMeters,
                     const StrategicScreenPoint& center){
    std::array<StudioPoint,3> directions{};
    std::array<StudioPoint,3> perUnit{};
    std::array<bool,3> projectable{};
    for(std::size_t i=0;i<directions.size();++i){
        const auto worldUnit=ConstructionTransformBasisSystem::Axis(axisWorldUnits,static_cast<int>(i));
        const auto fallback=i==1?Vector3{0,1,0}:(i==2?Vector3{0,0,1}:Vector3{1,0,0});
        const auto worldDirection=ConstructionTransformBasisSystem::Normalize(worldUnit,fallback);
        const auto probe=NativeBattlefieldRenderer::WorldToScreen(
            origin+worldDirection*probeMeters,width,height,camera);
        const StudioPoint point{probe.x,probe.y};
        // `visible` means the whole probe is inside the camera frustum, which
        // is NOT required for a 68-pixel on-screen handle. Accept finite
        // projected coordinates even when the long probe is off-screen.
        const bool usable=StudioGizmoProjectionPolicy::ProbeUsable(probe.depth,point);
        const StudioPoint delta=usable?StudioPoint{point.x-center.x,point.y-center.y}
                                      :StudioPoint{std::numeric_limits<float>::quiet_NaN(),0};
        directions[i]=StudioGizmoProjectionPolicy::Direction(static_cast<StudioAxis>(i),delta);
        // Project the ACTUAL world vector produced by one local authoring unit.
        // This is important for rotated parts under anisotropic ship width /
        // length scaling: a local X/Y handle must follow the geometry it edits.
        const auto unit=NativeBattlefieldRenderer::WorldToScreen(
            origin+worldUnit,width,height,camera);
        const StudioPoint pixel{unit.x,unit.y};
        projectable[i]=StudioGizmoProjectionPolicy::ProbeUsable(unit.depth,pixel);
        if(projectable[i])perUnit[i]={pixel.x-center.x,pixel.y-center.y};
    }
    snapshot.handles=StudioGizmoProjectionPolicy::BuildHandles(
        {center.x,center.y},directions,
        {snapshot.viewportLeft+10.0f,snapshot.viewportTop+10.0f,
         snapshot.viewportRight-10.0f,snapshot.viewportBottom-10.0f});
    // If the selected axis points into the camera, screen-space displacement
    // cannot geometrically resolve depth. Give that axis a bounded depth-drag
    // sensitivity derived from the two visible axes at this SAME camera zoom.
    for(std::size_t i=0;i<3;++i){
        float sum=0;int count=0;
        for(std::size_t j=0;j<3;++j)if(j!=i&&projectable[j]){
            const float density=StudioGizmoMath::Length(perUnit[j]);
            if(std::isfinite(density)&&density>=2.0f){sum+=density;++count;}
        }
        auto& handle=snapshot.handles[i];
        handle.physicalPixelsPerUnit=perUnit[i];
        handle.projectedAxisUsable=projectable[i];
        handle.fallbackPixelsPerUnit=std::clamp(count?sum/count:24.0f,8.0f,160.0f);
        snapshot.visible|=handle.valid;
    }
}
} // namespace
StudioGizmoSnapshot StudioAxisGizmo::Build(const ShipyardBuilderRuntimeModel& model,
                                          const StrategicCamera& camera,int width,int height) {
    StudioGizmoSnapshot out;
    const bool modelMode=model.workspaceMode==ShipyardWorkspaceMode::Model&&!model.modeling.recipe.primitives.empty();
    const bool assemblyMode=model.workspaceMode==ShipyardWorkspaceMode::Build&&!model.recipe.modules.empty();
    if(width<=0||height<=0||(!modelMode&&!assemblyMode)||!model.dcc.showGizmos||model.testWorkspaceActive||
       (assemblyMode&&model.inspectorTab==ShipyardInspectorTab::Sockets)||model.dragPreview.active||model.dragPreview.staged)return out;
    const auto layout=ShipyardBuilderSystem::Layout(model,width,height);
    out.viewportLeft=layout.viewportLeft;out.viewportRight=layout.viewportRight;
    out.viewportTop=layout.viewportTop;out.viewportBottom=layout.viewportBottom;
    if(out.viewportRight-out.viewportLeft<150||out.viewportBottom-out.viewportTop<150)return out;
    if(modelMode){
        const auto& p=model.modeling.recipe.primitives[std::min(model.modeling.selectedPrimitiveIndex,model.modeling.recipe.primitives.size()-1)];
        out.readout.position={{p.position.x,p.position.y,p.position.z}};
        out.readout.rotationDegrees={{p.rotationDegrees.x,p.rotationDegrees.y,p.rotationDegrees.z}};
        out.readout.scalePercent={{100.0f,100.0f,100.0f}};
        out.readout.nominalLocalMeters={{p.size.x,p.size.y,p.size.z}};out.readout.nominalDimensionsAvailable=true;out.readoutVisible=true;
        if(model.transformTool==ShipyardTransformTool::Select)return out;
        const auto center=NativeBattlefieldRenderer::WorldToScreen(p.position,width,height,camera);
        if(!center.visible||center.x<out.viewportLeft+18||center.x>out.viewportRight-18||center.y<out.viewportTop+18||center.y>out.viewportBottom-18)return out;
        // Primitive size X/Y/Z is local pre-rotation geometry. Scale therefore
        // ALWAYS presents local object axes. Move/Rotate retain world axes
        // unless the user explicitly toggled the local constraint.
        ConstructionTransformBasis basis{};
        if(model.transformTool==ShipyardTransformTool::Scale||model.transformConstraintLocal)
            basis=ConstructionTransformBasisSystem::ModelLocal(p.rotationDegrees);
        const float probe=std::clamp(p.size.length()*.12f,0.35f,2.0f);
        PopulateHandles(out,camera,width,height,p.position,basis,probe,center);
        return out;
    }
    const auto& part=model.recipe.modules[std::min(model.selectedPlacedModule,model.recipe.modules.size()-1)];
    const VisualModuleSource* source=nullptr;
    for(const auto& record:model.catalog){
        if(record.source.moduleId==part.moduleId){source=&record.source;break;}
    }
    out.readout=StudioTransformReadout::Build(part,model.recipe,source);
    out.readoutVisible=true;
    if(model.transformTool==ShipyardTransformTool::Select)return out;
    const std::string role=model.recipe.role.empty()?"INDUSTRIAL":model.recipe.role;
    const auto presentation=ForwardSpacePresentationSystem{}.ForShip(role,true,.22f);
    const float sx=.24f*presentation.widthScale*model.recipe.widthScale;
    const float sy=.24f*presentation.lengthScale*model.recipe.lengthScale;
    const float sz=.24f;
    const float yaw=EditorTransformSpaceSystem::RenderedRootYawRadians(0,model.recipe.forwardVisualYawDegrees);
    const float c=std::cos(yaw),s=std::sin(yaw);
    const Vector3 origin{part.x*sx*c-part.y*sy*s,part.x*sx*s+part.y*sy*c,.30f+part.z*sz};
    const auto center=NativeBattlefieldRenderer::WorldToScreen(origin,width,height,camera);
    if(!center.visible||center.x<out.viewportLeft+18||center.x>out.viewportRight-18||
       center.y<out.viewportTop+18||center.y>out.viewportBottom-18)return out;

    const Vector3 rootScale{sx,sy,sz};
    // scaleX/scaleY/scaleZ are module-local dimensions, therefore the Scale
    // gizmo must follow the selected module's actual rotated/mirrored basis.
    // This closes the "drag Y, get width / drag X, get length" presentation
    // mismatch on rotated kitbash pieces without swapping serialized X and Y.
    ConstructionTransformBasis basis=ConstructionTransformBasisSystem::ShipWorld(yaw,rootScale);
    if(model.transformTool==ShipyardTransformTool::Scale||
       model.transformSpace==ShipyardTransformSpace::Local||model.transformConstraintLocal)
        basis=ConstructionTransformBasisSystem::AssemblyWorld(part,yaw,rootScale,true);
    PopulateHandles(out,camera,width,height,origin,basis,3.0f,center);
    return out;
}
} // namespace subspace
