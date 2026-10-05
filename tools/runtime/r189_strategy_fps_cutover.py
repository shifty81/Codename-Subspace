#!/usr/bin/env python3
"""R189 guarded strategy-first runtime + real FPS cutover."""
from __future__ import annotations
import argparse, difflib, hashlib, json, shutil, subprocess, sys, time
from pathlib import Path

BASELINE = "22aa9510b77e94df3a8d6062505b1bde00f78d28"
TARGETS = (
    "engine/include/application/NativeGameApplication.h",
    "engine/src/application/NativeGameApplication.cpp",
    "engine/include/application/NativeBattlefieldRenderer.h",
    "engine/src/application/NativeBattlefieldRenderer.cpp",
    "engine/CMakeLists.txt",
)
POST = {
    TARGETS[0]: (
        "FleetStrategyControlSystem _fleetStrategy",
        "PlayerController _playerController",
        "RuntimeControlContext _controlContext",
        "bool _fleetStrategyActive = true",
        "void RefreshRuntimeControlContext()",
        "void UpdateFleetStrategyControl()",
        "bool BoardPlayerShipFromStrategy()",
    ),
    TARGETS[1]: (
        "R189_STRATEGY_FIRST_BOOT",
        "R189_CONTEXTUAL_INPUT_AUTHORITY",
        "R189_REAL_FPS_POSE",
        "BoardPlayerShipFromStrategy",
        "_fleetStrategy.TickCamera",
        "ControlDomain::FleetStrategy",
        "FirstPersonViewSystem::BuildOnFootLocal",
    ),
    TARGETS[2]: (
        "bool fleetStrategyActive = false",
        "bool hasFirstPersonPose = false",
        "FirstPersonViewPose firstPersonPose",
    ),
    TARGETS[3]: (
        "R189_REAL_FPS_PROJECTION",
        "frame.hasFirstPersonPose",
        "FLEET COMMAND",
        "COCKPIT / PILOTING",
    ),
    TARGETS[4]: (
        "subspace_r189_strategy_fps_cutover_tests",
        "SubspaceR189StrategyFpsCutoverTests",
    ),
}

# Exact one-off descendant created by the temporary chat repair issued before
# the R189 materializer defect was isolated.  This is NOT a general dirty-tree
# bypass: only this byte-exact replacement is normalized, and only when doing
# so reproduces the certified 22aa951 file exactly.
TEMP_UNDOCK_BASELINE = """                if(_dockingSystem.RequestUndock(_docking)) {
                    _workspace.Close();
                    _embodiment=ShipEmbodimentSystem{};
                }
"""
TEMP_UNDOCK_DESCENDANT = """                if(_dockingSystem.RequestUndock(_docking)) {
                    _workspace.Close();
                    // R189 strategy-first runtime authority: undocking returns to
                    // remote strategic command. WASD therefore belongs to the
                    // strategy camera until the player explicitly takes the helm.
                    _strategicFlight.SetMode(FlightControlMode::Strategic);
                    if(auto* controls=_engine.GetPlayerControlSystem())controls->ClearControlledShip();
                    _embodiment=ShipEmbodimentSystem{};
                }
"""

def _cmake_format_canonical(text: str) -> str:
    # CMake is the only target known to be rewritten by multiple historical
    # tooling passes.  Treat encoding/newline/trailing-space drift as formatting
    # only, but preserve every semantic character and line.
    if text.startswith('\ufeff'):
        text = text[1:]
    text = text.replace('\r\n', '\n').replace('\r', '\n')
    lines = [line.rstrip(' \t') for line in text.split('\n')]
    while lines and lines[-1] == '':
        lines.pop()
    return '\n'.join(lines) + '\n'

def _preimage_diagnostic(rel: str, current: str, baseline: str) -> str:
    current_hash = hashlib.sha256(current.encode('utf-8')).hexdigest()
    baseline_hash = hashlib.sha256(baseline.encode('utf-8')).hexdigest()
    diff = list(difflib.unified_diff(
        baseline.splitlines(), current.splitlines(),
        fromfile=f'{rel}@{BASELINE}', tofile=f'{rel}@working',
        n=3, lineterm=''))
    preview = '\n'.join(diff[:28]) if diff else '<no line diff; encoding/newline-only drift>'
    return (f'R189 refuses uncommitted/unknown preimage for {rel}; '
            f'baseline_sha256={baseline_hash} current_sha256={current_hash}\n{preview}')

# The 2026-10-02 18:45 governed local rollup legitimately contains Studio
# CMake registrations newer than commit 22aa951 while HEAD intentionally remains
# pinned at that commit until the next green checkpoint. Preserve that exact
# descendant instead of erasing later certified Studio test/gate registrations.
R189_KNOWN_CMAKE_GOVERNED_DESCENDANT_SHA256 = "b8d94527cc1d2a9a641dac5526d8ac60375ad829ff4fe7b36b2ad1fec3ceba6c"
R189_KNOWN_CMAKE_MARKERS = (
    "SubspaceStudioR82R2GateRepairSourceGate",
    "SubspaceStudioR82R4FourFailureRecoverySourceGate",
    "subspace_studio_duplicate_snap_r73_tests",
)

