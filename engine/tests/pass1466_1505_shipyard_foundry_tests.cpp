#include "content/ShipyardMaterialAuditSystem.h"
#include "editor/ConstructionEditorCameraSystem.h"
#include "interior/ShipInteriorStructureAuthoringSystem.h"
#include "ships/ShipArticulationSystem.h"
#include "ui/SubspaceUiFramework.h"

#include <cmath>
#include <iostream>

using namespace subspace;
namespace { int failures=0; void Check(bool v,const char* m){if(!v){std::cerr<<"FAIL: "<<m<<"\n";++failures;}} }

int main(){
    {
        ConstructionEditorCameraState c;ConstructionEditorCameraSystem::Reset(c,{0,0,0},8);
        const Vector3 before=c.eye-c.assemblyCenter;const float yaw=c.yawDegrees,pitch=c.pitchDegrees,dist=c.orbitDistance;
        ConstructionEditorCameraSystem::TruckPedestal(c,2.0f,-1.0f);
        const Vector3 after=c.eye-c.assemblyCenter;
        Check((after-before).length()<1e-4f,"MMB pan preserves eye-target ray");
        Check(std::fabs(c.yawDegrees-yaw)<1e-5f&&std::fabs(c.pitchDegrees-pitch)<1e-5f,"MMB pan preserves orbit angles");
        Check(std::fabs(c.orbitDistance-dist)<1e-5f,"MMB pan preserves orbit distance");
        ConstructionEditorCameraSystem::Orbit(c,8.0f,-4.0f);
        Check(std::fabs(c.yawDegrees-(yaw+8.0f))<1e-4f,"orbit continues from panned orientation without snap");
    }
    {
        ShipyardObjCertification m;m.sourceName="antenna.obj";m.valid=true;m.hasNormals=true;m.hasTexcoords=true;m.materialLibraries={"antenna.mtl"};
        auto r=ShipyardMaterialAuditSystem::Audit(m);
        Check(r.certification==KitbashMaterialCertification::BrokenDependency,"referenced MTL with no resolved materials is broken dependency");
        Check(r.semanticFallbackRecommended,"broken material recommends semantic fallback");
        ShipyardCertifiedMaterial mat;mat.name="sensor_metal";mat.resolvedFromMtl=true;mat.hasBaseColorTexture=false;mat.hasNormalTexture=true;mat.hasMetallicTexture=true;mat.hasRoughnessTexture=true;m.materials={mat};
        r=ShipyardMaterialAuditSystem::Audit(m);
        Check(r.certification==KitbashMaterialCertification::NormalizedFallback,"resolved untextured material uses normalized fallback");
    }
    {
        ShipInteriorStructuralModel model;auto i=ShipInteriorStructureAuthoringSystem::Add(model,ShipInteriorElementKind::Airlock,0,0);(void)i;
        Check(!model.elements.empty()&&model.elements[0].pressureBoundary&&model.elements[0].openable,"authored airlock is openable pressure boundary");
        Check(ShipInteriorStructureAuthoringSystem::AssignMaterial(model,"INTERIOR_BLACK_CHROME"),"airlock material can be reassigned");
        Check(model.elements[0].materialId=="INTERIOR_BLACK_CHROME","airlock material assignment persists");
    }
    {
        SubspaceDockWorkspace w=SubspaceDockSystem::CreateMinimalWorkspace("shipyard");SubspaceDockPanel p;p.id="properties";p.title="Properties";p.defaultLeafId="right";
        Check(SubspaceDockSystem::RegisterPanel(w,p),"dock panel registers");
        Check(SubspaceDockSystem::FloatPanel(w,"properties",{40,60,360,280}),"panel can float");
        Check(SubspaceDockSystem::TogglePinned(w,"properties"),"floating panel can pin");
        Check(SubspaceDockSystem::SetAutoHide(w,"properties",true),"panel supports auto-hide");
        std::string err;Check(SubspaceDockSystem::Validate(w,&err),"dock workspace remains valid after float/pin/autohide");
        const auto serialized=SubspaceDockSystem::Serialize(w);SubspaceDockWorkspace restored;
        Check(SubspaceDockSystem::Deserialize(serialized,restored,&err),"dock workspace layout round-trips");
        const auto* restoredPanel=SubspaceDockSystem::FindPanel(restored,"properties");
        Check(restoredPanel&&restoredPanel->pinned&&restoredPanel->autoHide,"panel pin/autohide persists in layout");
    }
    return failures==0?0:1;
}
