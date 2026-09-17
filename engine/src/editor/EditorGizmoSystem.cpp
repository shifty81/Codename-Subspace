#include "editor/EditorGizmoSystem.h"
#include "rendering/ForwardSpacePresentationSystem.h"
#include "rendering/StrategicViewProjection.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace subspace {
namespace {
constexpr float kPi=3.14159265358979323846f;
Vector3 RotateZ(const Vector3& v,float radians){
    const float c=std::cos(radians),s=std::sin(radians);
    return {v.x*c-v.y*s,v.x*s+v.y*c,v.z};
}
float Dot2(float x,float y,float u,float v){return x*u+y*v;}
}
EditorScreenDirection EditorGizmoSystem::ProjectPlanarDirection(const Vector3&d,float deg,float v){
    float y=deg*kPi/180,c=std::cos(y),s=std::sin(y);
    float x=d.x*c+d.y*s,sy=-(d.x*(-s)+d.y*c)*v,l=std::sqrt(x*x+sy*sy);
    if(l<1e-5f)return{0,-1,false};return{x/l,sy/l,true};
}
EditorGizmoLayout EditorGizmoSystem::BuildModule(const VisualModulePlacement& module,
    const ProceduralShipVisualRecipe& recipe,const StrategicCamera& camera,
    float w,float h,const Vector3& shipWorld,float shipYaw,
    ShipyardTransformSpace space,ShipyardTransformTool tool){
    EditorGizmoLayout out;
    if(w<1||h<1||tool==ShipyardTransformTool::Select)return out;
    const std::string role=recipe.role.empty()?"INDUSTRIAL":recipe.role;
    const auto presentation=ForwardSpacePresentationSystem{}.ForShip(role,true,.22f);
    const Vector3 scales{.24f*presentation.widthScale*recipe.widthScale,
                         .24f*presentation.lengthScale*recipe.lengthScale,.24f};
    const float rootYaw=shipYaw+recipe.forwardVisualYawDegrees*kPi/180.0f;
    const Vector3 origin=shipWorld+RotateZ({module.x*scales.x,module.y*scales.y,module.z*scales.z},rootYaw);
    const auto pivot=StrategicViewProjection::WorldToScreen(origin,w,h,camera);
    if(!pivot.visible)return out;
    const Vector3 axes[3]={{1,0,0},{0,1,0},{0,0,1}};
    for(int i=0;i<3;++i){
        Vector3 assembly=axes[i];
        // Model scale and Euler rotation properties address their own XYZ components.
        // Translation in Local follows the selected module's yaw; View uses the camera axes.
        if(tool==ShipyardTransformTool::Move){
            if(space==ShipyardTransformSpace::Local)assembly=RotateZ(assembly,module.yawDegrees*kPi/180.0f);
            else if(space==ShipyardTransformSpace::View&&i<2){
                const auto basis=StrategicViewProjection::Build(camera,w,h);
                const Vector3 view=i==0?basis.right:basis.up;
                assembly=RotateZ(view,-rootYaw);
                // Keep view axes planar: Z remains the actual up/down axis.
                assembly.z=0.0f;
                const float length=std::sqrt(assembly.x*assembly.x+assembly.y*assembly.y);
                if(length>1.0e-5f)assembly=assembly*(1.0f/length);
                else assembly=axes[i];
            }
        }
        const Vector3 worldAxis=RotateZ({assembly.x*scales.x,assembly.y*scales.y,assembly.z*scales.z},rootYaw);
        const auto projected=StrategicViewProjection::WorldToScreen(origin+worldAxis,w,h,camera);
        const float px=projected.x-pivot.x,py=projected.y-pivot.y;
        const float length=std::sqrt(px*px+py*py);
        EditorGizmoHandle handle;handle.axis=static_cast<EditorGizmoAxis>(i);
        handle.assemblyDirection=assembly;handle.centerX=pivot.x;handle.centerY=pivot.y;
        // Suppress degenerate / camera-aligned handles; never generate a fake hit region.
        handle.valid=projected.depth>0.0f&&std::isfinite(length)&&length>1.8f;
        if(handle.valid){handle.tipX=pivot.x+px/length*46.0f;handle.tipY=pivot.y+py/length*46.0f;out.visible=true;}
        out.handles[static_cast<std::size_t>(i)]=handle;
    }
    return out;
}
const EditorGizmoHandle* EditorGizmoSystem::Handle(const EditorGizmoLayout& layout,EditorGizmoAxis axis){
    const int index=static_cast<int>(axis);
    return index>=0&&index<3&&layout.handles[static_cast<std::size_t>(index)].valid?
           &layout.handles[static_cast<std::size_t>(index)]:nullptr;
}
EditorGizmoAxis EditorGizmoSystem::Pick(const EditorGizmoLayout& layout,float x,float y,float threshold){
    if(!layout.visible)return EditorGizmoAxis::None;
    float best=std::max(1.0f,threshold)*std::max(1.0f,threshold);
    EditorGizmoAxis picked=EditorGizmoAxis::None;
    for(const auto& a:layout.handles){
        if(!a.valid)continue;
        const float vx=a.tipX-a.centerX,vy=a.tipY-a.centerY;
        const float len2=vx*vx+vy*vy;
        if(len2<1.0f)continue;
        const float t=std::clamp(Dot2(x-a.centerX,y-a.centerY,vx,vy)/len2,0.20f,1.13f);
        const float nearX=a.centerX+vx*t,nearY=a.centerY+vy*t;
        const float ex=x-nearX,ey=y-nearY;
        const float d2=ex*ex+ey*ey;
        // Ignore the shared center: three overlapping handles there are ambiguous.
        if(Dot2(x-a.centerX,y-a.centerY,vx,vy)/len2<.20f)continue;
        if(d2<best){best=d2;picked=a.axis;}
    }
    return picked;
}
float EditorGizmoSystem::DragPixels(const EditorGizmoHandle& a,float dx,float dy){
    if(!a.valid)return 0.0f;
    const float vx=a.tipX-a.centerX,vy=a.tipY-a.centerY;
    const float len=std::sqrt(vx*vx+vy*vy);
    return len>1.0e-5f?(dx*vx+dy*vy)/len:0.0f;
}
} // namespace subspace
