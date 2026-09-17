#include "editor/EditorGizmoSystem.h"
#include "rendering/StrategicViewProjection.h"
#include "ship_editor/ShipyardTransformSystem.h"
#include <cmath>
#include <iostream>
using namespace subspace;
namespace {
int failures=0,assertions=0;
void Check(bool ok,const char* message){++assertions;if(!ok){++failures;std::cerr<<"FAIL: "<<message<<"\n";}}
bool Near(float a,float b,float eps=.0001f){return std::fabs(a-b)<eps;}
}
int main(){
    StrategicCamera camera;camera.SetEditorView({10,-20,12},{0,0,0});
    ProceduralShipVisualRecipe recipe;recipe.role="INDUSTRIAL";recipe.widthScale=recipe.lengthScale=1;
    VisualModulePlacement p;p.moduleId="gizmo_test";p.scaleX=p.scaleY=p.scaleZ=1;
    const auto layout=EditorGizmoSystem::BuildModule(p,recipe,camera,1280,768,{0,0,.30f},0,
        ShipyardTransformSpace::Ship,ShipyardTransformTool::Move);
    Check(layout.visible,"selected module has projected gizmo");
    for(int i=0;i<3;++i){
        const auto axis=static_cast<EditorGizmoAxis>(i);
        const auto* h=EditorGizmoSystem::Handle(layout,axis);
        Check(h&&h->valid,"axis has valid projected handle");
        if(!h)continue;
        const float x=h->centerX+(h->tipX-h->centerX)*.75f;
        const float y=h->centerY+(h->tipY-h->centerY)*.75f;
        Check(EditorGizmoSystem::Pick(layout,x,y)==axis,"rendered handle picks same axis");
        const float ux=(h->tipX-h->centerX)/46.0f,uy=(h->tipY-h->centerY)/46.0f;
        Check(EditorGizmoSystem::DragPixels(*h,ux*12,uy*12)>11.8f,"drag projects along the selected axis");
        Check(Near(EditorGizmoSystem::DragPixels(*h,-uy*12,ux*12),0),"perpendicular pointer motion does not move handle");
        ShipyardTransformTransaction tx;ShipyardTransformSystem::Begin(tx,0,p,ShipyardTransformTool::Move);
        ShipyardTransformSystem::Translate(tx,h->assemblyDirection*.25f,false);
        Check(Near(tx.working.x,i==0?.30f:0)&&Near(tx.working.y,i==1?.30f:0)&&Near(tx.working.z,i==2?.30f:0),
            "axis-constrained move preserves other coordinates with translation snapping");
    }
    Check(EditorGizmoSystem::Pick(layout,layout.handles[0].centerX,layout.handles[0].centerY)==EditorGizmoAxis::None,
          "ambiguous shared pivot does not steal selection clicks");
    const auto select=EditorGizmoSystem::BuildModule(p,recipe,camera,1280,768,{0,0,.30f},0,
        ShipyardTransformSpace::Ship,ShipyardTransformTool::Select);
    Check(!select.visible&&EditorGizmoSystem::Pick(select,640,384)==EditorGizmoAxis::None,"select tool draws no draggable handles");
    const auto resized=EditorGizmoSystem::BuildModule(p,recipe,camera,1920,1080,{0,0,.30f},.5f,
        ShipyardTransformSpace::Ship,ShipyardTransformTool::Scale);
    Check(resized.visible,"perspective gizmo remains visible after resize and ship yaw");
    for(int i=0;i<3;++i){
        const auto* h=EditorGizmoSystem::Handle(resized,static_cast<EditorGizmoAxis>(i));
        if(!h)continue;
        const float mx=(h->centerX+h->tipX)*.5f,my=(h->centerY+h->tipY)*.5f;
        Check(EditorGizmoSystem::Pick(resized,mx,my)==h->axis,"resized scale handle pick matches drawing");
    }
    ShipyardTransformTransaction scale;
    ShipyardTransformSystem::Begin(scale,0,p,ShipyardTransformTool::Scale);
    ShipyardTransformSystem::Scale(scale,{.2f,0,0});
    Check(Near(scale.working.scaleX,1.2f)&&Near(scale.working.scaleY,1)&&Near(scale.working.scaleZ,1),
          "per-axis scale changes only the selected size property");
    ShipyardTransformTransaction rotate;
    ShipyardTransformSystem::Begin(rotate,0,p,ShipyardTransformTool::Rotate);
    ShipyardTransformSystem::Rotate(rotate,{0,0,30});
    Check(Near(rotate.working.pitchDegrees,0)&&Near(rotate.working.yawDegrees,0)&&Near(rotate.working.rollDegrees,30),
          "Z-axis rotation does not modify pitch or yaw");
    std::cout<<"Unified gizmo G1 assertions: "<<assertions-failures<<" / "<<assertions<<" passed\n";
    return failures?1:0;
}
