#include "studio/StudioMeasurementFormat.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>

using subspace::StudioMeasurementFormat;
using subspace::StudioMeasurementKind;

namespace {
int checks=0;
void Check(bool condition,const char* label){
    ++checks;
    if(!condition){std::cerr<<"FAIL: "<<label<<'\n';std::exit(1);}
}
}
int main(){
    using K=StudioMeasurementKind;
    Check(std::string(StudioMeasurementFormat::Label(K::Position))=="POS","position label");
    Check(std::string(StudioMeasurementFormat::Label(K::Rotation))=="ROT","rotation label");
    Check(std::string(StudioMeasurementFormat::Label(K::Scale))=="SCL","scale label");
    Check(std::string(StudioMeasurementFormat::Label(K::NominalDimensions))=="DIM","dimension label");
    Check(std::string(StudioMeasurementFormat::AxisLabel(K::Position,2))=="Z","position Z");
    Check(std::string(StudioMeasurementFormat::AxisLabel(K::Rotation,1))=="Y","physical rotation Y");
    Check(std::string(StudioMeasurementFormat::AxisLabel(K::NominalDimensions,0))=="W","width is W");
    Check(std::string(StudioMeasurementFormat::AxisLabel(K::NominalDimensions,1))=="L","length is L");
    Check(std::string(StudioMeasurementFormat::AxisLabel(K::NominalDimensions,2))=="H","height is H");
    Check(std::string(StudioMeasurementFormat::AxisLabel(K::NominalDimensions,3))=="?","invalid axis rejected");
    Check(StudioMeasurementFormat::IsMeters(K::Position),"position in meters");
    Check(StudioMeasurementFormat::IsMeters(K::NominalDimensions),"dimensions in meters");
    Check(!StudioMeasurementFormat::IsMeters(K::Rotation),"rotation not meters");
    Check(StudioMeasurementFormat::IsDegrees(K::Rotation),"degrees only rotation");
    Check(StudioMeasurementFormat::IsPercent(K::Scale),"percent only scale");
    Check(StudioMeasurementFormat::Compact(2.5f,2)=="2.50","decimal display");
    Check(StudioMeasurementFormat::Compact(-0.0001f,2)=="0.00","no negative zero");
    Check(StudioMeasurementFormat::Compact(-45.0f,1)=="-45.0","negative angle");
    Check(StudioMeasurementFormat::Compact(std::numeric_limits<float>::infinity(),2)=="--","nonfinite not displayed as number");
    Check(StudioMeasurementFormat::Compact(std::numeric_limits<float>::quiet_NaN(),2)=="--","nan not displayed as number");
    Check(StudioMeasurementFormat::Compact(10000.0f,2)=="--","oversize cannot lie as 9999");
    Check(StudioMeasurementFormat::Compact(0.0f,-1)=="--","invalid precision rejected");
    std::cout<<"S15B measurement contract: "<<checks<<"/"<<checks<<" PASS\n";
}
