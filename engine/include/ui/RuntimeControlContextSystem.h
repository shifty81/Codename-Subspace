#pragma once
#include "hangar/DockingExperienceSystem.h"
#include "interior/ShipEmbodimentSystem.h"
#include "rendering/EnvironmentPresentationSystem.h"
#include "ui/SandboxWorkspaceSystem.h"
#include <string>
namespace subspace {
struct RuntimeControlContext {
    CameraMode cameraMode=CameraMode::ShipFlight;
    bool flightControls=true;
    bool interiorControls=false;
    bool dockingControls=false;
    bool weapons=true;
    bool scanner=true;
    bool vectorCommands=true;
    std::string modeLabel="FLIGHT";
};
class RuntimeControlContextSystem {
public:
    RuntimeControlContext Build(SandboxWorkspaceMode workspace,ShipEmbodimentMode embodiment,DockingExperienceStage docking,bool vectorTransit,bool strategicFlight) const;
};
}
