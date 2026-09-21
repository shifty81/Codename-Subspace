#include "studio/StudioGizmoProjectionPolicy.h"
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
    std::cout<<"Studio gizmo offscreen probes / end-on / angular separation / picking: PASS\n";
}
