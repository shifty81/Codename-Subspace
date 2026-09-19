#include "studio/StudioTransformReadout.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
using namespace subspace;
namespace {
int count=0;
void Check(bool condition,const char* label){++count;if(!condition){std::cerr<<"FAIL "<<label<<'\n';std::exit(1);}}
bool Equal(float x,float y){return std::fabs(x-y)<0.001f;}
}
int main(){
    ProceduralShipVisualRecipe recipe{};recipe.widthScale=1.5f;recipe.lengthScale=.75f;
    VisualModulePlacement part{};part.moduleId="test_hull";part.x=-3;part.y=8;part.z=2;
    part.pitchDegrees=12;part.rollDegrees=30;part.yawDegrees=-45;
    part.scaleX=2;part.scaleY=.5f;part.scaleZ=1.25f;
    VisualModuleSource source{};source.moduleId="test_hull";
    source.halfWidth=2;source.halfLength=4;source.halfHeight=1;
    const auto r=StudioTransformReadout::Build(part,recipe,&source);
    Check(Equal(r.position[0],-3),"assembly X value preserved");
    Check(Equal(r.position[1],8),"assembly Y value preserved");
    Check(Equal(r.position[2],2),"assembly Z value preserved");
    Check(Equal(r.rotationDegrees[0],12),"X reads pitch");
    Check(Equal(r.rotationDegrees[1],30),"Y reads roll, not yaw");
    Check(Equal(r.rotationDegrees[2],-45),"Z reads yaw, not roll");
    Check(Equal(r.scalePercent[0],200),"X scale in percent");
    Check(Equal(r.scalePercent[1],50),"Y scale in percent");
    Check(Equal(r.scalePercent[2],125),"Z scale in percent");
    Check(r.nominalDimensionsAvailable,"source nominal extents available");
    Check(Equal(r.nominalLocalMeters[0],12),"nominal width includes recipe X scale");
    Check(Equal(r.nominalLocalMeters[1],3),"nominal length includes recipe Y scale");
    Check(Equal(r.nominalLocalMeters[2],2.5f),"nominal height includes part Z scale");
    const auto missing=StudioTransformReadout::Build(part,recipe,nullptr);
    Check(!missing.nominalDimensionsAvailable,"missing source does not fabricate dimensions");
    source.halfWidth=std::numeric_limits<float>::quiet_NaN();
    Check(!StudioTransformReadout::Build(part,recipe,&source).nominalDimensionsAvailable,
          "nonfinite geometry cannot yield dimensions");
    source.halfWidth=2;part.scaleX=-2;
    const auto mirrored=StudioTransformReadout::Build(part,recipe,&source);
    Check(mirrored.nominalDimensionsAvailable && Equal(mirrored.nominalLocalMeters[0],12),
          "mirroring preserves positive physical dimension");
    part.yawDegrees=1090;
    Check(Equal(StudioTransformReadout::Build(part,recipe,&source).rotationDegrees[2],1090),
          "readout presents stored angle, does not silently wrap authority");
    std::cout<<"Studio transform readout: "<<count<<"/"<<count<<" PASS\n";
}