# R191 intentionally advances the application shell beyond several literal R189
# marker strings while preserving the R189 capabilities. Full Gate still runs
# this historical materializer, so recognize only the byte-exact certified R191
# descendants. This is deliberately fail-closed: marker presence without the
# exact hash is not sufficient.
R189_KNOWN_FORWARD_DESCENDANTS = {
    TARGETS[0]: (
        "3188561d6dca15d26190fe8b27b5499b245915813638a1ae3a2f5d3890d97e6b",
        (
            "GameplayControlMode _gameplayMode = GameplayControlMode::OnFoot",
            "void SetGameplayControlMode(GameplayControlMode mode)",
            "FleetStrategyControlSystem _fleetStrategy",
            "PlayerController _playerController",
            "RuntimeControlContext _controlContext",
            "void RefreshRuntimeControlContext()",
            "bool BoardPlayerShipFromStrategy()",
        ),
    ),
    TARGETS[1]: (
        "126b145f2e9165bf11bee28c367f09552efb4dba7376c2a2db5ad30af3e53239",
        (
            "R191_PRIMARY_FPS_BOOT",
            "GameplayControlMode::OnFoot",
            "GameplayControlMode::Pilot",
            "GameplayControlMode::FleetCommand",
            "BoardPlayerShipFromStrategy",
            "_fleetStrategy.TickCamera",
            "ControlDomain::FleetStrategy",
            "FirstPersonViewSystem::BuildOnFootLocal",
        ),
    ),
}


# R192 advances the same two application targets with the physical starter
# interior interaction slice. Preserve the certified R191 hashes above as valid
# historical descendants while adding a second byte-exact forward lineage.
R189_KNOWN_R192_FORWARD_DESCENDANTS = {
    TARGETS[0]: (
        "a8750adffd0ad3af0ce2d85c0b9df6bdbc5050ac80b0353445adf43bf9789c9a",
        (
            "StarterInteriorScene _starterInteriorScene",
            "void UpdateInteriorInteractionFocus()",
            "bool ExecuteInteriorInteraction()",
            "GameplayControlMode _gameplayMode = GameplayControlMode::OnFoot",
            "bool BoardPlayerShipFromStrategy()",
        ),
    ),
    TARGETS[1]: (
        "c939bb3b81adafdb017ea67df98f17bff68723bd20a9f18c92009ec2884fe55e",
        (
            "R191_PRIMARY_FPS_BOOT",
            "StarterShipInteriorSceneSystem::ResolveFixtureCollision",
            "ExecuteInteriorInteraction()",
            "GameplayControlMode::Pilot",
            "GameplayControlMode::FleetCommand",
            "_fleetStrategy.TickCamera",
            "FirstPersonViewSystem::BuildOnFootLocal",
        ),
    ),
}

def _sha256_text(text: str) -> str:
    return hashlib.sha256(text.encode('utf-8')).hexdigest()

def _is_known_forward_descendant(rel: str, text: str) -> bool:
    for table in (R189_KNOWN_FORWARD_DESCENDANTS, R189_KNOWN_R192_FORWARD_DESCENDANTS):
        spec = table.get(rel)
        if spec is None:
            continue
        expected_hash, markers = spec
        if _sha256_text(text) == expected_hash and all(marker in text for marker in markers):
            return True
    return False

def _satisfies_r189_target(rel: str, text: str) -> bool:
    tokens = POST[rel]
    return all(token in text for token in tokens) or _is_known_forward_descendant(rel, text)

def normalize_known_preimage(rel: str, current: str, baseline: str) -> str:
    if current == baseline:
        return baseline
    if rel == TARGETS[1] and current.count(TEMP_UNDOCK_DESCENDANT) == 1:
        normalized = current.replace(TEMP_UNDOCK_DESCENDANT, TEMP_UNDOCK_BASELINE, 1)
        if normalized == baseline:
            print('INFO: R189 recognized exact temporary undock-repair descendant; normalizing to certified 22aa951 preimage')
            return baseline
    if rel == TARGETS[4]:
        if _cmake_format_canonical(current) == _cmake_format_canonical(baseline):
            print('INFO: R189 recognized formatting-only engine/CMakeLists.txt drift; normalizing to certified 22aa951 preimage')
            return baseline
        if (_sha256_text(current) == R189_KNOWN_CMAKE_GOVERNED_DESCENDANT_SHA256 and
            all(marker in current for marker in R189_KNOWN_CMAKE_MARKERS)):
            print('INFO: R189 recognized exact governed 2026-10-02 CMake descendant; preserving Studio registrations and layering R189 on top')
            return current
    raise RuntimeError(_preimage_diagnostic(rel, current, baseline))

def run(root: Path, *args: str) -> str:
    p = subprocess.run(args, cwd=str(root), text=True, capture_output=True, check=False)
    if p.returncode:
        raise RuntimeError(f"{' '.join(args)} failed ({p.returncode}): {(p.stderr+p.stdout).strip()}")
    return p.stdout

