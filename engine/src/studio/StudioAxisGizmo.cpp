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
    if(width<=0||height<=0||model.recipe.modules.empty()||!model.dcc.showGizmos||
       model.workspaceMode!=ShipyardWorkspaceMode::Build||model.testWorkspaceActive||
       model.inspectorTab==ShipyardInspectorTab::Sockets||
       model.dragPreview.active||model.dragPreview.staged)return out;
    const auto layout=ShipyardBuilderSystem::Layout(model,width,height);
    out.viewportLeft=layout.viewportLeft;out.viewportRight=layout.viewportRight;
    out.viewportTop=layout.viewportTop;out.viewportBottom=layout.viewportBottom;
    if(out.viewportRight-out.viewportLeft<150||out.viewportBottom-out.viewportTop<150)return out;
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
    if(!center.visible||center.x<out.viewportLeft+78||center.x>out.viewportRight-78||
       center.y<out.viewportTop+78||center.y>out.viewportBottom-78)return out;
    const Vector3 basis[3]={{c,s,0},{-s,c,0},{0,0,1}};
    for(int i=0;i<3;++i){
        const auto projected=NativeBattlefieldRenderer::WorldToScreen(origin+basis[i]*3.0f,width,height,camera);
        auto& h=out.handles[static_cast<std::size_t>(i)];h.axis=static_cast<StudioAxis>(i);
        h.center={center.x,center.y};
        if(!projected.visible)continue;
        const auto dir=StudioGizmoMath::Unit({projected.x-center.x,projected.y-center.y});
        if(StudioGizmoMath::Length(dir)<.8f)continue;
        h.tip={center.x+dir.x*68.0f,center.y+dir.y*68.0f};
        h.valid=h.tip.x>=out.viewportLeft+10&&h.tip.x<=out.viewportRight-10&&
                h.tip.y>=out.viewportTop+10&&h.tip.y<=out.viewportBottom-10;
        out.visible=out.visible||h.valid;
    }
    return out;
}
}
