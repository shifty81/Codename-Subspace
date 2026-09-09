#include "ship_editor/ShipyardTransformSystem.h"
#include <algorithm>
#include <cmath>
namespace subspace {
namespace {float Snap(float v,float step){return step>0?std::round(v/step)*step:v;}}
bool ShipyardTransformSystem::Begin(ShipyardTransformTransaction& tx,std::size_t i,const VisualModulePlacement& p,ShipyardTransformTool tool,ShipyardTransformSpace space){tx={};tx.active=true;tx.moduleIndex=i;tx.tool=tool;tx.space=space;tx.before=p;tx.working=p;return true;}
void ShipyardTransformSystem::Translate(ShipyardTransformTransaction& tx,const Vector3& d,bool fine){if(!tx.active)return;const float scale=fine?.10f:1.0f;tx.working.x+=d.x*scale;tx.working.y+=d.y*scale;tx.working.z+=d.z*scale;if(tx.snap&&!fine){tx.working.x=Snap(tx.working.x,tx.translationSnap);tx.working.y=Snap(tx.working.y,tx.translationSnap);tx.working.z=Snap(tx.working.z,tx.translationSnap);}}
void ShipyardTransformSystem::Rotate(ShipyardTransformTransaction& tx,const Vector3& d,bool fine){if(!tx.active)return;const float scale=fine?.10f:1.0f;tx.working.pitchDegrees+=d.x*scale;tx.working.yawDegrees+=d.y*scale;tx.working.rollDegrees+=d.z*scale;if(tx.snap&&!fine){tx.working.pitchDegrees=Snap(tx.working.pitchDegrees,tx.rotationSnapDegrees);tx.working.yawDegrees=Snap(tx.working.yawDegrees,tx.rotationSnapDegrees);tx.working.rollDegrees=Snap(tx.working.rollDegrees,tx.rotationSnapDegrees);}}

void ShipyardTransformSystem::Scale(ShipyardTransformTransaction& tx,const Vector3& d,bool fine){
    if(!tx.active)return;
    const float precision=fine?.10f:1.0f;
    tx.working.scaleX=std::clamp(tx.working.scaleX+d.x*precision,.10f,4.0f);
    tx.working.scaleY=std::clamp(tx.working.scaleY+d.y*precision,.10f,4.0f);
    tx.working.scaleZ=std::clamp(tx.working.scaleZ+d.z*precision,.10f,4.0f);
    if(tx.snap&&!fine){
        tx.working.scaleX=std::clamp(Snap(tx.working.scaleX,tx.scaleSnap),.10f,4.0f);
        tx.working.scaleY=std::clamp(Snap(tx.working.scaleY,tx.scaleSnap),.10f,4.0f);
        tx.working.scaleZ=std::clamp(Snap(tx.working.scaleZ,tx.scaleSnap),.10f,4.0f);
    }
}
VisualModulePlacement ShipyardTransformSystem::Commit(ShipyardTransformTransaction& tx){auto p=tx.working;tx.active=false;return p;}
VisualModulePlacement ShipyardTransformSystem::Cancel(ShipyardTransformTransaction& tx){auto p=tx.before;tx.active=false;return p;}
} // namespace subspace
