#include "ui/RuntimeControlContextSystem.h"
namespace subspace {
RuntimeControlContext RuntimeControlContextSystem::Build(SandboxWorkspaceMode workspace,ShipEmbodimentMode embodiment,DockingExperienceStage docking,bool vectorTransit,bool strategic) const{
    RuntimeControlContext c;
    if(workspace==SandboxWorkspaceMode::ShipBuilder){c.flightControls=false;c.weapons=false;c.scanner=false;c.vectorCommands=false;c.mouseLook=false;c.sixDofFlight=false;c.modeLabel="SHIPYARD / DEV MODE";c.cameraMode=CameraMode::ShipBuilder;c.viewAuthority=RuntimeViewAuthority::AuthoringDev;c.firstPerson=false;return c;}
    if(embodiment==ShipEmbodimentMode::InteriorOnFoot){c.flightControls=false;c.interiorControls=true;c.weapons=false;c.scanner=false;c.vectorCommands=false;c.sixDofFlight=false;c.modeLabel="ON FOOT / FIRST PERSON";c.cameraMode=CameraMode::OnFoot;c.viewAuthority=RuntimeViewAuthority::OnFootFirstPerson;c.firstPerson=true;return c;}
    if(docking==DockingExperienceStage::Docked||embodiment==ShipEmbodimentMode::DockedHangar){c.flightControls=false;c.dockingControls=true;c.weapons=false;c.vectorCommands=false;c.sixDofFlight=false;c.modeLabel="STATION HANGAR";c.cameraMode=CameraMode::DockedHangar;c.viewAuthority=RuntimeViewAuthority::DockedService;c.firstPerson=true;return c;}
    if(vectorTransit){c.flightControls=false;c.weapons=false;c.vectorCommands=false;c.sixDofFlight=false;c.modeLabel="VECTOR TRANSIT";c.cameraMode=CameraMode::ShipFlight;c.viewAuthority=RuntimeViewAuthority::Transit;c.firstPerson=true;return c;}
    if(strategic){c.modeLabel="REMOTE FLEET COMMAND";c.cameraMode=CameraMode::TacticalFleet;c.viewAuthority=RuntimeViewAuthority::RemoteFleetCommand;c.firstPerson=false;c.mouseLook=false;c.sixDofFlight=false;c.remoteFleetCommand=true;return c;}
    c.modeLabel="COCKPIT / FIRST PERSON 6DOF";c.mouseLook=true;c.sixDofFlight=true;return c;
}
RuntimeControlContext RuntimeControlContextSystem::BuildWithCommandSeat(SandboxWorkspaceMode workspace,
    ShipEmbodimentMode embodiment,DockingExperienceStage docking,bool vectorTransit,bool requested,
    std::uint64_t actorId,const FleetCommandSession& session,const FleetCommandSeatSnapshot& seat) const{
    const bool authorized=requested && FleetCommandSeatSystem::CanCommand(session,actorId,seat).allowed;
    // Legacy Build prioritizes on-foot/docked states over strategic view; a real
    // seated station terminal must be allowed to take UI ownership from on-foot.
    RuntimeControlContext context=Build(workspace,embodiment,docking,vectorTransit,false);
    if(authorized && workspace!=SandboxWorkspaceMode::ShipBuilder && !vectorTransit){
        context.cameraMode=CameraMode::TacticalFleet;
        context.viewAuthority=RuntimeViewAuthority::RemoteFleetCommand;
        context.modeLabel="REMOTE FLEET COMMAND / SEATED";
        context.remoteFleetCommand=true;
        context.firstPerson=false;
        // Tactical pointer must not simultaneously drive vessel flight or fire weapons.
        context.flightControls=false;
        context.weapons=false;
        context.scanner=false;
        context.vectorCommands=false;
        context.interiorControls=false;
        context.dockingControls=false;
        context.mouseLook=false;
        context.sixDofFlight=false;
    }
    return context;
}
}