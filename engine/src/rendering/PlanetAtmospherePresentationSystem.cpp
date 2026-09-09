#include "rendering/PlanetAtmospherePresentationSystem.h"
#include <cmath>
namespace subspace {
PlanetAtmospherePresentationProfile PlanetAtmospherePresentationSystem::Default(){return {};}
float PlanetAtmospherePresentationSystem::Alpha(float vd,const PlanetAtmospherePresentationProfile&p,float sunlight){vd=std::clamp(vd,0.0f,1.0f);sunlight=std::clamp(sunlight,0.0f,1.0f);const float limb=std::pow(1.0f-vd,p.limbPower);return std::clamp((p.centerOpacity+(p.limbOpacity-p.centerOpacity)*limb)*(.18f+.82f*sunlight),0.0f,1.0f);}
} // namespace subspace
