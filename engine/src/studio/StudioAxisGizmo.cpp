#include "studio/StudioAxisGizmo.h"
#include "studio/StudioGizmoProjectionPolicy.h"
#include "application/NativeBattlefieldRenderer.h"
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
                     const std::array<Vector3,3>& basis,float probeMeters,
                     const StrategicScreenPoint& center){
    std::array<StudioPoint,3> directions{};
    for(std::size_t i=0;i<directions.size();++i){
        const auto probe=NativeBattlefieldRenderer::WorldToScreen(
            origin+basis[i]*probeMeters,width,height,camera);
        const StudioPoint point{probe.x,probe.y};
        // `visible` means the whole probe is inside the camera frustum, which
        // is NOT required for a 68-pixel on-screen handle. Accept finite
        // projected coordinates even when the long probe is off-screen.
        const bool usable=StudioGizmoProjectionPolicy::ProbeUsable(probe.depth,point);
        const StudioPoint delta=usable?StudioPoint{point.x-center.x,point.y-center.y}
                                      :StudioPoint{std::numeric_limits<float>::quiet_NaN(),0};
        directions[i]=StudioGizmoProjectionPolicy::Direction(static_cast<StudioAxis>(i),delta);
    }
    snapshot.handles=StudioGizmoProjectionPolicy::BuildHandles(
        {center.x,center.y},directions,
        {snapshot.viewportLeft+10.0f,snapshot.viewportTop+10.0f,
         snapshot.viewportRight-10.0f,snapshot.viewportBottom-10.0f});
    for(const auto& handle:snapshot.handles)snapshot.visible|=handle.valid;
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
        const std::array<Vector3,3> basis{{{1,0,0},{0,1,0},{0,0,1}}};
        // A large box must not require an endpoint inside the frustum; only
        // direction matters. Keep the probe local to the pivot for stability.
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
    const std::array<Vector3,3> basis{{{c,s,0},{-s,c,0},{0,0,1}}};
    PopulateHandles(out,camera,width,height,origin,basis,3.0f,center);
    return out;
}
} // namespace subspace