def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one certified anchor, found {count}")
    return text.replace(old, new, 1)

def transform_header(text: str) -> str:
    text = replace_once(text,
        '#include "fleet/FleetCaptainAiSystem.h"\n',
        '#include "fleet/FleetCaptainAiSystem.h"\n#include "fleet/FleetStrategyControlSystem.h"\n#include "runtime/PlayerController.h"\n#include "rendering/FirstPersonViewSystem.h"\n#include "ui/RuntimeControlContextSystem.h"\n',
        'application header includes')
    text = replace_once(text,
        '    void UpdateFleetCaptains();\n',
        '    void UpdateFleetCaptains();\n    void RefreshRuntimeControlContext();\n    void UpdateFleetStrategyControl();\n    bool EnterFleetStrategy();\n    bool BoardPlayerShipFromStrategy();\n',
        'application header methods')
    text = replace_once(text,
        '    StrategicFlightSystem _strategicFlight;\n',
        '    StrategicFlightSystem _strategicFlight;\n    FleetStrategyControlSystem _fleetStrategy;\n    PlayerController _playerController;\n    RuntimeControlContext _controlContext{};\n    bool _fleetStrategyActive = true;\n',
        'application header state')
    return text

def transform_renderer_header(text: str) -> str:
    text = replace_once(text,
        '#include "interior/ShipEmbodimentSystem.h"\n',
        '#include "interior/ShipEmbodimentSystem.h"\n#include "rendering/FirstPersonViewSystem.h"\n',
        'renderer header FPS include')
    return replace_once(text,
        '    bool strategicFlightMode = false;\n',
        '    bool strategicFlightMode = false;\n    bool fleetStrategyActive = false;\n    bool hasFirstPersonPose = false;\n    FirstPersonViewPose firstPersonPose{};\n',
        'renderer frame FPS fields')

