#pragma once
#include "hangar/DockingExperienceSystem.h"
#include "interior/ShipEmbodimentSystem.h"
#include "rendering/EnvironmentPresentationSystem.h"
#include "ui/SandboxWorkspaceSystem.h"
#include <string>
namespace subspace {
enum class RuntimeViewAuthority { CockpitFirstPerson, OnFootFirstPerson, RemoteFleetCommand, AuthoringDev, DockedService, Transit };
struct RuntimeControlContext {
    CameraMode cameraMode=CameraMode::ShipFlight;
    RuntimeViewAuthority viewAuthority=RuntimeViewAuthority::CockpitFirstPerson;
    bool firstPerson=true;
    bool remoteFleetCommand=false;
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
