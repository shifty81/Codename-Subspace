#!/usr/bin/env python3
"""R182: certified-baseline recovery and R178/R179 forward port.

R178/R179 were authored in a validation tree whose application/renderer generation
predated the certified September 30 Studio source. R180 correctly failed closed on
a textual merge conflict; R181 correctly switched to semantic reconstruction but
its verifier incorrectly expected StudioApplication transform tokens in
NativeGameApplication. R182 fixes the ownership model instead of weakening gates.

For each exact known R178/R179 postimage, this tool:
  * proves the current Studio R82 authority in its actual owner files,
  * reads the certified September 30 file from Git provenance (git show),
  * verifies the exact baseline blob SHA,
  * reapplies only the intended R178/R179 semantic gameplay delta at exact anchors,
  * validates gameplay postconditions in the rebuilt file,
  * backs up all current working files, and
  * writes the rebuilt set transactionally with an idempotent receipt.

No reset, checkout, stash, force, broad source replacement, historical gate bypass,
or September-source promotion is used. Unknown/partial states fail closed.

R186 descendant policy: after a valid R182 transaction receipt exists, later
governed source evolution is authorized by semantic postconditions instead of
freezing all seven files to their original recovery SHA values forever.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
from datetime import datetime, timezone

BASELINE_COMMIT = "8484a081f8d60be980d3aac300ba2f1f9397cd75"
# Historical static-gate compatibility alias; R182 supersedes this strategy.
LEGACY_R181_STRATEGY_ID = "certified-baseline-plus-semantic-r178-r179-deltas"

# path -> certified Sep30 blob, exact R178/R179 working postimage blob
FILES = {
    "engine/include/input/InputState.h": (
        "4097e4969e63b49fe53fb455df2ae1af6e1357a2", "e977db0247b64ffb22cc8755994ac8c6468b5671"),
    "engine/include/interior/ShipEmbodimentSystem.h": (
        "6e3f60a9f858c9c5a4e6d9e80246adaa2565c903", "0cdac8cf1a258523866e37a70be676fffb408ca0"),
    "engine/include/ui/RuntimeControlContextSystem.h": (
        "5c014f5292493648a4f7ef5ffe55fc96668850bc", "cdb2e9282c10bade499984c8fc42eda2a543faf0"),
    "engine/src/ui/RuntimeControlContextSystem.cpp": (
        "486e9bf088377a9365260f6063ddf86ac9d59818", "0bb67835be941f7c24eea3f691246221a9ebb6b4"),
    "engine/src/application/NativeBattlefieldRenderer.cpp": (
        "64a83e3826e844b7830c69ca0f593674d2eecd6c", "dfa54412f1250cbbde43ad5cda8d87cbb3c35b42"),
    "engine/src/application/NativeGameApplication.cpp": (
        "1bedb3e60205173b718ca62374875ae7524b2d93", "2cb3b6987fbe35234f7b8c3801017743818943d7"),
    "engine/src/platform/NativeWindow.cpp": (
        "9122cb84b628b497d713f223c9ce9937f9257124", "ff5e50ec3d1bf6c6d67be024cf53da457825c855"),
}

POSTCONDITIONS = {
    "engine/include/input/InputState.h": (
        "DccCycleTransformSpace", "DccDuplicateSelection", "PilotForward",
        "CharacterMoveForward", "FleetCameraForward", "PlanetaryCommandCycleOverlay"),
    "engine/include/interior/ShipEmbodimentSystem.h": (
        "InteriorAvatarStance", "planarVelocity", "ConfigureLocomotion",
        "StopLocomotion", "SetCertifiedFootPosition"),
    "engine/include/ui/RuntimeControlContextSystem.h": (
        'input/ControlIntentRouterSystem.h', "ControlDomain controlDomain",
        "BuildWithCommandSeat", "FleetCommandSeatSnapshot"),
    "engine/src/ui/RuntimeControlContextSystem.cpp": (
        "ControlDomain::Pilot", "ControlDomain::FirstPerson", "ControlDomain::FleetStrategy",
        "BuildWithCommandSeat", "FleetCommandSeatSystem::CanCommand"),
    "engine/src/application/NativeBattlefieldRenderer.cpp": (
        "cycle PARENT, OBJECT, and VIEW orientations. Scale always uses OBJECT W/L/H.",
        "ShipyardPanelCompositorSystem", "DrawStudioInteriorShell",
        "PLANETARY COMMAND", "PlanetaryIndustrySystem::ProjectionName", "ClaimStateName"),
    "engine/src/application/NativeGameApplication.cpp": (
        "ShipyardOverlayLayoutStore::Load", "ShipInteriorShellTraversalSystem::Move",
        "TryBeginShipyardGizmo", "PlanetaryCommandCycleOverlay",
        "HitTestPlanetaryHex", "AdvancePlanetarySector"),
    "engine/src/platform/NativeWindow.cpp": (
        "DccCommandSearch", "DccWorkspacePrevious", "DccWorkspaceNext",
        "DccCycleTransformSpace", "PlanetaryCommandCycleOverlay"),
}

# Current Studio transform authority lives in the standalone Studio application
# and Shipyard systems, not NativeGameApplication. These files are not rebuilt by
# R182; they are proven before any recovery write and then again by the existing
# six historical Studio static gates.
EXTERNAL_AUTHORITIES = {
    "engine/src/studio/StudioApplication.cpp": (
        "StudioTransformMoveDelta::AssemblyAuthored",
        "StudioTransformMoveDelta::ModelAuthored",
        "TranslateSelectedResolvedParent",
    ),
    "engine/include/ship_editor/ShipyardBuilderSystem.h": (
        "TranslateSelectedResolvedParent",
    ),
    "engine/src/ship_editor/ShipyardBuilderSystem.cpp": (
        "ShipyardBuilderSystem::TranslateSelectedResolvedParent",
    ),
    "engine/src/ship_editor/ShipyardTransformSystem.cpp": (
        "ConstructionTransformBasisSystem::AssemblyLocal(tx.before,true)",
    ),
}

RECEIPT = Path("artifacts/gates/migrations/r182_certified_baseline_recovery/ACTIVE.json")


def run_git(root: Path, *args: str) -> subprocess.CompletedProcess:
    return subprocess.run(["git", "-C", str(root), *args],
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False)


def git_blob_sha(data: bytes) -> str:
    return hashlib.sha1(f"blob {len(data)}\0".encode("ascii") + data).hexdigest()


def normalized_blob_sha(data: bytes) -> str:
    return git_blob_sha(data.replace(b"\r\n", b"\n"))


def git_show(root: Path, commit: str, rel: str) -> bytes:
    r = run_git(root, "show", f"{commit}:{rel}")
    if r.returncode:
        raise RuntimeError(f"git show failed for {commit}:{rel}: {r.stderr.decode(errors='replace').strip()}")
    return r.stdout


def git_blob_at(root: Path, commit: str, rel: str) -> str:
    r = run_git(root, "rev-parse", f"{commit}:{rel}")
    if r.returncode:
        raise RuntimeError(f"git rev-parse failed for {commit}:{rel}: {r.stderr.decode(errors='replace').strip()}")
    return r.stdout.decode().strip()


def replace_exact(text: str, old: str, new: str, label: str) -> str:
    if new in text:
        return text
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one certified baseline anchor, found {count}")
    return text.replace(old, new, 1)


def insert_after(text: str, anchor: str, addition: str, label: str) -> str:
    if addition in text:
        return text
    count = text.count(anchor)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one certified baseline anchor, found {count}")
    return text.replace(anchor, anchor + addition, 1)


def rebuild_input_state(text: str) -> str:
    old = "    DccDuplicateSelection,\n    Count"
    new = '''    DccDuplicateSelection,

    // R178 convergence: context-specific semantic actions are append-only.
    // Legacy physical mappings continue to work through ControlIntentRouterSystem,
    // so serialized historical action indices remain stable.
    PilotForward,
    PilotReverse,
    PilotStrafeLeft,
    PilotStrafeRight,
    PilotThrustUp,
    PilotThrustDown,
    PilotBoost,
    PilotBrake,
    PilotFirePrimary,
    PilotFireSecondary,
    CharacterMoveForward,
    CharacterMoveBackward,
    CharacterMoveLeft,
    CharacterMoveRight,
    CharacterSprint,
    CharacterCrouch,
    CharacterJump,
    CharacterInteract,
    FleetCameraForward,
    FleetCameraBackward,
    FleetCameraLeft,
    FleetCameraRight,
    FleetCameraUp,
    FleetCameraDown,
    FleetQueueModifier,
    FleetAddSelection,
    FleetCommandConfirm,
    FleetCommandCancel,

    // R179 Planetary Command. Appended after R178 so all historical and R178
    // serialized action indices remain stable.
    PlanetaryCommandCycleOverlay,
    Count'''
    return replace_exact(text, old, new, "InputAction append-only semantic tail")


def rebuild_embodiment_header(text: str) -> str:
    text = insert_after(text,
        "enum class ShipEmbodimentMode { CockpitControl, InteriorOnFoot, CutawayInspection, DockedHangar };\n",
        "enum class InteriorAvatarStance { Standing, Crouched };\n",
        "FPS stance enum")
    text = replace_exact(text,
        "    Vector3 localPosition{0.0f, 1.45f, 0.0f};\n    int deck = 0;",
        "    Vector3 localPosition{0.0f, 1.45f, 0.0f};\n    Vector3 planarVelocity{};\n    int deck = 0;",
        "FPS planar velocity")
    old_profile = '''    float moveSpeed = 3.2f;
    float eyeHeightMeters = 1.68f;
    float capsuleHeightMeters = 1.80f;
    float capsuleRadiusMeters = 0.32f;'''
    new_profile = '''    // Compatibility/current values consumed by shell traversal and camera code.
    float moveSpeed = 3.2f;
    float eyeHeightMeters = 1.68f;
    float capsuleHeightMeters = 1.80f;
    float capsuleRadiusMeters = 0.32f;

    // Normalized FPS movement profile. Movement acceleration happens here;
    // collision/occupancy remains owned by ShipInteriorShellTraversalSystem.
    float walkSpeedMetersPerSecond = 3.2f;
    float sprintSpeedMetersPerSecond = 5.6f;
    float crouchSpeedMetersPerSecond = 1.8f;
    float accelerationMetersPerSecond2 = 18.0f;
    float decelerationMetersPerSecond2 = 24.0f;
    float standingEyeHeightMeters = 1.68f;
    float crouchedEyeHeightMeters = 1.05f;
    float standingCapsuleHeightMeters = 1.80f;
    float crouchedCapsuleHeightMeters = 1.18f;
    float stamina01 = 1.0f;
    InteriorAvatarStance stance = InteriorAvatarStance::Standing;
    bool sprinting = false;
    bool grounded = true;'''
    text = replace_exact(text, old_profile, new_profile, "FPS locomotion profile")
    old_move = '''    void SetInspection(bool enabled);
    void Move(float forward, float strafe, double seconds);
    // Runtime shell traversal sets only a certified ship-local foot position.'''
    new_move = '''    void SetInspection(bool enabled);

    // Compatibility API used by the live runtime. It now uses accelerated
    // locomotion instead of teleport-style constant displacement.
    void Move(float forward, float strafe, double seconds);
    void ConfigureLocomotion(bool sprintRequested, bool crouchRequested);
    void StopLocomotion();

    // Runtime shell traversal sets only a certified ship-local foot position.'''
    return replace_exact(text, old_move, new_move, "FPS locomotion API")


def rebuild_context_header(text: str) -> str:
    text = insert_after(text, '#include "fleet/FleetCommandSeatSystem.h"\n',
                        '#include "input/ControlIntentRouterSystem.h"\n',
                        "control intent include")
    return replace_exact(text,
        "    RuntimeViewAuthority viewAuthority=RuntimeViewAuthority::CockpitFirstPerson;\n    bool firstPerson=true;",
        "    RuntimeViewAuthority viewAuthority=RuntimeViewAuthority::CockpitFirstPerson;\n    ControlDomain controlDomain=ControlDomain::Pilot;\n    bool firstPerson=true;",
        "control-domain field")


def rebuild_context_cpp(text: str) -> str:
    replacements = [
        ('if(workspace==SandboxWorkspaceMode::ShipBuilder){c.flightControls=false;',
         'if(workspace==SandboxWorkspaceMode::ShipBuilder){c.controlDomain=ControlDomain::Authoring;c.flightControls=false;', 'authoring control domain'),
        ('if(embodiment==ShipEmbodimentMode::InteriorOnFoot){c.flightControls=false;',
         'if(embodiment==ShipEmbodimentMode::InteriorOnFoot){c.controlDomain=ControlDomain::FirstPerson;c.flightControls=false;', 'FPS control domain'),
        ('if(docking==DockingExperienceStage::Docked||embodiment==ShipEmbodimentMode::DockedHangar){c.flightControls=false;',
         'if(docking==DockingExperienceStage::Docked||embodiment==ShipEmbodimentMode::DockedHangar){c.controlDomain=ControlDomain::DockedService;c.flightControls=false;', 'docked control domain'),
        ('c.modeLabel="STATION HANGAR";', 'c.modeLabel="STATION HANGAR / FIRST PERSON";', 'docked label'),
        ('if(vectorTransit){c.flightControls=false;',
         'if(vectorTransit){c.controlDomain=ControlDomain::Transit;c.flightControls=false;', 'transit control domain'),
        ('    if(strategic){\n        // Until physical seat routing replaces the legacy path, do not allow',
         '    if(strategic){\n        c.controlDomain=ControlDomain::FleetStrategy;\n        // Until physical seat routing replaces the legacy path, do not allow', 'fleet strategy control domain'),
        ('    c.modeLabel="COCKPIT / FIRST PERSON 6DOF";',
         '    c.controlDomain=ControlDomain::Pilot;\n    c.modeLabel="COCKPIT / FIRST PERSON 6DOF";', 'pilot control domain'),
        ('    if(authorized && workspace!=SandboxWorkspaceMode::ShipBuilder && !vectorTransit){\n        context.cameraMode=CameraMode::TacticalFleet;',
         '    if(authorized && workspace!=SandboxWorkspaceMode::ShipBuilder && !vectorTransit){\n        context.controlDomain=ControlDomain::FleetStrategy;\n        context.cameraMode=CameraMode::TacticalFleet;', 'seated fleet strategy domain'),
    ]
    for old,new,label in replacements:
        text = replace_exact(text, old, new, label)
    return text


def rebuild_native_window(text: str) -> str:
    return replace_exact(text,
        '        case VK_F3: _inputState.SetAction(InputAction::DccCommandSearch, down); break;\n        case VK_F6:',
        '        case VK_F3: _inputState.SetAction(InputAction::DccCommandSearch, down); break;\n        case VK_F5: _inputState.SetAction(InputAction::PlanetaryCommandCycleOverlay, down); break;\n        case VK_F6:',
        "Planetary Command F5 binding")


def rebuild_native_app(text: str) -> str:
    text = replace_exact(text,
        '        else {auto pit=_planetProjects.find(planetIndex);lines.push_back(pit==_planetProjects.end()?"Planetary Manufacturing locked: certify survey then deploy tether":"Tether / elevator project registered");}',
        '''        else if(_activePiPlanet==planetIndex){
            PlanetaryIndustrySystem pi;
            const auto hit=_activePiRuntime.industry.hexes.find(_activePiRuntime.selectedHex);
            lines.push_back(std::string("Planetary Command: ")+PlanetaryIndustrySystem::ProjectionName(_activePiRuntime.projection)+" / "+PlanetaryIndustrySystem::OverlayName(_activePiRuntime.overlay));
            if(hit!=_activePiRuntime.industry.hexes.end()){
                lines.push_back("Sector: "+pi.Identity(_activePiRuntime.industry,_activePiRuntime.selectedHex).StableId()+" / "+PlanetaryIndustrySystem::ClaimStateName(hit->second.claimState));
            }
            lines.push_back(_activePiRuntime.commandStatus);
            lines.push_back("LMB select  RMB inspect/projection  F5 overlay  ENTER advance  SHIFT+ENTER deploy");
        }else lines.push_back("Planetary Command initializing...");''',
        "Planetary Command inspector")

    pointer_anchor = '''    if (inGame) {
        bool contextCommandConsumed = false;
        // Pass402: RMB short-click is a universal context request in flight.'''
    pointer_new = '''    if (inGame) {
        bool contextCommandConsumed = false;

        // R179 Planetary Command owns pointer clicks while its workspace is open.
        // LMB selects a stable planetId+axial sector. RMB pins the sector inspector;
        // RMB on empty map space toggles globe/sector projection.
        if(_workspace.Mode()==SandboxWorkspaceMode::PlanetaryManufacturing &&
           _activePiPlanet!=static_cast<std::size_t>(-1)){
            float px=0.0f,py=0.0f;HexCoord hit{};
            if(_window.ConsumePrimaryClick(px,py)){
                if(_playerFacing.HitTestPlanetaryHex(_activePiRuntime,_window.GetWidth(),_window.GetHeight(),px,py,hit)){
                    _activePiRuntime.selectedHex=hit;
                    _activePiRuntime.inspectorPinned=false;
                    const PlanetaryIndustrySystem pi;
                    _activePiRuntime.commandStatus=pi.Identity(_activePiRuntime.industry,hit).StableId();
                }
            }
            if(_window.ConsumeSecondaryClick(px,py)){
                if(_playerFacing.HitTestPlanetaryHex(_activePiRuntime,_window.GetWidth(),_window.GetHeight(),px,py,hit)){
                    _activePiRuntime.selectedHex=hit;
                    _activePiRuntime.inspectorPinned=true;
                    const PlanetaryIndustrySystem pi;
                    _activePiRuntime.commandStatus=std::string("INSPECT ")+pi.Identity(_activePiRuntime.industry,hit).StableId();
                }else{
                    _playerFacing.TogglePlanetaryProjection(_activePiRuntime);
                }
                contextCommandConsumed=true;
            }
        }

        // Pass402: RMB short-click is a universal context request in flight.'''
    text = replace_exact(text, pointer_anchor, pointer_new, "Planetary Command pointer ownership")
    text = replace_exact(text,
        '        if(_workspace.Mode()==SandboxWorkspaceMode::SystemMap && _window.ConsumeSecondaryClick(contextX,contextY)) {',
        '        if(!contextCommandConsumed && _workspace.Mode()==SandboxWorkspaceMode::SystemMap && _window.ConsumeSecondaryClick(contextX,contextY)) {',
        "Planetary Command context-menu isolation")

    toggle_anchor = '''        toggle(InputAction::OpenExploration,SandboxWorkspaceMode::Exploration);
        toggle(InputAction::OpenFleetCorporation,SandboxWorkspaceMode::FleetCorporation);

        if (_workspace.Mode()==SandboxWorkspaceMode::PlanetSurvey ||'''
    toggle_new = '''        toggle(InputAction::OpenExploration,SandboxWorkspaceMode::Exploration);
        toggle(InputAction::OpenFleetCorporation,SandboxWorkspaceMode::FleetCorporation);

        if(_workspace.Mode()==SandboxWorkspaceMode::PlanetaryManufacturing &&
           _activePiPlanet!=static_cast<std::size_t>(-1) &&
           input.WasPressed(InputAction::PlanetaryCommandCycleOverlay)){
            _playerFacing.CyclePlanetaryOverlay(_activePiRuntime,+1);
        }

        if (_workspace.Mode()==SandboxWorkspaceMode::PlanetSurvey ||'''
    text = replace_exact(text, toggle_anchor, toggle_new, "Planetary Command overlay cycle")

    old_accept = '''                const auto& record=_planetSurveys[index];
                if(record.stage>=PlanetSurveyStage::Detailed){
                    PlanetaryIndustrializationSystem industrial;
                    auto& project=_planetProjects[index];
                    if(project.stage==PlanetaryDevelopmentStage::Unsurveyed)industrial.BindSurvey(project,record);
                    else if(project.stage==PlanetaryDevelopmentStage::Surveyed)industrial.SelectAnchor(project,record.elevatorAnchorScore);
                    else if(project.stage==PlanetaryDevelopmentStage::SiteSelected)industrial.DeployTether(project);
                    else if(project.stage==PlanetaryDevelopmentStage::ElevatorOnline)industrial.UnlockManufacturing(project);
                }'''
    new_accept = '''                if(_activePiPlanet!=index){
                    _activePiRuntime=_playerFacing.BuildPlanetIndustry(_sector.planets[index],3,static_cast<std::uint32_t>(index*97+31));
                    _activePiPlanet=index;
                }
                if(_window.IsShiftDown()){
                    const auto kind=_playerFacing.RecommendedIndustryKind(_activePiRuntime);
                    _playerFacing.PlaceIndustry(_activePiRuntime,kind,PowerTechnology::Burner);
                }else{
                    const auto result=_playerFacing.AdvancePlanetarySector(_activePiRuntime);
                    // Keep the older planet-scale industrialization authority synchronized
                    // without making it a second claim/sector owner.
                    const auto& record=_planetSurveys[index];
                    if(result.changed && result.after==PiClaimState::Developed &&
                       record.stage>=PlanetSurveyStage::Detailed){
                        PlanetaryIndustrializationSystem industrial;
                        auto& project=_planetProjects[index];
                        if(project.stage==PlanetaryDevelopmentStage::Unsurveyed)industrial.BindSurvey(project,record);
                    }
                }'''
    return replace_exact(text, old_accept, new_accept, "Planetary Command Enter/deploy semantics")


def rebuild_renderer(text: str) -> str:
    old = '''            const float left=64,top=62,right=w-64,bottom=h-58;FilledRect(left,top,0,right-left,bottom-top,{0.004f,0.016f,0.026f,0.95f});DrawText5x7("PLANETARY INDUSTRY - "+frame.planetIndustryRuntime->planetName,left+22,top+20,1.35f,{.72f,.88f,.92f,.96f});
            const float hx=(left+right)*.5f,hy=(top+bottom)*.52f,hs=28.0f;for(const auto&kv:frame.planetIndustryRuntime->industry.hexes){const auto&hdata=kv.second;const float x=hx+hs*1.5f*hdata.coord.q;const float y=hy+hs*0.8660254f*(2*hdata.coord.r+hdata.coord.q);const bool selected=hdata.coord.q==frame.planetIndustryRuntime->selectedHex.q&&hdata.coord.r==frame.planetIndustryRuntime->selectedHex.r;Ring(x,y,0,hs*.82f,selected?Rgba{1.0f,.68f,.18f,.94f}:Rgba{.22f,.58f,.62f,.60f},selected?2.2f:1.0f,6);if(hdata.surveyed)FilledCircle(x,y,0,3.0f,{.30f,.78f,.58f,.80f},10);}
            std::ostringstream pi;pi<<"TETHER STORAGE "<<static_cast<int>(frame.planetIndustryRuntime->tetherStored)<<"   INSTALLATIONS "<<frame.planetIndustryRuntime->industry.installations.size();DrawText5x7(pi.str(),left+22,bottom-32,.90f,{.52f,.74f,.78f,.82f});'''
    new = '''            const auto& model=*frame.planetIndustryRuntime;
            PlayerFacingIntegrationSystem piUi;
            PlanetaryIndustrySystem piSystem;
            const auto layout=piUi.LayoutPlanetaryCommand(w,h);
            const float left=layout.left,top=layout.top,right=layout.right,bottom=layout.bottom;
            FilledRect(left,top,0,right-left,bottom-top,{0.004f,0.016f,0.026f,0.96f});
            Line(left,top,0,right,top,0,{0.16f,0.62f,0.70f,0.76f},1.5f);
            DrawText5x7("PLANETARY COMMAND - "+model.planetName,left+22,top+18,1.35f,{.72f,.88f,.92f,.96f});
            DrawText5x7(std::string("PROJECTION ")+PlanetaryIndustrySystem::ProjectionName(model.projection)+"   OVERLAY "+PlanetaryIndustrySystem::OverlayName(model.overlay),left+22,top+48,.80f,{.46f,.72f,.76f,.86f});

            if(model.projection==PiProjectionMode::Globe){
                FilledCircle(layout.centerX,layout.centerY,0,layout.globeRadius,{.025f,.075f,.095f,.94f},72);
                Ring(layout.centerX,layout.centerY,0,layout.globeRadius,{.18f,.58f,.68f,.78f},1.6f,72);
            }

            auto overlayColor=[&](const PiHexData& hdata)->Rgba{
                switch(model.overlay){
                    case PiOverlayMode::Resources:{const float v=std::clamp(hdata.resource,0.0f,1.0f);return {.16f+.55f*v,.30f+.54f*v,.18f+.18f*v,.78f};}
                    case PiOverlayMode::Ownership:
                        if(hdata.ownerId==model.ownerId&&(hdata.claimState==PiClaimState::Claimed||hdata.claimState==PiClaimState::Developed))return {.18f,.78f,.90f,.88f};
                        if(hdata.claimState>=PiClaimState::Surveyed)return {.38f,.50f,.54f,.66f};
                        return {.16f,.22f,.25f,.42f};
                    case PiOverlayMode::Industry:{
                        const bool occupied=std::any_of(model.industry.installations.begin(),model.industry.installations.end(),[&](const PiInstallation&i){return i.hex==hdata.coord;});
                        return occupied?Rgba{.96f,.60f,.18f,.90f}:Rgba{.24f,.34f,.38f,.54f};}
                    case PiOverlayMode::Logistics:{
                        bool logistics=false,storage=false;for(const auto&i:model.industry.installations)if(i.hex==hdata.coord){logistics|=i.kind==PiInstallationKind::Logistics;storage|=i.kind==PiInstallationKind::Storage;}
                        return logistics?Rgba{.22f,.72f,.98f,.92f}:storage?Rgba{.36f,.56f,.88f,.82f}:Rgba{.18f,.28f,.38f,.48f};}
                    case PiOverlayMode::Power:{
                        bool power=false;for(const auto&i:model.industry.installations)if(i.hex==hdata.coord)power|=i.kind==PiInstallationKind::Power;
                        return power?Rgba{.42f,.92f,.46f,.92f}:Rgba{.28f,.34f,.24f,.50f};}
                    case PiOverlayMode::Hazard:{const float v=std::clamp(hdata.hazard,0.0f,1.0f);return {.32f+.62f*v,.54f*(1.0f-v),.14f,.82f};}
                }
                return {.22f,.58f,.62f,.60f};
            };

            std::vector<std::pair<float,HexCoord>> drawOrder;
            drawOrder.reserve(model.industry.hexes.size());
            for(const auto&kv:model.industry.hexes){const auto pt=piUi.ProjectPlanetaryHex(model,kv.first,w,h);if(pt.visible)drawOrder.push_back({pt.depth,kv.first});}
            std::sort(drawOrder.begin(),drawOrder.end(),[](const auto&a,const auto&b){return a.first<b.first;});
            for(const auto&entry:drawOrder){
                const auto it=model.industry.hexes.find(entry.second);if(it==model.industry.hexes.end())continue;const auto&hdata=it->second;
                const auto pt=piUi.ProjectPlanetaryHex(model,hdata.coord,w,h);const bool selected=hdata.coord==model.selectedHex;
                const float depthScale=model.projection==PiProjectionMode::Globe?std::clamp(.72f+pt.depth*.28f,.55f,1.0f):1.0f;
                const float radius=layout.hexSize*.72f*depthScale;const auto c=overlayColor(hdata);
                Ring(pt.x,pt.y,0,radius,c,selected?2.5f:1.2f,6);
                if(hdata.claimState==PiClaimState::Surveyed)FilledCircle(pt.x,pt.y,0,2.8f*depthScale,{.52f,.72f,.72f,.78f},10);
                if(hdata.claimState==PiClaimState::Claimed)FilledCircle(pt.x,pt.y,0,4.0f*depthScale,{.18f,.72f,.90f,.86f},10);
                if(hdata.claimState==PiClaimState::Developed)FilledCircle(pt.x,pt.y,0,5.0f*depthScale,{.96f,.64f,.18f,.92f},10);
                const bool installed=std::any_of(model.industry.installations.begin(),model.industry.installations.end(),[&](const PiInstallation&i){return i.hex==hdata.coord;});
                if(installed){Line(pt.x-5,pt.y,0,pt.x+5,pt.y,0,{1.0f,.88f,.48f,.94f},1.6f);Line(pt.x,pt.y-5,0,pt.x,pt.y+5,0,{1.0f,.88f,.48f,.94f},1.6f);}
                if(selected)Ring(pt.x,pt.y,0,radius+5.0f,{1.0f,.82f,.26f,.98f},2.2f,18);
            }

            Line(layout.inspectorLeft-7,top+70,0,layout.inspectorLeft-7,bottom-16,0,{.10f,.34f,.40f,.56f},1.0f);
            float iy=top+82;const float ix=layout.inspectorLeft+8;
            DrawText5x7("SECTOR INSPECTOR",ix,iy,1.0f,{.68f,.88f,.92f,.96f});iy+=30;
            const auto selected=model.industry.hexes.find(model.selectedHex);
            if(selected!=model.industry.hexes.end()){
                const auto&id=selected->second;DrawText5x7(piSystem.Identity(model.industry,model.selectedHex).StableId(),ix,iy,.66f,{.84f,.90f,.90f,.92f});iy+=24;
                DrawText5x7(PlanetaryIndustrySystem::ClaimStateName(id.claimState),ix,iy,.90f,id.claimState>=PiClaimState::Claimed?Rgba{.34f,.86f,.72f,.94f}:Rgba{.62f,.76f,.78f,.88f});iy+=26;
                std::ostringstream row;row<<"RESOURCE "<<static_cast<int>(id.resource*100)<<"%";DrawText5x7(row.str(),ix,iy,.74f,{.60f,.82f,.62f,.88f});iy+=22;row.str("");row.clear();row<<"HAZARD "<<static_cast<int>(id.hazard*100)<<"%";DrawText5x7(row.str(),ix,iy,.74f,id.hazard>.55f?Rgba{.96f,.42f,.22f,.94f}:Rgba{.62f,.78f,.66f,.86f});iy+=22;row.str("");row.clear();row<<"BUILDABILITY "<<static_cast<int>(id.buildability*100)<<"%";DrawText5x7(row.str(),ix,iy,.74f,{.64f,.76f,.80f,.86f});iy+=28;
                const auto frontier=piSystem.ClaimFrontier(model.industry,model.ownerId);row.str("");row.clear();row<<"CLAIM FRONTIER "<<frontier.size();DrawText5x7(row.str(),ix,iy,.70f,{.50f,.70f,.74f,.82f});iy+=26;
                std::size_t localInstall=0;for(const auto&i:model.industry.installations)localInstall+=i.hex==model.selectedHex?1u:0u;row.str("");row.clear();row<<"INSTALLATIONS "<<localInstall;DrawText5x7(row.str(),ix,iy,.70f,{.86f,.70f,.46f,.86f});iy+=30;
            }
            DrawText5x7(model.commandStatus,ix,std::min(iy+10,bottom-112),.66f,{.90f,.76f,.44f,.90f});
            std::ostringstream pi;pi<<"TETHER "<<static_cast<int>(model.tetherStored)<<"   TOTAL INSTALLATIONS "<<model.industry.installations.size();DrawText5x7(pi.str(),left+22,bottom-55,.78f,{.52f,.74f,.78f,.82f});
            DrawText5x7("P CLOSE   LMB SELECT   RMB INSPECT / EMPTY=PROJECTION   F5 OVERLAY   ENTER ADVANCE   SHIFT+ENTER DEPLOY",left+22,bottom-28,.68f,{.46f,.66f,.70f,.78f});'''
    return replace_exact(text, old, new, "Planetary Command renderer")


REBUILDERS = {
    "engine/include/input/InputState.h": rebuild_input_state,
    "engine/include/interior/ShipEmbodimentSystem.h": rebuild_embodiment_header,
    "engine/include/ui/RuntimeControlContextSystem.h": rebuild_context_header,
    "engine/src/ui/RuntimeControlContextSystem.cpp": rebuild_context_cpp,
    "engine/src/application/NativeBattlefieldRenderer.cpp": rebuild_renderer,
    "engine/src/application/NativeGameApplication.cpp": rebuild_native_app,
    "engine/src/platform/NativeWindow.cpp": rebuild_native_window,
}


def validate_text(rel: str, data: bytes) -> None:
    text = data.decode("utf-8", errors="strict")
    missing = [token for token in POSTCONDITIONS[rel] if token not in text]
    if missing:
        raise RuntimeError(f"rebuilt {rel} is missing authority token(s): {', '.join(missing)}")
    if "<<<<<<<" in text or "=======" in text or ">>>>>>>" in text:
        raise RuntimeError(f"rebuilt {rel} contains conflict markers")


def validate_external_authorities(root: Path) -> None:
    for rel, tokens in EXTERNAL_AUTHORITIES.items():
        path = root / rel
        if not path.is_file():
            raise RuntimeError(f"current Studio authority file is missing: {rel}")
        text = path.read_text(encoding="utf-8", errors="strict")
        missing = [token for token in tokens if token not in text]
        if missing:
            raise RuntimeError(
                f"current Studio authority drift in {rel}; missing: {', '.join(missing)}")


def rebuild_file(root: Path, rel: str, baseline_blob: str, post_blob: str) -> bytes:
    path = root / rel
    if not path.is_file():
        raise RuntimeError(f"affected source file is missing: {rel}")

    actual = git_blob_at(root, BASELINE_COMMIT, rel)
    if actual != baseline_blob:
        raise RuntimeError(f"certified baseline blob drift for {rel}: expected {baseline_blob}, got {actual}")

    current = path.read_bytes()
    if post_blob not in (git_blob_sha(current), normalized_blob_sha(current)):
        raise RuntimeError(
            f"unknown working postimage for {rel}: raw={git_blob_sha(current)} lf={normalized_blob_sha(current)}; "
            f"expected exact R178/R179 postimage {post_blob}")

    baseline = git_show(root, BASELINE_COMMIT, rel).replace(b"\r\n", b"\n").decode("utf-8", errors="strict")
    rebuilt = REBUILDERS[rel](baseline).encode("utf-8")
    validate_text(rel, rebuilt)
    return rebuilt


def receipt_matches(root: Path) -> bool:
    """Return True when the one-time R182 recovery has completed and the
    current tree remains a semantically valid descendant.

    The receipt proves that the transactional certified-baseline reconstruction
    already ran. Later governed milestones are allowed to evolve any of the seven
    recovered files, so their current SHA-256 values must not be frozen forever to
    the original R182 after-images. Descendants are accepted only while every R182
    gameplay postcondition and every external Studio authority still validates.

    Unknown/partial trees *without* this valid receipt continue through the exact
    historical-postimage classifier below and therefore still fail closed.
    """
    p = root / RECEIPT
    if not p.is_file():
        return False
    try:
        record = json.loads(p.read_text(encoding="utf-8"))
        if record.get("baselineCommit") != BASELINE_COMMIT:
            return False
        if record.get("strategy") != "certified-8484a08-baseline-plus-r178-r179-forward-port":
            return False
        rows = {row["path"]: row for row in record.get("files", [])}
        for rel in FILES:
            row = rows.get(rel)
            path = root / rel
            if not row or not path.is_file():
                return False
            # The recorded afterSha256 remains provenance for the original R182
            # transaction, not a permanent lock on future governed source edits.
            if not isinstance(row.get("afterSha256"), str) or len(row["afterSha256"]) != 64:
                return False
            validate_text(rel, path.read_bytes())
        validate_external_authorities(root)
        return True
    except Exception:
        return False


def repair_known_overlay_regression(root: Path, *, dry_run: bool = False) -> bool:
    root = Path(root).resolve()
    validate_external_authorities(root)
    if receipt_matches(root):
        return False

    states = {}
    for rel, (_, post_blob) in FILES.items():
        path = root / rel
        data = path.read_bytes() if path.is_file() else b""
        states[rel] = bool(data) and post_blob in (git_blob_sha(data), normalized_blob_sha(data))

    matched = [rel for rel, yes in states.items() if yes]
    if not matched:
        # A clean already-reconciled or otherwise unaffected tree must not be mutated.
        return False
    if len(matched) != len(FILES):
        missing = [rel for rel, yes in states.items() if not yes]
        raise RuntimeError(
            "partial R178/R179 overlay regression detected; refusing mixed-state semantic repair. "
            f"Known postimages: {len(matched)}/{len(FILES)}; nonmatching: {', '.join(missing)}")

    rebuilt = {}
    for rel, (baseline_blob, post_blob) in FILES.items():
        rebuilt[rel] = rebuild_file(root, rel, baseline_blob, post_blob)

    if dry_run:
        return True

    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    backup = root / "artifacts/gates/migrations/r182_certified_baseline_recovery" / stamp
    backup.mkdir(parents=True, exist_ok=False)
    rows = []
    for rel in FILES:
        src = root / rel
        dst = backup / rel
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src, dst)
        rows.append({
            "path": rel,
            "beforeSha256": hashlib.sha256(src.read_bytes()).hexdigest(),
            "afterSha256": hashlib.sha256(rebuilt[rel]).hexdigest(),
        })

    written = []
    try:
        for rel, data in rebuilt.items():
            path = root / rel
            tmp = path.with_name(path.name + ".r182.tmp")
            tmp.write_bytes(data)
            os.replace(tmp, path)
            written.append(rel)
        validate_external_authorities(root)
        record = {
            "schema": "subspace.r182-certified-baseline-recovery.v1",
            "timestampUtc": stamp,
            "baselineCommit": BASELINE_COMMIT,
            "strategy": "certified-8484a08-baseline-plus-r178-r179-forward-port",
            "files": rows,
        }
        p = root / RECEIPT
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
    except Exception:
        for rel in written:
            src = backup / rel
            if src.is_file():
                shutil.copy2(src, root / rel)
        raise
    return True


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", required=True)
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()
    try:
        changed = repair_known_overlay_regression(Path(args.root), dry_run=args.dry_run)
        if args.dry_run and changed:
            print("R182 certified-baseline recovery: REPAIR REQUIRED/VALID")
        elif changed:
            print("R182 certified-baseline recovery: REPAIRED")
        else:
            print("R182 certified-baseline recovery: NO ACTION")
        return 0
    except Exception as exc:
        print(f"R182 CERTIFIED BASELINE RECOVERY BLOCKED: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