def transform_app_cpp(text: str) -> str:
    old = """    if (auto* controls=_engine.GetPlayerControlSystem()) controls->SetControlledShip(_playerEntity);
    _embodiment = ShipEmbodimentSystem{};
    _docking = DockingExperienceState{};
    _miningLoop.Initialize(_sector);
    _engine.GetStrategicCamera().SetZoomLimits(0.12f,28.0f);
"""
    new = """    // R189_STRATEGY_FIRST_BOOT: command view owns startup; manual thrust requires the physical helm.
    if (auto* controls=_engine.GetPlayerControlSystem()) controls->ClearControlledShip();
    _embodiment = ShipEmbodimentSystem{};
    _docking = DockingExperienceState{};
    _fleetStrategyActive = true;
    _strategicFlight.SetMode(FlightControlMode::Manual);
    _playerController.SetActorId(_playerEntity);
    _playerController.SetControlledEntity(std::to_string(_playerEntity));
    _miningLoop.Initialize(_sector);
    _engine.GetStrategicCamera().SetZoomLimits(0.12f,28.0f);
"""
    text = replace_once(text, old, new, 'strategy-first bootstrap')

    old = """    _engine.GetStrategicCamera().SetZoom(0.56f);
    _engine.GetStrategicCamera().SetVisualTilt(0.46f);
    _engine.GetStrategicCamera().SetVisualHeight(0.68f);
    _engine.GetStrategicCamera().SetVelocityLookAhead(0.24f);
"""
    new = old + """    if(auto* playerPhysics=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity)){
        _fleetStrategy.Focus(playerPhysics->position);
        _engine.GetStrategicCamera().SetCenter(playerPhysics->position);
        _engine.GetStrategicCamera().SetTargetCenter(playerPhysics->position);
    }
    RefreshRuntimeControlContext();
"""
    text = replace_once(text, old, new, 'strategy camera initialization')

    text = replace_once(text,
        '    if(!inGame)return; // frontend must be the sole pointer-input owner outside gameplay\n',
        '    if(!inGame)return; // frontend must be the sole pointer-input owner outside gameplay\n    RefreshRuntimeControlContext(); // R189_CONTEXTUAL_INPUT_AUTHORITY\n',
        'global context refresh')

    old = """        // Pass383 runtime refinement: Tab is the explicit Manual/Strategic
        // flight-mode switch. Boost moved to Shift at the native window layer.
        // Strategic mode currently preserves direct keys as an immediate
        // override while the contextual/autopilot order layer remains active.
        if(input.WasPressed(InputAction::ToggleFlightMode) && _docking.stage==DockingExperienceStage::Undocked && !_vectorTravelSystem.InTransit(_vectorTravel)){
            _strategicFlight.ToggleMode();
            auto& camera=_engine.GetStrategicCamera();
            if(_strategicFlight.Mode()==FlightControlMode::Strategic){camera.SetTargetZoom(.43f);camera.SetVisualTilt(.50f);}
            else {camera.SetTargetZoom(.56f);camera.SetVisualTilt(.46f);}
        }
"""
    new = """        // R189: Tab cannot globally change WASD into ship thrust. On foot it is a temporary Remote Fleet Command bridge.
        if(input.WasPressed(InputAction::ToggleFlightMode) && _docking.stage==DockingExperienceStage::Undocked && !_vectorTravelSystem.InTransit(_vectorTravel)){
            if(_embodiment.IsOnFoot()){
                if(_fleetStrategyActive){_fleetStrategyActive=false;RefreshRuntimeControlContext();}
                else EnterFleetStrategy();
            }else if(!_fleetStrategyActive && _controlContext.controlDomain==ControlDomain::Pilot){
                _strategicFlight.ToggleMode();
            }
        }
"""
    text = replace_once(text, old, new, 'Tab contextual ownership')

    start = text.index('    // Pass518: cockpit/interior transition is explicit.')
    end = text.index('\n    if(input.IsDown(InputAction::ToggleShipInspection)', start)
    new = """    // R189: I is the explicit strategy -> FPS -> physical helm embodiment bridge.
    if(input.WasPressed(InputAction::ToggleInterior) && !_vectorTravelSystem.InTransit(_vectorTravel) && !_workspace.IsOverlayOpen()){
        if(_docking.stage==DockingExperienceStage::Docked){
            if(_embodiment.IsOnFoot()){_embodiment.EnterDockedHangar(_playerEntity);_workspace.Open(SandboxWorkspaceMode::HangarFitting);}
            else if(_embodiment.Mode()==ShipEmbodimentMode::DockedHangar){if(_embodiment.BoardInterior(_playerEntity))_workspace.Close();}
        }else if(_docking.stage==DockingExperienceStage::Undocked){
            if(_fleetStrategyActive){BoardPlayerShipFromStrategy();}
            else if(_embodiment.IsPiloting()){
                if(_embodiment.ExitCockpit(_playerEntity)){if(auto* controls=_engine.GetPlayerControlSystem())controls->ClearControlledShip();if(auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity)){player->velocity={};player->angularVelocity={};}RefreshRuntimeControlContext();}
            }else if(_embodiment.IsOnFoot()&&_embodiment.TakeControls()){
                _fleetStrategyActive=false;if(auto* controls=_engine.GetPlayerControlSystem())controls->SetControlledShip(_playerEntity);RestoreGameplayCameraLimits();RefreshRuntimeControlContext();
            }
        }
    }
"""
    text = text[:start] + new + text[end:]

    text = replace_once(text,
        '        } else activeCamera.ZoomBy(wheel);\n',
        '        } else if(_fleetStrategyActive&&_workspace.Mode()==SandboxWorkspaceMode::Flight)_fleetStrategy.Zoom(wheel);\n        else if(_controlContext.controlDomain!=ControlDomain::FirstPerson)activeCamera.ZoomBy(wheel);\n',
        'contextual wheel')

    old = """        } else {
            const float currentPitch=activeCamera.HasElevationOverride()?activeCamera.GetElevationOverrideDegrees():34.0f;
            activeCamera.SetVisualYawDegrees(activeCamera.GetVisualYawDegrees()+orbitX*0.30f);
            activeCamera.SetElevationOverrideDegrees(std::clamp(currentPitch-orbitY*0.28f,-89.0f,89.0f));
        }
"""
    new = """        } else if(_controlContext.controlDomain==ControlDomain::FirstPerson){
            _embodiment.Look(orbitX*0.0035f,-orbitY*0.0035f);
        } else if(_fleetStrategyActive&&_workspace.Mode()==SandboxWorkspaceMode::Flight){
            _fleetStrategy.Orbit(orbitX*0.30f,-orbitY*0.28f);
        } else {
            const float currentPitch=activeCamera.HasElevationOverride()?activeCamera.GetElevationOverrideDegrees():34.0f;
            activeCamera.SetVisualYawDegrees(activeCamera.GetVisualYawDegrees()+orbitX*0.30f);
            activeCamera.SetElevationOverrideDegrees(std::clamp(currentPitch-orbitY*0.28f,-89.0f,89.0f));
        }
"""
    text = replace_once(text, old, new, 'contextual orbit/look')

    old = """        } else if(_workspace.Mode()!=SandboxWorkspaceMode::GalaxyMap){
            const float panScale=0.022f/std::max(.35f,activeCamera.GetZoom());
            activeCamera.PanViewRelative(panX*panScale,panY*panScale);
        }
"""
    new = """        } else if(_fleetStrategyActive&&_workspace.Mode()==SandboxWorkspaceMode::Flight){
            auto& fc=_fleetStrategy.Camera();const float yaw=fc.yawDegrees*0.01745329251994329577f;
            const Vector3 forward{-std::sin(yaw),std::cos(yaw),0.0f},right{std::cos(yaw),std::sin(yaw),0.0f};
            const float scale=0.010f*std::clamp(fc.distance/120.0f,0.25f,6.0f);fc.focus=fc.focus+right*(-panX*scale)+forward*(panY*scale);
        } else if(_workspace.Mode()!=SandboxWorkspaceMode::GalaxyMap&&_controlContext.controlDomain!=ControlDomain::FirstPerson){
            const float panScale=0.022f/std::max(.35f,activeCamera.GetZoom());activeCamera.PanViewRelative(panX*panScale,panY*panScale);
        }
"""
    text = replace_once(text, old, new, 'fleet pointer pan')

    a = text.index('void NativeGameApplication::UpdateEmbodiment()')
    b = text.index('\nvoid NativeGameApplication::UpdateDocking()', a)
    new = """void NativeGameApplication::RefreshRuntimeControlContext()
{
    RuntimeControlContextSystem contexts;_controlContext=contexts.Build(_workspace.Mode(),_embodiment.Mode(),_docking.stage,_vectorTravelSystem.InTransit(_vectorTravel),_fleetStrategyActive);
    _playerController.SetActorId(_playerEntity);if(_playerEntity!=InvalidEntityId)_playerController.SetControlledEntity(std::to_string(_playerEntity));
    _playerController.SetControlDomain(_controlContext.controlDomain);_playerController.RouteInput(_engine.GetInputState());
}

bool NativeGameApplication::EnterFleetStrategy()
{
    if(_workspace.Mode()!=SandboxWorkspaceMode::Flight||_vectorTravelSystem.InTransit(_vectorTravel))return false;_fleetStrategyActive=true;
    if(auto* controls=_engine.GetPlayerControlSystem())controls->ClearControlledShip();if(auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity))_fleetStrategy.Focus(player->position);
    RefreshRuntimeControlContext();return _controlContext.controlDomain==ControlDomain::FleetStrategy;
}

bool NativeGameApplication::BoardPlayerShipFromStrategy()
{
    if(!_fleetStrategyActive||_docking.stage!=DockingExperienceStage::Undocked||!_playerInteriorLayout.shell.ready)return false;
    if(_embodiment.IsPiloting()){if(!_embodiment.ExitCockpit(_playerEntity))return false;}else if(!_embodiment.IsOnFoot())return false;
    _fleetStrategyActive=false;if(auto* controls=_engine.GetPlayerControlSystem())controls->ClearControlledShip();
    if(auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity)){player->velocity={};player->angularVelocity={};}
    RefreshRuntimeControlContext();return _controlContext.controlDomain==ControlDomain::FirstPerson;
}

void NativeGameApplication::UpdateFleetStrategyControl()
{
    if(!_fleetStrategyActive||_workspace.Mode()!=SandboxWorkspaceMode::Flight||_vectorTravelSystem.InTransit(_vectorTravel))return;RefreshRuntimeControlContext();
    if(_controlContext.controlDomain!=ControlDomain::FleetStrategy)return;if(auto* controls=_engine.GetPlayerControlSystem())controls->ClearControlledShip();
    const float dt=std::max(0.001f,_engine.GetLastDeltaTime());_fleetStrategy.TickCamera(_playerController.Intent(),dt);const auto& fc=_fleetStrategy.Camera();auto& camera=_engine.GetStrategicCamera();
    camera.SetTargetCenter(fc.focus);camera.SetVisualYawDegrees(fc.yawDegrees);camera.SetElevationOverrideDegrees(fc.tiltDegrees);camera.SetTargetZoom(std::clamp(120.0f/std::max(12.0f,fc.distance),0.12f,8.0f));camera.Update(dt);
}

void NativeGameApplication::UpdateEmbodiment()
{
    if(!_embodiment.IsOnFoot()||_fleetStrategyActive)return;auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity);if(!player)return;RefreshRuntimeControlContext();
    if(_controlContext.controlDomain!=ControlDomain::FirstPerson&&_controlContext.controlDomain!=ControlDomain::DockedService)return;if(auto* controls=_engine.GetPlayerControlSystem())controls->ClearControlledShip();player->velocity={};player->angularVelocity={};
    const auto& intent=_playerController.Intent();_embodiment.ConfigureLocomotion(intent.sprint,intent.crouch);if(!_playerInteriorLayout.shell.ready)return;
    const auto original=_embodiment.Avatar().localPosition;const float radius=_embodiment.Avatar().capsuleRadiusMeters,height=_embodiment.Avatar().capsuleHeightMeters;Vector3 certified=original;
    if(!ShipInteriorShellTraversalSystem::Spawn(_playerInteriorLayout.carve,_playerInteriorLayout.shell,radius,height,certified))return;_embodiment.SetCertifiedFootPosition(certified);_embodiment.SetTraversalBounds({{}, {}, false});
    _embodiment.Move(intent.forward,intent.right,std::max(0.001f,_engine.GetLastDeltaTime()));const auto intended=_embodiment.Avatar().localPosition;
    _embodiment.SetCertifiedFootPosition(ShipInteriorShellTraversalSystem::Move(_playerInteriorLayout.carve,_playerInteriorLayout.shell,certified,intended-certified,radius,height));
}
"""
    text = text[:a] + new + text[b:]

    a = text.index('void NativeGameApplication::UpdateCameraAndSelection()')
    brace = text.index('{', a)
    depth = 0
    end = brace
    for i in range(brace, len(text)):
        if text[i] == '{': depth += 1
        elif text[i] == '}':
            depth -= 1
            if depth == 0:
                end = i + 1
                break
    new = """void NativeGameApplication::UpdateCameraAndSelection()
{
    UpdateVectorCamera();auto* player=_engine.GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity);if(!player)return;auto& camera=_engine.GetStrategicCamera();
    if(_workspace.Mode()==SandboxWorkspaceMode::ShipBuilder){camera.Update(_engine.GetLastDeltaTime());return;}RefreshRuntimeControlContext();
    if(_fleetStrategyActive&&_controlContext.controlDomain==ControlDomain::FleetStrategy){UpdateFleetStrategyControl();return;}
    EnvironmentPresentationSystem environment;const auto motion=environment.MotionFor(_controlContext.cameraMode,player->velocity.length());camera.SetFollowSmoothness(motion.followSmoothness);camera.SetVelocityLookAhead(motion.velocityLookAhead);
    if(_embodiment.IsOnFoot()){const auto& avatar=_embodiment.Avatar();const float yaw=player->rotation.z,cy=std::cos(yaw),sy=std::sin(yaw);const Vector3 avatarWorld=player->position+Vector3{(avatar.localPosition.x*cy-avatar.localPosition.y*sy)*.72f,(avatar.localPosition.x*sy+avatar.localPosition.y*cy)*.72f,avatar.localPosition.z*.72f};camera.SetCenter(avatarWorld);camera.SetTargetCenter(avatarWorld);}
    else camera.FollowTarget(player->position,player->velocity,_engine.GetLastDeltaTime());
}
"""
    text = text[:a] + new + text[end:]

    old = """    }else if(_docking.stage==DockingExperienceStage::Undocked){
        if(auto* controls=_engine.GetPlayerControlSystem())controls->SetControlledShip(_playerEntity);
        _embodiment=ShipEmbodimentSystem{};
    }
"""
    text = replace_once(text, old,
        """    }else if(_docking.stage==DockingExperienceStage::Undocked){
        _embodiment=ShipEmbodimentSystem{};EnterFleetStrategy();
    }
""", 'undock strategy authority')

    text = replace_once(text,
        '            UpdateEmbodiment();\n            UpdateCameraAndSelection();\n',
        '            UpdateEmbodiment();\n            UpdateFleetStrategyControl();\n            UpdateCameraAndSelection();\n',
        'frame strategy tick')

    text = replace_once(text,
        '    f.strategicFlightMode=_strategicFlight.Mode()==FlightControlMode::Strategic;\n',
        '    f.strategicFlightMode=_strategicFlight.Mode()==FlightControlMode::Strategic;\n    f.fleetStrategyActive=_fleetStrategyActive;\n',
        'render strategy flag')

    old = '    const auto* hudPlayer=const_cast<Engine&>(_engine).GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity);\n    const float hudSpeed=hudPlayer?hudPlayer->velocity.length():0.0f;\n'
    new = '    const auto* hudPlayer=const_cast<Engine&>(_engine).GetEntityManager().GetComponent<PhysicsComponent>(_playerEntity);\n    if(hudPlayer&&_embodiment.IsOnFoot()&&!_fleetStrategyActive){const auto local=FirstPersonViewSystem::BuildOnFootLocal(_embodiment.Avatar());const float yaw=hudPlayer->rotation.z,cy=std::cos(yaw),sy=std::sin(yaw);const auto rotate=[&](const Vector3& v){return Vector3{v.x*cy-v.y*sy,v.x*sy+v.y*cy,v.z};};f.firstPersonPose=local;f.firstPersonPose.position=hudPlayer->position+rotate(local.position)*.72f;f.firstPersonPose.forward=rotate(local.forward).normalized();f.firstPersonPose.right=rotate(local.right).normalized();f.firstPersonPose.up=rotate(local.up).normalized();f.hasFirstPersonPose=true;} // R189_REAL_FPS_POSE\n    const float hudSpeed=hudPlayer?hudPlayer->velocity.length():0.0f;\n'
    text = replace_once(text, old, new, 'FPS render pose')

    text = replace_once(text,
        '    f.commandRail=_standaloneShipyard?CommandRailRuntimeModel{}:_playerFacing.BuildCommandRail(_workspace.Mode());\n',
        '    f.commandRail=(_standaloneShipyard||(_embodiment.IsOnFoot()&&!_fleetStrategyActive))?CommandRailRuntimeModel{}:_playerFacing.BuildCommandRail(_workspace.Mode());\n',
        'FPS command rail suppression')

    old = '    f.commandHud=ShipCommandHudSystem::Build(hudSpeed,hudDamp,hudBoost,{"PRIMARY","MINING","SCAN","DRONES","REPAIR","UTILITY"},hudCombat);\n'
    new = old + '    if(_fleetStrategyActive){f.productionHud.modeLabel="FLEET COMMAND";f.productionHud.statusLines={"WASD PAN COMMAND CAMERA","I BOARD PLAYER SHIP","RMB ORBIT / MMB PAN / WHEEL ZOOM"};f.productionHud.showScanner=false;f.productionHud.showModuleRack=false;}else if(_embodiment.IsOnFoot()){f.flightHud={};f.commandHud={};f.tacticalContacts={};}\n'
    return replace_once(text, old, new, 'context HUD')

