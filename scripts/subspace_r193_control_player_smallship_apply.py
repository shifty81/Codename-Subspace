#!/usr/bin/env python3
"""Transactional R193 source migration for cdb0f0b.

The PCC package installs new authority/test files. This migration performs the
small exact-anchor edits required in large active runtime sources without
shipping whole-file replacements that could erase unrelated local changes.
"""
from __future__ import annotations

import argparse
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from datetime import datetime, timezone

BASELINE = "cdb0f0b796c0f7c79a851dc7c9767260e1861c30"
PREIMAGE_SHA256 = {
    "engine/src/input/PlayerControlSystem.cpp": "e7032abba946df7506706242ffc67c1443b09ab9ba644c2b9f9d87a6e650e434",
    "engine/src/application/NativeGameApplication.cpp": "c939bb3b81adafdb017ea67df98f17bff68723bd20a9f18c92009ec2884fe55e",
    "engine/src/application/NativeBattlefieldRenderer.cpp": "66a9d65aa8f4bd1a992301d641ee44e9ddbd3adf70114675d0e8c8f769b2063b",
    "engine/tests/r189_strategy_fps_cutover_tests.cpp": "7b04592b4eb4b441e5bd74aed7bb8e98fa6ad8b4d431cfe507d3fbf51466357c",
    "engine/CMakeLists.txt": "21dc370a2389a81853de318c10ede023159df4255980459d7752c07ca04f0feb",
}

MARKERS = {
    "engine/src/input/PlayerControlSystem.cpp": "R193: yaw/pitch are transient mouse-rate demands",
    "engine/src/application/NativeGameApplication.cpp": "R193_FULL_SHIP_ATTITUDE",
    "engine/src/application/NativeBattlefieldRenderer.cpp": "R193: the playable interior is ship-local in all three attitude axes",
    "engine/tests/r189_strategy_fps_cutover_tests.cpp": "mouse-right produces immediate right-yaw torque",
    "engine/CMakeLists.txt": "SubspaceR193PlayerScaleSmallShipTests",
}


