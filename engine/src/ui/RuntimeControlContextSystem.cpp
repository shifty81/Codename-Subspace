#include "ui/RuntimeControlContextSystem.h"
namespace subspace {
RuntimeControlContext RuntimeControlContextSystem::BuildForMode(SandboxWorkspaceMode workspace,GameplayControlMode gameplayMode,ShipEmbodimentMode embodiment,DockingExperienceStage docking,bool vectorTransit) const{
    RuntimeControlContext c;c.gameplayMode=gameplayMode;
    if(workspace==SandboxWorkspaceMode::ShipBuilder){c.controlDomain=ControlDomain::Authoring;c.flightControls=false;c.weapons=false;c.scanner=false;c.vectorCommands=false;c.mouseLook=false;c.sixDofFlight=false;c.modeLabel="SHIPYARD / DEV MODE";c.cameraMode=CameraMode::ShipBuilder;c.viewAuthority=RuntimeViewAuthority::AuthoringDev;c.firstPerson=false;c.pointerPolicy=RuntimePointerPolicy::AbsoluteVisible;return c;}
    if(vectorTransit){c.controlDomain=ControlDomain::Transit;c.flightControls=false;c.weapons=false;c.vectorCommands=false;c.sixDofFlight=false;c.modeLabel="VECTOR TRANSIT";c.cameraMode=CameraMode::ShipFlight;c.viewAuthority=RuntimeViewAuthority::Transit;c.firstPerson=true;c.pointerPolicy=RuntimePointerPolicy::AbsoluteVisible;return c;}
    if(gameplayMode==GameplayControlMode::FleetCommand){
        c.controlDomain=ControlDomain::FleetStrategy;c.modeLabel="FLEET COMMAND";c.cameraMode=CameraMode::TacticalFleet;c.viewAuthority=RuntimeViewAuthority::RemoteFleetCommand;
        c.firstPerson=false;c.mouseLook=false;c.sixDofFlight=false;c.remoteFleetCommand=true;c.flightControls=false;c.weapons=false;c.scanner=false;c.vectorCommands=false;c.interiorControls=false;c.dockingControls=false;c.pointerPolicy=RuntimePointerPolicy::AbsoluteVisible;return c;
    }
    if(gameplayMode==GameplayControlMode::OnFoot){
        c.flightControls=false;c.weapons=false;c.scanner=false;c.vectorCommands=false;c.sixDofFlight=false;c.interiorControls=true;c.firstPerson=true;c.mouseLook=true;c.pointerPolicy=RuntimePointerPolicy::RelativeCaptured;
        if(docking==DockingExperienceStage::Docked||embodiment==ShipEmbodimentMode::DockedHangar){c.controlDomain=ControlDomain::DockedService;c.dockingControls=true;c.modeLabel="STATION / ON FOOT";c.cameraMode=CameraMode::DockedHangar;c.viewAuthority=RuntimeViewAuthority::DockedService;return c;}
        c.controlDomain=ControlDomain::FirstPerson;c.modeLabel="ON FOOT / FIRST PERSON";c.cameraMode=CameraMode::OnFoot;c.viewAuthority=RuntimeViewAuthority::OnFootFirstPerson;return c;
    }
    c.controlDomain=ControlDomain::Pilot;c.gameplayMode=GameplayControlMode::Pilot;c.modeLabel="COCKPIT / PILOTING";c.mouseLook=true;c.sixDofFlight=true;c.pointerPolicy=RuntimePointerPolicy::RelativeCaptured;return c;
}
RuntimeControlContext RuntimeControlContextSystem::Build(SandboxWorkspaceMode workspace,ShipEmbodimentMode embodiment,DockingExperienceStage docking,bool vectorTransit,bool strategic) const{
    const GameplayControlMode mode=strategic?GameplayControlMode::FleetCommand:(embodiment==ShipEmbodimentMode::InteriorOnFoot||embodiment==ShipEmbodimentMode::DockedHangar?GameplayControlMode::OnFoot:GameplayControlMode::Pilot);
    return BuildForMode(workspace,mode,embodiment,docking,vectorTransit);
}
RuntimeControlContext RuntimeControlContextSystem::BuildWithCommandSeat(SandboxWorkspaceMode workspace,
    ShipEmbodimentMode embodiment,DockingExperienceStage docking,bool vectorTransit,bool requested,
    std::uint64_t actorId,const FleetCommandSession& session,const FleetCommandSeatSnapshot& seat) const{
    const bool authorized=requested && FleetCommandSeatSystem::CanCommand(session,actorId,seat).allowed;
    RuntimeControlContext context=BuildForMode(workspace,embodiment==ShipEmbodimentMode::InteriorOnFoot||embodiment==ShipEmbodimentMode::DockedHangar?GameplayControlMode::OnFoot:GameplayControlMode::Pilot,embodiment,docking,vectorTransit);
    if(authorized && workspace!=SandboxWorkspaceMode::ShipBuilder && !vectorTransit)context=BuildForMode(workspace,GameplayControlMode::FleetCommand,embodiment,docking,false);
    if(authorized)context.modeLabel="FLEET COMMAND / SEATED";
    return context;
}
}