def transform_renderer_cpp(text: str) -> str:
    old = """void SetupPerspectiveProjection(const NativeBattlefieldFrame& frame) {
    const StrategicViewBasis b = StrategicViewProjection::Build(
        *frame.camera, static_cast<float>(frame.viewportWidth), static_cast<float>(frame.viewportHeight));
    gCameraEye=b.eye;

    const float top = b.nearPlane*b.tanHalfFov;
    const float right = top*b.aspect;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-right, right, -top, top, b.nearPlane, b.farPlane);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    const GLfloat m[16] = {
        b.right.x, b.up.x, -b.forward.x, 0.0f,
        b.right.y, b.up.y, -b.forward.y, 0.0f,
        b.right.z, b.up.z, -b.forward.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    glMultMatrixf(m);
    glTranslatef(-b.eye.x, -b.eye.y, -b.eye.z);
}
"""
    new = """void SetupPerspectiveProjection(const NativeBattlefieldFrame& frame) {
    if(frame.hasFirstPersonPose){ // R189_REAL_FPS_PROJECTION
        const auto& p=frame.firstPersonPose;gCameraEye=p.position;const float aspect=static_cast<float>(std::max(1,frame.viewportWidth))/static_cast<float>(std::max(1,frame.viewportHeight));const float nearPlane=.035f,farPlane=50000.0f;const float tanHalf=std::tan(p.verticalFovDegrees*.5f*kPi/180.0f);const float top=nearPlane*tanHalf,right=top*aspect;
        glMatrixMode(GL_PROJECTION);glLoadIdentity();glFrustum(-right,right,-top,top,nearPlane,farPlane);glMatrixMode(GL_MODELVIEW);glLoadIdentity();const GLfloat m[16]={p.right.x,p.up.x,-p.forward.x,0.0f,p.right.y,p.up.y,-p.forward.y,0.0f,p.right.z,p.up.z,-p.forward.z,0.0f,0.0f,0.0f,0.0f,1.0f};glMultMatrixf(m);glTranslatef(-p.position.x,-p.position.y,-p.position.z);return;}
    const StrategicViewBasis b = StrategicViewProjection::Build(*frame.camera, static_cast<float>(frame.viewportWidth), static_cast<float>(frame.viewportHeight));gCameraEye=b.eye;const float top=b.nearPlane*b.tanHalfFov,right=top*b.aspect;glMatrixMode(GL_PROJECTION);glLoadIdentity();glFrustum(-right,right,-top,top,b.nearPlane,b.farPlane);glMatrixMode(GL_MODELVIEW);glLoadIdentity();const GLfloat m[16]={b.right.x,b.up.x,-b.forward.x,0.0f,b.right.y,b.up.y,-b.forward.y,0.0f,b.right.z,b.up.z,-b.forward.z,0.0f,0.0f,0.0f,0.0f,1.0f};glMultMatrixf(m);glTranslatef(-b.eye.x,-b.eye.y,-b.eye.z);
}
"""
    text = replace_once(text, old, new, 'FPS projection')
    text = replace_once(text,
        '        // The present on-foot camera remains a top-down strategic view. Hide\n        // ceiling *visually* in this preview, never remove it from collision.\n        if(surface.axis==2&&surface.direction>0)continue;\n',
        '        // R189 real FPS keeps the authored ceiling visible; only legacy preview omits it.\n        if(!frame.hasFirstPersonPose&&surface.axis==2&&surface.direction>0)continue;\n',
        'interior ceiling')
    old = '    const auto& avatar=frame.interiorAvatar;\n    DrawSphere(avatar.localPosition.x,avatar.localPosition.y,\n               avatar.localPosition.z+avatar.capsuleHeightMeters*.5f,.18f,\n               {.88f,.66f,.22f,1.0f},16,8,SpaceMaterialKind::ShipHull);\n'
    new = '    if(!frame.hasFirstPersonPose){const auto& avatar=frame.interiorAvatar;DrawSphere(avatar.localPosition.x,avatar.localPosition.y,avatar.localPosition.z+avatar.capsuleHeightMeters*.5f,.18f,{.88f,.66f,.22f,1.0f},16,8,SpaceMaterialKind::ShipHull);}\n'
    text = replace_once(text, old, new, 'hide avatar in FPS')
    old = '    if(frame.dockingStage!=DockingExperienceStage::Docked){\n        DrawText5x7(frame.strategicFlightMode?"STRATEGIC FLIGHT":"MANUAL FLIGHT",w-302,20,1.05f,frame.strategicFlightMode?Rgba{0.30f,0.82f,0.72f,0.90f}:Rgba{0.90f,0.64f,0.24f,0.90f});\n        DrawText5x7("TAB MODE   SHIFT BOOST",w-302,40,.76f,{0.42f,0.62f,0.68f,0.66f});\n    }\n'
    new = '    if(frame.dockingStage!=DockingExperienceStage::Docked&&frame.embodimentMode!=ShipEmbodimentMode::InteriorOnFoot){DrawText5x7(frame.fleetStrategyActive?"FLEET COMMAND":"COCKPIT / PILOTING",w-302,20,1.05f,frame.fleetStrategyActive?Rgba{0.30f,0.82f,0.72f,0.90f}:Rgba{0.90f,0.64f,0.24f,0.90f});DrawText5x7(frame.fleetStrategyActive?"WASD CAMERA   I BOARD":"I LEAVE HELM   SHIFT BOOST",w-302,40,.76f,{0.42f,0.62f,0.68f,0.66f});}\n'
    text = replace_once(text, old, new, 'strategy/pilot HUD label')
    return replace_once(text,
        '    if(!compressedCruise&&!frame.standaloneShipyard)DrawWorldLabels(frame);\n',
        '    if(!compressedCruise&&!frame.standaloneShipyard&&!frame.hasFirstPersonPose)DrawWorldLabels(frame);\n',
        'FPS world labels')

