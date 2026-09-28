#include "studio/StudioTransformViewBasis.h"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace subspace;
namespace { bool near(float a,float b,float e=.002f){return std::fabs(a-b)<e;} float dot(Vector3 a,Vector3 b){return a.x*b.x+a.y*b.y+a.z*b.z;} }
int main(){
    StrategicCamera camera;
    camera.SetEditorView({0,-10,5},{0,0,0},0);
    auto b=StudioTransformViewBasis::Build(camera);
    assert(near(b.x.length(),1)&&near(b.y.length(),1)&&near(b.z.length(),1));
    assert(std::fabs(dot(b.x,b.y))<.002f&&std::fabs(dot(b.x,b.z))<.002f&&std::fabs(dot(b.y,b.z))<.002f);
    // Orbiting the camera changes VIEW basis, while the PARENT basis remains an external fixed authority.
    camera.SetEditorView({10,0,5},{0,0,0},0);
    auto c=StudioTransformViewBasis::Build(camera);
    assert(std::fabs(dot(b.x,c.x))<.2f);
    camera.SetEditorView({10,0,5},{0,0,0},90);
    auto rolled=StudioTransformViewBasis::Build(camera);
    assert(std::fabs(dot(c.x,rolled.y))>.98f);
    std::cout<<"R48 real camera VIEW basis: PASS\n";
}
