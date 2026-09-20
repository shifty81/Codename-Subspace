#include "studio/StudioAxisGizmo.h"
#include "application/NativeBattlefieldRenderer.h"
#include "editor/EditorTransformSpaceSystem.h"
#include "rendering/ForwardSpacePresentationSystem.h"
#include <algorithm>
#include <cmath>

namespace subspace {
StudioGizmoSnapshot StudioAxisGizmo::Build(const ShipyardBuilderRuntimeModel& model,
                                          const StrategicCamera& camera,int width,int height) {
    StudioGizmoSnapshot out;
    const bool modelMode=model.workspaceMode==ShipyardWorkspaceMode::Model&&!model.modeling.recipe.primitives.empty();
    const bool assemblyMode=model.workspaceMode==ShipyardWorkspaceMode::Build&&!model.recipe.modules.empty();
    if(width<=0||height<=0||(!modelMode&&!assemblyMode)||!model.dcc.showGizmos||model.testWorkspaceActive||
       model.inspectorTab==ShipyardInspectorTab::Sockets||model.dragPreview.active||model.dragPreview.staged)return out;
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
        const Vector3 basis[3]={{1,0,0},{0,1,0},{0,0,1}};
        for(int i=0;i<3;++i){
            const auto projected=NativeBattlefieldRenderer::WorldToScreen(p.position+basis[i]*std::max(1.0f,p.size.length()*.55f),width,height,camera);
            auto& h=out.handles[static_cast<std::size_t>(i)];h.axis=static_cast<StudioAxis>(i);h.center={center.x,center.y};
            if(!projected.visible)continue;const StudioPoint raw{projected.x-center.x,projected.y-center.y};if(StudioGizmoMath::Length(raw)<1.8f)continue;
            const auto dir=StudioGizmoMath::Unit(raw);float length=68.0f;
            const float left=out.viewportLeft+10,right=out.viewportRight-10,top=out.viewportTop+10,bottom=out.viewportBottom-10;
            if(dir.x>.001f)length=std::min(length,(right-center.x)/dir.x);else if(dir.x<-.001f)length=std::min(length,(left-center.x)/dir.x);
            if(dir.y>.001f)length=std::min(length,(bottom-center.y)/dir.y);else if(dir.y<-.001f)length=std::min(length,(top-center.y)/dir.y);
            if(!std::isfinite(length)||length<24)continue;h.tip={center.x+dir.x*length,center.y+dir.y*length};h.valid=true;out.visible=true;
        }
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
    const Vector3 basis[3]={{c,s,0},{-s,c,0},{0,0,1}};
    for(int i=0;i<3;++i){
        const auto projected=NativeBattlefieldRenderer::WorldToScreen(origin+basis[i]*3.0f,width,height,camera);
        auto& h=out.handles[static_cast<std::size_t>(i)];h.axis=static_cast<StudioAxis>(i);
        h.center={center.x,center.y};
        if(!projected.visible)continue;
        const StudioPoint projectedDelta{projected.x-center.x,projected.y-center.y};
        if(StudioGizmoMath::Length(projectedDelta)<1.8f)continue;
        const auto dir=StudioGizmoMath::Unit(projectedDelta);
        float length=68.0f;const float left=out.viewportLeft+10,right=out.viewportRight-10,top=out.viewportTop+10,bottom=out.viewportBottom-10;
        if(dir.x>.001f)length=std::min(length,(right-center.x)/dir.x);else if(dir.x<-.001f)length=std::min(length,(left-center.x)/dir.x);
        if(dir.y>.001f)length=std::min(length,(bottom-center.y)/dir.y);else if(dir.y<-.001f)length=std::min(length,(top-center.y)/dir.y);
        if(!std::isfinite(length)||length<24.0f)continue;
        h.tip={center.x+dir.x*length,center.y+dir.y*length};
        h.valid=h.tip.x>=out.viewportLeft+10&&h.tip.x<=out.viewportRight-10&&
                h.tip.y>=out.viewportTop+10&&h.tip.y<=out.viewportBottom-10;
        out.visible=out.visible||h.valid;
    }
    return out;
}
}