def transform_cmake(text: str) -> str:
    if 'subspace_r189_strategy_fps_cutover_tests' in text:
        return text
    return text.rstrip() + """

# R189: strategy-first runtime / real FPS cutover.
if(SUBSPACE_BUILD_TESTS)
    add_executable(subspace_r189_strategy_fps_cutover_tests tests/r189_strategy_fps_cutover_tests.cpp)
    target_link_libraries(subspace_r189_strategy_fps_cutover_tests PRIVATE subspace_engine)
    add_test(NAME SubspaceR189StrategyFpsCutoverTests COMMAND subspace_r189_strategy_fps_cutover_tests)
endif()
"""

TRANSFORMS = {
    TARGETS[0]: transform_header,
    TARGETS[1]: transform_app_cpp,
    TARGETS[2]: transform_renderer_header,
    TARGETS[3]: transform_renderer_cpp,
    TARGETS[4]: transform_cmake,
}

def postconditions(root: Path) -> bool:
    for rel in POST:
        p = root / rel
        if not p.is_file():
            return False
        text = p.read_text(encoding='utf-8')
        if not _satisfies_r189_target(rel, text):
            return False
    return True

def materialize(root: Path, apply: bool) -> None:
    root = root.resolve()
    if postconditions(root):
        print('PASS: R189 strategy-first runtime / real FPS source already materialized')
        return
    head = run(root, 'git', 'rev-parse', 'HEAD').strip()
    if head != BASELINE:
        raise RuntimeError(f'R189 fresh materialization requires exact certified HEAD {BASELINE}; got {head}')
    outputs = {}
    for rel in TARGETS:
        current = (root/rel).read_text(encoding='utf-8')
        # Preserve any target that already satisfies R189, including exact
        # governed forward descendants such as R191. This prevents a repair to
        # one older target from re-transforming or downgrading newer targets.
        if _satisfies_r189_target(rel, current):
            outputs[rel] = current
            continue
        baseline = run(root, 'git', 'show', f'{BASELINE}:{rel}')
        certified = normalize_known_preimage(rel, current, baseline)
        outputs[rel] = TRANSFORMS[rel](certified)
    for rel, tokens in POST.items():
        if any(t not in outputs[rel] for t in tokens):
            raise RuntimeError(f'R189 transform failed semantic postconditions for {rel}')
    if not apply:
        print('READY: R189 exact 22aa951 preimage validated; --apply required')
        return
    stamp = time.strftime('%Y%m%d-%H%M%S')
    backup = root/'artifacts/gates/migrations/r189_strategy_fps_cutover'/stamp
    backup.mkdir(parents=True, exist_ok=True)
    for rel in TARGETS:
        dst = backup/rel; dst.parent.mkdir(parents=True, exist_ok=True); shutil.copy2(root/rel, dst)
    written=[]
    try:
        for rel, data in outputs.items():
            (root/rel).write_text(data, encoding='utf-8', newline='\n'); written.append(rel)
        if not postconditions(root): raise RuntimeError('R189 write completed but semantic verification failed')
    except Exception:
        for rel in written: shutil.copy2(backup/rel, root/rel)
        raise
    receipt = {
        'schema':'nullharbor.r189.strategy_fps_cutover.v1',
        'baseline':BASELINE,
        'strategy':'exact-certified-baseline-semantic-cutover',
        'files':{rel:hashlib.sha256((root/rel).read_bytes()).hexdigest() for rel in TARGETS},
        'forwardPolicy':'future governed descendants are accepted once semantic R189 postconditions remain present',
    }
    (backup.parent/'ACTIVE.json').write_text(json.dumps(receipt,indent=2)+'\n',encoding='utf-8')
    print('PASS: R189 strategy-first runtime / real FPS cutover materialized')

def main() -> int:
    ap=argparse.ArgumentParser(); ap.add_argument('--root',required=True); ap.add_argument('--apply',action='store_true'); a=ap.parse_args()
    try: materialize(Path(a.root), a.apply); return 0
    except Exception as exc: print(f'R189 CUTOVER BLOCKED: {exc}', file=sys.stderr); return 2
if __name__=='__main__': raise SystemExit(main())
