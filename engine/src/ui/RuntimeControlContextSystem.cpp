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
}
