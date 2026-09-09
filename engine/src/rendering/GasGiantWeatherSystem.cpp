#include "rendering/GasGiantWeatherSystem.h"
#include <algorithm>
#include <cmath>
namespace subspace {
namespace {float Unit(std::uint32_t& x){x=x*1103515245u+12345u;return float((x>>8)&0xffffu)/65535.0f;}}
GasGiantWeatherProfile GasGiantWeatherSystem::Build(const PlanetData& p,std::uint32_t seed){GasGiantWeatherProfile g;if(p.type!=PlanetType::GasGiant)return g;std::uint32_t r=seed?seed:1;g.volatileAtmosphere=p.hazardLevel>.48f;g.lightningProbability=.08f+p.hazardLevel*.52f;g.giantVortexProbability=.20f+p.hazardLevel*.62f;for(int i=-5;i<=5;++i){GasGiantBandState b;b.latitude=float(i)/5.5f;b.angularVelocity=((i%2)?-1.0f:1.0f)*(.00035f+.0010f*Unit(r))*(1.0f+p.hazardLevel);b.turbulence=.25f+.65f*Unit(r);b.brightness=.76f+.42f*Unit(r);g.bands.push_back(b);}return g;}
float GasGiantWeatherSystem::BandLongitude(const GasGiantBandState& b,double seconds){return std::fmod(float(seconds)*b.angularVelocity,6.2831853f);}
} // namespace subspace
