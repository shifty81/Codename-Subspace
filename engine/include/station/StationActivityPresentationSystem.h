#pragma once
#include "hangar/DockingExperienceSystem.h"
#include "station/StationEcologySystem.h"
namespace subspace {
struct StationActivityProfile { int cargoCraft=0; int serviceDrones=0; int securityCraft=0; int inboundTraffic=0; int outboundTraffic=0; int approachLanes=1; int activeDockLights=0; float industrialMotion=0.0f; float trafficDensity=0.0f; bool constructionActivity=false; bool queueing=false; };
class StationActivityPresentationSystem { public: StationActivityProfile Build(int population, bool shipyard, DockingExperienceStage stage) const; StationActivityProfile Build(int population,StationArchetype archetype,DockingExperienceStage stage) const; };
}
