#include <cmath>
#include <iostream>
#include <memory>
#include <utility>
#include "fleet/FleetStrategyControlSystem.h"
#include "core/physics/PhysicsComponent.h"
#include "input/ControlIntentRouterSystem.h"
#include "input/MouseLookProfileSystem.h"
#include "input/PlayerControlSystem.h"
#include "interior/ShipEmbodimentSystem.h"
#include "interior/StarterShipInteriorSceneSystem.h"
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
    InputState fleetExitInput;fleetExitInput.SetAction(InputAction::CharacterInteract,true);
    CHECK("fleet F remains an explicit physical exit action",ControlIntentRouterSystem::Build(fleetExitInput,ControlDomain::FleetStrategy).interact);

    const auto rightMouse=MouseLookProfileSystem::OnFoot(20.0f,0.0f);
    CHECK("rightward raw mouse produces rightward FPS yaw",rightMouse.yawRadians<0.0f);
    const auto pilotMouseRight=MouseLookProfileSystem::PilotSteer(20.0f,0.0f);
    const auto pilotMouseUp=MouseLookProfileSystem::PilotSteer(0.0f,-20.0f);
    CHECK("pilot mouse-right maps to positive steer-right axis",pilotMouseRight.yawAxis>0.0f&&std::fabs(pilotMouseRight.pitchAxis)<.0001f);
    CHECK("pilot mouse-up maps to negative screen-down pitch axis",pilotMouseUp.pitchAxis<0.0f&&std::fabs(pilotMouseUp.yawAxis)<.0001f);

    auto runFlightCase=[](InputAction action,float value,Vector3 rotation=Vector3{}){
        EntityManager entities;InputState flightInput;PlayerControlSystem flightControls(entities,flightInput);
        auto& entity=entities.CreateEntity("R193 control fixture");
        auto component=std::make_unique<PhysicsComponent>();
        component->mass=1000.0f;component->momentOfInertia=1000.0f;component->maxThrust=100.0f;component->maxTorque=50.0f;component->rotation=rotation;
        auto* physics=entities.AddComponent<PhysicsComponent>(entity.id,std::move(component));
        flightControls.SetControlledShip(entity.id);flightInput.SetActionValue(action,value);flightControls.Update(.10f);
        return std::pair<Vector3,Vector3>{physics->appliedForce,physics->appliedTorque};
    };
    const auto levelForward=runFlightCase(InputAction::PilotForward,1.0f);
    CHECK("level W is nose-forward without accidental vertical thrust",levelForward.first.y>1.0f&&std::fabs(levelForward.first.x)<.001f&&std::fabs(levelForward.first.z)<.001f);
    const auto thrustUp=runFlightCase(InputAction::PilotThrustUp,1.0f);
    CHECK("Space owns positive local vertical thrust",thrustUp.first.z>1.0f&&std::fabs(thrustUp.first.y)<.001f);
    const auto rollLeft=runFlightCase(InputAction::FlightRollLeft,1.0f);
    const auto rollRight=runFlightCase(InputAction::FlightRollRight,1.0f);
    CHECK("Q is roll-left",rollLeft.second.y<-.1f&&std::fabs(rollLeft.second.x)<.001f&&std::fabs(rollLeft.second.z)<.001f);
    CHECK("E is roll-right",rollRight.second.y>.1f&&std::fabs(rollRight.second.x)<.001f&&std::fabs(rollRight.second.z)<.001f);
    {
        EntityManager entities;InputState flightInput;PlayerControlSystem flightControls(entities,flightInput);
        auto& entity=entities.CreateEntity("R193 mouse fixture");auto component=std::make_unique<PhysicsComponent>();component->maxTorque=50.0f;
        auto* physics=entities.AddComponent<PhysicsComponent>(entity.id,std::move(component));flightControls.SetControlledShip(entity.id);
        flightInput.SetActionValue(InputAction::TurnRight,pilotMouseRight.yawAxis);flightControls.Update(.016f);
        CHECK("mouse-right produces immediate right-yaw torque",physics->appliedTorque.z<-20.0f);
    }
    {
        EntityManager entities;InputState flightInput;PlayerControlSystem flightControls(entities,flightInput);
        auto& entity=entities.CreateEntity("R193 mouse pitch fixture");auto component=std::make_unique<PhysicsComponent>();component->maxTorque=50.0f;
        auto* physics=entities.AddComponent<PhysicsComponent>(entity.id,std::move(component));flightControls.SetControlledShip(entity.id);
        flightInput.SetActionValue(InputAction::FlightPitchUp,-pilotMouseUp.pitchAxis);flightControls.Update(.016f);
        CHECK("mouse-up produces immediate pitch-up torque",physics->appliedTorque.x>16.0f);
    }
    const auto pitchedForward=runFlightCase(InputAction::PilotForward,1.0f,{.45f,0.0f,0.0f});
    CHECK("after deliberate pitch W follows the ship nose",pitchedForward.first.y>1.0f&&pitchedForward.first.z>1.0f);
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

    InteriorLayoutPlan layout;layout.carve.valid=true;layout.shell.ready=true;
    InteriorCarvedVolume cabin;cabin.moduleIndex=1;cabin.moduleId="starter.cabin";cabin.roomType=InteriorRoomType::Cockpit;cabin.center={0,0,1.2f};cabin.halfExtents={2.0f,3.0f,1.2f};cabin.walkable=true;cabin.pressureCapable=true;layout.carve.volumes.push_back(cabin);
    const auto starter=StarterShipInteriorSceneSystem::Build(layout);
    CHECK("starter interior materializes functional fixtures",starter.ready&&starter.fixtures.size()>=5&&starter.fixtures.front().interaction.kind==InteriorFixtureKind::HelmSeat);
    ShipEmbodimentSystem walker;walker.ExitCockpit(22);walker.SetCertifiedFootPosition(starter.spawnFeet);
    const auto focus=StarterShipInteriorSceneSystem::Focus(starter,walker.Avatar(),4.0f,.40f);
    CHECK("starter FPS spawn faces the physical helm",focus.valid()&&focus.fixtureIndex==0);
    CHECK("starter helm can transfer nearby embodiment to pilot",walker.TakeControlsAt(starter.fixtures.front().useFeet,starter.fixtures.front().interaction.interactionRangeMeters+.20f)&&walker.IsPiloting());
    const auto blocked=StarterShipInteriorSceneSystem::ResolveFixtureCollision(starter,starter.spawnFeet,starter.fixtures.front().localCenter,.32f);
    CHECK("starter furnishings participate in locomotion collision",(blocked-starter.fixtures.front().localCenter).length()>.05f);

    std::cout<<"R189/R191/R192/R193 assertions: "<<passed<<" passed / "<<failed<<" failed\n";
    return failed?1:0;
}
