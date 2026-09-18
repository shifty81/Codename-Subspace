#pragma once
#include "hangar/DockingExperienceSystem.h"
#include "fleet/FleetCommandSeatSystem.h"
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
    bool mouseLook=true;
    bool sixDofFlight=true;
    bool remoteFleetCommand=false;
    bool flightControls=true;
    bool interiorControls=false;
    bool dockingControls=false;
    bool weapons=true;
    bool scanner=true;
    bool vectorCommands=true;
    bool authoringPlaytest=false;
    std::string modeLabel="COCKPIT / FIRST PERSON 6DOF";
};
class RuntimeControlContextSystem {
public:
    RuntimeControlContext Build(SandboxWorkspaceMode workspace,ShipEmbodimentMode embodiment,DockingExperienceStage docking,bool vectorTransit,bool strategicFlight) const;
    // Seat-aware path for the physical Fleet Command Terminal / compatible pilot seat.
    RuntimeControlContext BuildWithCommandSeat(SandboxWorkspaceMode workspace,ShipEmbodimentMode embodiment,
        DockingExperienceStage docking,bool vectorTransit,bool requested,std::uint64_t actorId,
        const FleetCommandSession& session,const FleetCommandSeatSnapshot& seat) const;
};
}
