#include "studio/StudioTransformMoveDelta.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace subspace;
namespace { float dot(Vector3 a,Vector3 b){return a.x*b.x+a.y*b.y+a.z*b.z;} bool near(float a,float b){return std::fabs(a-b)<.003f;} }
int main(){
    ShipyardBuilderRuntimeModel model{};model.recipe.role="INDUSTRIAL";model.recipe.widthScale=1.0f;model.recipe.lengthScale=1.0f;model.recipe.forwardVisualYawDegrees=37.0f;
    VisualModulePlacement p{};p.scaleX=p.scaleY=p.scaleZ=1;p.yawDegrees=90.0f;model.recipe.modules.push_back(p);
    StrategicCamera camera;camera.SetEditorView({8,-11,6},{0,0,0},0);

    model.transformSpace=ShipyardTransformSpace::Ship;
    auto parent=StudioTransformMoveDelta::AssemblyAuthored(model,camera,StudioAxis::X,2.0f);
    assert(near(parent.x,2)&&near(parent.y,0)&&near(parent.z,0));

    model.transformSpace=ShipyardTransformSpace::Local;
    auto object=StudioTransformMoveDelta::AssemblyAuthored(model,camera,StudioAxis::X,2.0f);
    assert(near(object.x,0)&&near(object.y,2)&&near(object.z,0));

    model.transformSpace=ShipyardTransformSpace::View;
    const auto view=StudioTransformViewBasis::Build(camera);
    const auto authored=StudioTransformMoveDelta::AssemblyAuthored(model,camera,StudioAxis::X,2.0f);
    const auto presentation=ForwardSpacePresentationSystem{}.ForShip("INDUSTRIAL",true,.22f);
    const Vector3 scale{.24f*presentation.widthScale,.24f*presentation.lengthScale,.24f};
    const float yaw=EditorTransformSpaceSystem::RenderedRootYawRadians(0,37.0f);
    auto world=ConstructionTransformBasisSystem::RotateZ({authored.x*scale.x,authored.y*scale.y,authored.z*scale.z},yaw);
    assert(near(world.length(),2.0f));
    assert(dot(world.normalized(),view.x)>.999f);
    std::cout<<"R48B PARENT/OBJECT/VIEW movement delta parity: PASS\n";
}
