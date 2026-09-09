#pragma once
#include "navigation/VectorTravelSystem.h"
#include "hangar/DockingExperienceSystem.h"
#include <string>
namespace subspace {
struct ProductionAudioCue { std::string cueId; float volume=1.0f; float pitch=1.0f; bool loop=false; };
class ProductionAudioCueSystem { public: ProductionAudioCue ForVector(VectorTravelStage stage,double progress) const; ProductionAudioCue ForDocking(DockingExperienceStage stage) const; ProductionAudioCue ForThruster(float throttle,bool reverse) const; };
}
