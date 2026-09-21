#include "studio/StudioGizmoProjectionPolicy.h"
#include "studio/StudioGizmoDragPolicy.h"
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
using namespace subspace;
static bool close(float a,float b){return std::fabs(a-b)<.0001f;}
static constexpr StudioGizmoProjectionPolicy::Bounds bounds{10,10,640,390};
static void check(const std::array<StudioAxisHandle,3>& h){
    for(std::size_t i=0;i<3;++i){
        assert(h[i].valid && h[i].axis==static_cast<StudioAxis>(i));
        assert(h[i].tip.x>=bounds.left-.001f&&h[i].tip.x<=bounds.right+.001f);
        assert(h[i].tip.y>=bounds.top-.001f&&h[i].tip.y<=bounds.bottom+.001f);
        assert(StudioGizmoMath::Length(StudioGizmoMath::Delta(h[i].tip,h[i].center))>=43.99f);
        assert(StudioGizmoMath::Hit(h[i],h[i].tip.x,h[i].tip.y));
        for(std::size_t j=0;j<i;++j){
            assert(StudioGizmoMath::Length(StudioGizmoMath::Delta(h[i].tip,h[j].tip))>=43.99f);
            const auto a=StudioGizmoMath::Unit(StudioGizmoMath::Delta(h[i].tip,h[i].center));
            const auto b=StudioGizmoMath::Unit(StudioGizmoMath::Delta(h[j].tip,h[j].center));
            assert(StudioGizmoMath::Dot(a,b)<=StudioGizmoProjectionPolicy::kMaximumDirectionDot+.001f);
            assert(!StudioGizmoMath::Hit(h[j],h[i].tip.x,h[i].tip.y));
        }
    }
}
int main(){
    using P=StudioGizmoProjectionPolicy;
    assert(P::UsesScreenSpaceFallback({0,0}));
    assert(P::UsesScreenSpaceFallback({1,0}));
    assert(!P::UsesScreenSpaceFallback({20,0}));
    const auto x=P::Direction(StudioAxis::X,{0,0});
    const auto y=P::Direction(StudioAxis::Y,{0,0});
    const auto z=P::Direction(StudioAxis::Z,{0,0});
    assert(close(x.x,1)&&close(x.y,0));
    assert(y.x<0&&y.y<0);
    assert(close(z.x,0)&&close(z.y,1));
    assert(!close(x.x,y.x)&&!close(y.x,z.x));
    const auto normal=P::Direction(StudioAxis::Z,{3,4});
    assert(close(normal.x,.6f)&&close(normal.y,.8f));
    assert(P::UsesScreenSpaceFallback({NAN,0}));
    // Prior bug: a valid offscreen projection was dropped by `if(!probe.visible)continue`.
    assert(P::ProbeUsable(16.0f,{100000.0f,-50000.0f}));
    assert(!P::ProbeUsable(0.02f,{10.0f,20.0f}));
    assert(!P::ProbeUsable(16.0f,{std::numeric_limits<float>::infinity(),20.0f}));
    // Camera-on-axis: a physical axis with zero projection still has its own marker.
    check(P::BuildHandles({320,190},{{x,y,z}},bounds));
    // Previously R3 left parallel nonzero projections coincident and unpickable.
    check(P::BuildHandles({320,190},{{{1,0},{1,0},{1,0}}},bounds));
    // Looking along X/Y may leave Z and Y at almost the same screen bearing.
    check(P::BuildHandles({320,190},{{{0,-1},{.035f,-.999f},{0,-1}}},bounds));
    // Near the viewport border, reverse/bend handles instead of dropping an axis.
    check(P::BuildHandles({628,378},{{{1,0},{0,1},{1,1}}},bounds));
    check(P::BuildHandles({22,22},{{{-1,0},{0,-1},{-1,-1}}},bounds));
    // Deterministic camera sweep: prove every axis still has an independent
    // hit target across coincident, nearly parallel and edge-clipped bearings.
    const std::array<StudioPoint,5> centers{{{320,190},{22,22},{628,378},{628,22},{22,378}}};
    int samples=0;
    for(const auto center:centers)for(int a=0;a<360;a+=7)for(int d=0;d<360;d+=19){
        constexpr float radians=.017453292519943295f;
        const std::array<StudioPoint,3> dirs{{
            {std::cos(a*radians),std::sin(a*radians)},
            {std::cos((a+d)*radians),std::sin((a+d)*radians)},
            {std::cos((a+2*d)*radians),std::sin((a+2*d)*radians)}}};
        check(P::BuildHandles(center,dirs,bounds));++samples;
    }
    assert(samples==4940);
    // Perspective calibration in authored units, independent of fixed HUD pixels.
    StudioAxisHandle model{StudioAxis::X,{200,150},{268,150},true};
    model.physicalPixelsPerUnit={100,0};model.projectedAxisUsable=true;
    model.fallbackPixelsPerUnit=24;
    assert(close(StudioGizmoDragPolicy::MoveUnits(model,{100,0}),1));
    assert(close(StudioGizmoDragPolicy::MoveUnits(model,{-50,20}),-.5f));
    assert(close(StudioGizmoDragPolicy::MoveUnits(model,{100,0},true),.1f));
    // Assembly world scale is ~.24m/unit but projection supplies the correct
    // authored sensitivity: 24 physical pixels => ONE assembly unit.
    StudioAxisHandle assembly=model;
    assembly.physicalPixelsPerUnit={0,-24};
    assert(close(StudioGizmoDragPolicy::MoveUnits(assembly,{0,-48}),2));
    assert(close(StudioGizmoDragPolicy::MoveUnits(assembly,{0,48}),-2));
    // A floating panel may rotate ONLY the drawn handle; real movement must
    // still follow the physical projected direction, with no false X motion.
    assembly.tip={240,190};
    assert(close(StudioGizmoDragPolicy::MoveUnits(assembly,{0,-24}),1));
    assert(close(StudioGizmoDragPolicy::MoveUnits(assembly,{24,0}),0));
    // Camera-facing axis: deterministic bounded manual depth mode.
    StudioAxisHandle depth=model;depth.physicalPixelsPerUnit={.01f,0};
    depth.fallbackPixelsPerUnit=24;
    assert(close(StudioGizmoDragPolicy::MoveUnits(depth,{48,0}),2));
    assert(close(StudioGizmoDragPolicy::MoveUnits(depth,{48,0},true),.2f));
    assert(close(StudioGizmoDragPolicy::MoveUnits(depth,{0,0}),0));
    assert(close(StudioGizmoDragPolicy::MoveUnits(depth,{NAN,0}),0));
    // Reflow must copy frozen calibration despite changing onscreen shafts.
    const auto projected=P::BuildHandles({320,190},{{{1,0},{0,-1},{0,1}}},bounds);
    auto calibrated=projected;
    calibrated[0].physicalPixelsPerUnit={100,0};
    calibrated[0].fallbackPixelsPerUnit=22;
    calibrated[0].projectedAxisUsable=true;
    const auto shifted=P::ReflowForOcclusion(calibrated,bounds,
        [](float x,float y){return x>365&&y>178&&y<215;});
    assert(shifted[0].valid);
    assert(close(shifted[0].physicalPixelsPerUnit.x,100));
    assert(close(shifted[0].fallbackPixelsPerUnit,22));
    assert(shifted[0].projectedAxisUsable);
    assert(close(StudioGizmoDragPolicy::MoveUnits(shifted[0],{100,0}),1));
    // Camera/zoom/axis sweep: pixel displacement of two projected authored
    // units must recover two units regardless of screen bearing and density.
    int calibrations=0;
    for(int degrees=0;degrees<360;degrees+=5)for(float density:{3.0f,8.0f,24.0f,80.0f,180.0f}){
        const float a=degrees*.017453292519943295f;
        StudioAxisHandle h=model;
        h.physicalPixelsPerUnit={density*std::cos(a),density*std::sin(a)};
        const StudioPoint expected{h.physicalPixelsPerUnit.x*2,h.physicalPixelsPerUnit.y*2};
        const StudioPoint lateral{-std::sin(a)*13,std::cos(a)*13};
        assert(std::fabs(StudioGizmoDragPolicy::MoveUnits(h,expected)-2.0f)<.0002f);
        assert(std::fabs(StudioGizmoDragPolicy::MoveUnits(h,
            {expected.x+lateral.x,expected.y+lateral.y})-2.0f)<.0002f);
        ++calibrations;
    }
    assert(calibrations==360);
    std::cout<<"Studio gizmo offscreen probes / end-on / angular separation / picking: PASS\n";
}
