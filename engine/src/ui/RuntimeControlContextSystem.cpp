#include "ui/RuntimeControlContextSystem.h"
namespace subspace {
RuntimeControlContext RuntimeControlContextSystem::Build(SandboxWorkspaceMode workspace,ShipEmbodimentMode embodiment,DockingExperienceStage docking,bool vectorTransit,bool strategic) const{
    RuntimeControlContext c;
    if(workspace==SandboxWorkspaceMode::ShipBuilder){c.flightControls=false;c.weapons=false;c.scanner=false;c.vectorCommands=false;c.modeLabel="SHIPYARD";c.cameraMode=CameraMode::DockedHangar;return c;}
    if(embodiment==ShipEmbodimentMode::InteriorOnFoot){c.flightControls=false;c.interiorControls=true;c.weapons=false;c.scanner=false;c.vectorCommands=false;c.modeLabel="SHIP INTERIOR";c.cameraMode=CameraMode::OnFoot;return c;}
    if(docking==DockingExperienceStage::Docked||embodiment==ShipEmbodimentMode::DockedHangar){c.flightControls=false;c.dockingControls=true;c.weapons=false;c.vectorCommands=false;c.modeLabel="STATION HANGAR";c.cameraMode=CameraMode::DockedHangar;return c;}
    if(vectorTransit){c.flightControls=false;c.weapons=false;c.vectorCommands=false;c.modeLabel="VECTOR TRANSIT";c.cameraMode=CameraMode::ShipFlight;return c;}
    if(strategic){c.modeLabel="TACTICAL";c.cameraMode=CameraMode::TacticalFleet;return c;}
    return c;
}
}
