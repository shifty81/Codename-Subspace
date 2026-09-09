#pragma once
#include <algorithm>
#include <cmath>
namespace subspace {
class PlanetPresentationSystem {
public:
    static float SmoothTerminator(float ndotl){float t=std::clamp((ndotl+0.08f)/0.22f,0.0f,1.0f);return t*t*(3.0f-2.0f*t);}
    static float AtmosphereLimb(float viewDotNormal){const float g=std::clamp(1.0f-std::fabs(viewDotNormal),0.0f,1.0f);const float g2=g*g;return g2*g2*g;}
    static float CloudDaylight(float ndotl){return 0.12f+0.88f*SmoothTerminator(ndotl);}

    // Pass474 weather-frame authority. Surface rotation and cloud rotation are
    // intentionally independent clocks; deriving cloud phase from the surface
    // phase made cloud artwork look painted onto the planet even when a small
    // multiplier was technically present.
    static float SurfaceRotationPhase(float seconds,float seed){
        const float rate=.00070f+.000003f*std::fmod(std::fabs(seed),17.0f);
        float phase=std::fmod(seconds*rate,1.0f);if(phase<0.0f)phase+=1.0f;return phase;
    }
    static float CloudRotationPhase(float seconds,int layer,float seed){
        // Low weather moves prograde faster than the crust; the high layer is
        // deliberately slower/retrograde so two cloud sheets never lock.
        const float seedJitter=.0000025f*std::fmod(std::fabs(seed),13.0f);
        const float rate=layer==0 ? (.00185f+seedJitter) : (-.00115f-seedJitter*.55f);
        const float offset=layer==0 ? .071f : .413f;
        float phase=std::fmod(offset+seconds*rate,1.0f);if(phase<0.0f)phase+=1.0f;return phase;
    }
    static float ProceduralWeatherLongitude(float longitude,float seconds,float drift){
        return longitude+seconds*.0120f*drift;
    }
};
}