def sha(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def git_head(root: Path) -> str:
    try:
        return subprocess.check_output(["git", "-C", str(root), "rev-parse", "HEAD"], text=True, stderr=subprocess.DEVNULL).strip()
    except Exception:
        return ""


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one source anchor, found {count}")
    return text.replace(old, new, 1)


def migrated(root: Path) -> bool:
    for rel, marker in MARKERS.items():
        path = root / rel
        if not path.is_file() or marker not in path.read_text(encoding="utf-8"):
            return False
    return True


def preflight(root: Path) -> None:
    head = git_head(root)
    if head and head != BASELINE and not migrated(root):
        raise RuntimeError(f"R193 baseline mismatch: expected {BASELINE}, current {head}. Rebase the patch; do not force it.")
    if migrated(root):
        return
    mismatches = []
    for rel, expected in PREIMAGE_SHA256.items():
        path = root / rel
        if not path.is_file():
            mismatches.append(f"missing {rel}")
            continue
        actual = sha(path)
        if actual != expected:
            mismatches.append(f"{rel}: expected {expected}, got {actual}")
    if mismatches:
        raise RuntimeError("R193 preimage conflict; refusing to overwrite local edits:\n  " + "\n  ".join(mismatches))


def transformed(root: Path) -> dict[str, str]:
    out: dict[str, str] = {}

    rel = "engine/src/input/PlayerControlSystem.cpp"
    text = (root / rel).read_text(encoding="utf-8")
    text = replace_once(text,
'''    _yawResponse=SmoothResponse(_yawResponse,rawYaw,_tuning.rotationalResponse,deltaTime);
    _pitchResponse=SmoothResponse(_pitchResponse,rawPitch,_tuning.rotationalResponse,deltaTime);
    _rollResponse=SmoothResponse(_rollResponse,rawRoll,_tuning.rotationalResponse,deltaTime);''',
'''    // R193: yaw/pitch are transient mouse-rate demands in the Pilot profile.
    // Filtering a one-frame raw-mouse pulse like a held keyboard axis made normal
    // mouse movement almost disappear. Apply those axes directly; preserve the
    // smoother held-key response for Q/E roll.
    _yawResponse=rawYaw;
    _pitchResponse=rawPitch;
    _rollResponse=SmoothResponse(_rollResponse,rawRoll,_tuning.rotationalResponse,deltaTime);''',
"PlayerControlSystem direct mouse response")
    out[rel] = text

    rel = "engine/src/application/NativeGameApplication.cpp"
    text = (root / rel).read_text(encoding="utf-8")
    anchor = '''Vector3 ShipLocalToWorld(const Vector3& local,float shipYaw)
{
    const float c=std::cos(shipYaw),s=std::sin(shipYaw);
    return {local.x*c-local.y*s,local.x*s+local.y*c,local.z};
}
'''
    helper = anchor + '''
Vector3 RotateShipLocalToWorld(const Vector3& v,const Vector3& r)
{
    // R193: Rz * Ry * Rx, identical to the Full3D flight-physics basis.
    const float cx=std::cos(r.x),sx=std::sin(r.x);
    const float cy=std::cos(r.y),sy=std::sin(r.y);
    const float cz=std::cos(r.z),sz=std::sin(r.z);
    const Vector3 x{v.x,v.y*cx-v.z*sx,v.y*sx+v.z*cx};
    const Vector3 y{x.x*cy+x.z*sy,x.y,-x.x*sy+x.z*cy};
    return {y.x*cz-y.y*sz,y.x*sz+y.y*cz,y.z};
}
'''
    text = replace_once(text, anchor, helper, "NativeGameApplication full-attitude helper")
    text = replace_once(text,
'''    if(hudPlayer&&_gameplayMode==GameplayControlMode::OnFoot&&_embodiment.IsOnFoot()){const auto local=FirstPersonViewSystem::BuildOnFootLocal(_embodiment.Avatar());const float yaw=hudPlayer->rotation.z,cy=std::cos(yaw),sy=std::sin(yaw);const auto rotate=[&](const Vector3& v){return Vector3{v.x*cy-v.y*sy,v.x*sy+v.y*cy,v.z};};f.firstPersonPose=local;f.firstPersonPose.position=hudPlayer->position+rotate(local.position)*.72f;f.firstPersonPose.forward=rotate(local.forward).normalized();f.firstPersonPose.right=rotate(local.right).normalized();f.firstPersonPose.up=rotate(local.up).normalized();f.hasFirstPersonPose=true;} // R189_REAL_FPS_POSE''',
'''    if(hudPlayer&&_gameplayMode==GameplayControlMode::OnFoot&&_embodiment.IsOnFoot()){const auto local=FirstPersonViewSystem::BuildOnFootLocal(_embodiment.Avatar());const auto rotate=[&](const Vector3& v){return RotateShipLocalToWorld(v,hudPlayer->rotation);};f.firstPersonPose=local;f.firstPersonPose.position=hudPlayer->position+rotate(local.position)*.72f;f.firstPersonPose.forward=rotate(local.forward).normalized();f.firstPersonPose.right=rotate(local.right).normalized();f.firstPersonPose.up=rotate(local.up).normalized();f.hasFirstPersonPose=true;} // R189_REAL_FPS_POSE / R193_FULL_SHIP_ATTITUDE''',
"OnFoot full ship attitude")
    text = replace_once(text,
'''    if(hudPlayer&&_gameplayMode==GameplayControlMode::Pilot&&_embodiment.IsPiloting()){const auto local=FirstPersonViewSystem::BuildCockpitLocal(_embodiment.EyeLocalPosition(),_pilotHeadYawRadians,_pilotHeadPitchRadians);const float yaw=hudPlayer->rotation.z,cy=std::cos(yaw),sy=std::sin(yaw);const auto rotate=[&](const Vector3& v){return Vector3{v.x*cy-v.y*sy,v.x*sy+v.y*cy,v.z};};f.firstPersonPose=local;f.firstPersonPose.position=hudPlayer->position+rotate(local.position)*.72f;f.firstPersonPose.forward=rotate(local.forward).normalized();f.firstPersonPose.right=rotate(local.right).normalized();f.firstPersonPose.up=rotate(local.up).normalized();f.hasFirstPersonPose=true;} // R191_REAL_COCKPIT_POSE''',
'''    if(hudPlayer&&_gameplayMode==GameplayControlMode::Pilot&&_embodiment.IsPiloting()){const auto local=FirstPersonViewSystem::BuildCockpitLocal(_embodiment.EyeLocalPosition(),_pilotHeadYawRadians,_pilotHeadPitchRadians);const auto rotate=[&](const Vector3& v){return RotateShipLocalToWorld(v,hudPlayer->rotation);};f.firstPersonPose=local;f.firstPersonPose.position=hudPlayer->position+rotate(local.position)*.72f;f.firstPersonPose.forward=rotate(local.forward).normalized();f.firstPersonPose.right=rotate(local.right).normalized();f.firstPersonPose.up=rotate(local.up).normalized();f.hasFirstPersonPose=true;} // R191_REAL_COCKPIT_POSE / R193_FULL_SHIP_ATTITUDE''',
"Pilot full ship attitude")
    out[rel] = text

    rel = "engine/src/application/NativeBattlefieldRenderer.cpp"
    text = (root / rel).read_text(encoding="utf-8")
    text = replace_once(text,
'''    glTranslatef(ship.position.x,ship.position.y,ship.position.z);
    glRotatef(ship.rotation.z*180.0f/kPi,0,0,1);
    glScalef(.72f,.72f,.72f); // same ship-local->world scale as avatar camera''',
'''    glTranslatef(ship.position.x,ship.position.y,ship.position.z);
    // R193: the playable interior is ship-local in all three attitude axes.
    // OpenGL post-multiplication yields Rz * Ry * Rx, matching flight physics.
    glRotatef(ship.rotation.z*180.0f/kPi,0,0,1);
    glRotatef(ship.rotation.y*180.0f/kPi,0,1,0);
    glRotatef(ship.rotation.x*180.0f/kPi,1,0,0);
    glScalef(.72f,.72f,.72f); // same ship-local->world scale as avatar camera''',
"interior full ship attitude")
    text = replace_once(text,
'''        if(frame.embodimentMode==ShipEmbodimentMode::InteriorOnFoot){
            if(frame.cutaway.visible)glDepthMask(GL_TRUE);
            DrawPlayableInterior(frame);
        }else{''',
'''        if(frame.hasFirstPersonPose){
            // R193: both walking and seated cockpit first-person views render
            // the authored ship-local interior rather than a yaw-only exterior.
            if(frame.cutaway.visible)glDepthMask(GL_TRUE);
            DrawPlayableInterior(frame);
        }else{''',
"seated cockpit physical interior")
    out[rel] = text

    rel = "engine/tests/r189_strategy_fps_cutover_tests.cpp"
    text = (root / rel).read_text(encoding="utf-8")
    text = replace_once(text, '#include <iostream>\n', '#include <iostream>\n#include <memory>\n#include <utility>\n', "R189 test standard includes")
    text = replace_once(text, '#include "fleet/FleetStrategyControlSystem.h"\n', '#include "fleet/FleetStrategyControlSystem.h"\n#include "core/physics/PhysicsComponent.h"\n', "R189 physics include")
    text = replace_once(text, '#include "input/MouseLookProfileSystem.h"\n', '#include "input/MouseLookProfileSystem.h"\n#include "input/PlayerControlSystem.h"\n', "R189 control include")
    anchor = '''    const auto rightMouse=MouseLookProfileSystem::OnFoot(20.0f,0.0f);
    CHECK("rightward raw mouse produces rightward FPS yaw",rightMouse.yawRadians<0.0f);
'''
    addition = anchor + '''    const auto pilotMouseRight=MouseLookProfileSystem::PilotSteer(20.0f,0.0f);
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
'''
    text = replace_once(text, anchor, addition, "R189 R193 flight regression block")
    text = replace_once(text, 'R189/R191/R192 assertions: ', 'R189/R191/R192/R193 assertions: ', "R189 summary")
    out[rel] = text

    rel = "engine/CMakeLists.txt"
    text = (root / rel).read_text(encoding="utf-8")
    anchor = '''# R189: strategy-first runtime / real FPS cutover.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_r189_strategy_fps_cutover_tests tests/r189_strategy_fps_cutover_tests.cpp)
    target_link_libraries(subspace_r189_strategy_fps_cutover_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceR189StrategyFpsCutoverTests COMMAND subspace_r189_strategy_fps_cutover_tests)
endif()
'''
    addition = anchor + '''
# R193: canonical player-scale authority + bounded small-ship certification.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_r193_player_scale_smallship_tests tests/r193_player_scale_smallship_tests.cpp)
    target_link_libraries(subspace_r193_player_scale_smallship_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceR193PlayerScaleSmallShipTests COMMAND subspace_r193_player_scale_smallship_tests)
    add_test(NAME SubspaceR193PlayerScaleSmallShipSourceGate COMMAND ${CMAKE_COMMAND}
        -DROOT=${CMAKE_CURRENT_SOURCE_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/../tools/control/static-gates/nullharbor_r193_player_scale_smallship.cmake)
endif()
'''
    text = replace_once(text, anchor, addition, "CMake R193 registration")
    out[rel] = text
    return out


def apply(root: Path) -> None:
    preflight(root)
    if migrated(root):
        print("[PASS] R193 source migration already present and marker-complete.")
        return
    outputs = transformed(root)
    stamp = datetime.now(timezone.utc).strftime("%Y%m%d-%H%M%S")
    backup = root / "artifacts" / "backups" / f"r193-control-player-smallship-{stamp}"
    backup.mkdir(parents=True, exist_ok=False)
    written: list[str] = []
    try:
        for rel, content in outputs.items():
            src = root / rel
            dst = backup / rel
            dst.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(src, dst)
            fd, temp_name = tempfile.mkstemp(prefix=src.name+".", suffix=".r193.tmp", dir=str(src.parent))
            try:
                with os.fdopen(fd, "w", encoding="utf-8", newline="") as f:
                    f.write(content)
                os.replace(temp_name, src)
            finally:
                if os.path.exists(temp_name): os.unlink(temp_name)
            written.append(rel)
        if not migrated(root):
            raise RuntimeError("post-write marker verification failed")
    except Exception:
        for rel in reversed(written):
            shutil.copy2(backup / rel, root / rel)
        raise
    print(f"[PASS] R193 transactional source migration applied. Backup: {backup}")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default=".")
    mode = ap.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true")
    mode.add_argument("--apply", action="store_true")
    args = ap.parse_args()
    root = Path(args.root).resolve()
    try:
        preflight(root)
        if args.check:
            state = "ALREADY_APPLIED" if migrated(root) else "READY_TO_APPLY"
            print(f"[PASS] R193 preflight: {state}; baseline={git_head(root) or 'NO_GIT'}")
            return 0
        apply(root)
        return 0
    except Exception as exc:
        print(f"[FAIL] {exc}", file=sys.stderr)
        return 2

if __name__ == "__main__":
    raise SystemExit(main())
