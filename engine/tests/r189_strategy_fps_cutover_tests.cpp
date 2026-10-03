#include <cmath>
#include <iostream>
#include "fleet/FleetStrategyControlSystem.h"
#include "input/ControlIntentRouterSystem.h"
#include "interior/ShipEmbodimentSystem.h"
#include "rendering/FirstPersonViewSystem.h"
#include "runtime/GameplayControlMode.h"
#include "ui/RuntimeControlContextSystem.h"
using namespace subspace;
static int passed=0,failed=0;
#define CHECK(n,e) do{if(e){++passed;std::cout<<"PASS: "<<n<<"\n";}else{++failed;std::cout<<"FAIL: "<<n<<"\n";}}while(0)
int main(){
    InputState input; input.SetAction(InputAction::ThrustForward,true);
    auto strategyIntent=ControlIntentRouterSystem::Build(input,ControlDomain::FleetStrategy);
    CHECK("strategy WASD camera-only",strategyIntent.cameraForward>.9f&&strategyIntent.forward==0&&!strategyIntent.AllowsShipThrust());
    FleetStrategyControlSystem fleet; auto before=fleet.Camera().focus; fleet.TickCamera(strategyIntent,.1f);
    CHECK("fleet camera pans",(fleet.Camera().focus-before).length()>.01f);

    RuntimeControlContextSystem contexts;
    auto remote=contexts.BuildForMode(SandboxWorkspaceMode::Flight,GameplayControlMode::FleetCommand,ShipEmbodimentMode::InteriorOnFoot,DockingExperienceStage::Undocked,false);
    CHECK("fleet context is explicit",remote.gameplayMode==GameplayControlMode::FleetCommand&&remote.controlDomain==ControlDomain::FleetStrategy&&!remote.flightControls&&!remote.weapons);
    CHECK("fleet cursor is absolute",remote.pointerPolicy==RuntimePointerPolicy::AbsoluteVisible&&!remote.firstPerson);

    ShipEmbodimentSystem body; CHECK("bootable body enters FPS",body.ExitCockpit(9)&&body.IsOnFoot());
    auto fps=contexts.BuildForMode(SandboxWorkspaceMode::Flight,GameplayControlMode::OnFoot,body.Mode(),DockingExperienceStage::Undocked,false);
    CHECK("FPS owns avatar",fps.gameplayMode==GameplayControlMode::OnFoot&&fps.controlDomain==ControlDomain::FirstPerson&&fps.interiorControls&&!fps.flightControls);
    CHECK("FPS cursor is captured-relative",fps.pointerPolicy==RuntimePointerPolicy::RelativeCaptured&&fps.mouseLook);

    InputState fpsInput;fpsInput.SetAction(InputAction::CharacterMoveForward,true);fpsInput.SetAction(InputAction::CharacterCrouch,true);fpsInput.SetAction(InputAction::CharacterHeadLook,true);fpsInput.SetAction(InputAction::CharacterJump,true);
    const auto fpsIntent=ControlIntentRouterSystem::Build(fpsInput,ControlDomain::FirstPerson);
    CHECK("FPS semantic controls are isolated",fpsIntent.forward>.9f&&fpsIntent.crouch&&fpsIntent.headLook&&fpsIntent.jump&&!fpsIntent.AllowsShipThrust());

    body.Look(.2f,.1f);const float bodyYaw=body.Avatar().facingRadians;body.HeadLook(.4f,-.05f);
    CHECK("head-look does not rotate body",std::fabs(body.Avatar().facingRadians-bodyYaw)<.0001f&&std::fabs(body.Avatar().headLookYawOffsetRadians)>.1f);
    auto pose=FirstPersonViewSystem::BuildOnFootLocal(body.Avatar());
    CHECK("FPS eye pose includes independent head-look",pose.position.z>1.5f&&pose.verticalFovDegrees>50.0f&&std::fabs(pose.forward.x)>.1f);
    body.UpdateHeadLook(false,1.0);CHECK("head-look recenters",std::fabs(body.Avatar().headLookYawOffsetRadians)<.001f&&std::fabs(body.Avatar().headLookPitchOffsetRadians)<.001f);

    CHECK("physical helm requires command-seat proximity",body.CanTakeControls());
    CHECK("helm is explicit",body.TakeControls()&&body.IsPiloting());
    auto pilot=contexts.BuildForMode(SandboxWorkspaceMode::Flight,GameplayControlMode::Pilot,body.Mode(),DockingExperienceStage::Undocked,false);
    CHECK("pilot context is separate",pilot.gameplayMode==GameplayControlMode::Pilot&&pilot.controlDomain==ControlDomain::Pilot&&pilot.flightControls&&pilot.pointerPolicy==RuntimePointerPolicy::RelativeCaptured);
    InputState pilotInput;pilotInput.SetAction(InputAction::PilotForward,true);pilotInput.SetAction(InputAction::PilotThrustUp,true);pilotInput.SetAction(InputAction::PilotBoost,true);pilotInput.SetAction(InputAction::PilotHeadLook,true);
    const auto pilotIntent=ControlIntentRouterSystem::Build(pilotInput,ControlDomain::Pilot);
    CHECK("pilot semantics are isolated",pilotIntent.forward>.9f&&pilotIntent.up>.9f&&pilotIntent.boost&&pilotIntent.headLook&&pilotIntent.AllowsShipThrust());

    std::cout<<"R189/R191 assertions: "<<passed<<" passed / "<<failed<<" failed\n";
    return failed?1:0;
}
