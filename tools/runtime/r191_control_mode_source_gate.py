#!/usr/bin/env python3
from pathlib import Path
import argparse, sys

REQUIRED = {
    'engine/include/runtime/GameplayControlMode.h': ['GameplayControlMode', 'OnFoot', 'Pilot', 'FleetCommand'],
    'engine/include/platform/NativeWindow.h': ['NativeInputProfile', 'NativePointerPolicy', 'ConsumeRelativeMouseDelta'],
    'engine/src/platform/NativeWindow.cpp': ['WM_INPUT', 'RegisterRawInputDevices', 'CharacterJump', 'PilotThrustUp', 'FleetCameraForward'],
    'engine/src/application/NativeGameApplication.cpp': ['R191_PRIMARY_FPS_BOOT', 'SetGameplayControlMode', 'UpdateModeMouseInput', 'R191_REAL_COCKPIT_POSE'],
    'engine/include/interior/ShipEmbodimentSystem.h': ['headLookYawOffsetRadians', 'CanTakeControls', 'UpdateHeadLook'],
    'engine/src/interior/ShipEmbodimentSystem.cpp': ['ShipEmbodimentSystem::HeadLook', 'ShipEmbodimentSystem::UpdateHeadLook'],
    'engine/src/rendering/FirstPersonViewSystem.cpp': ['headLookYawOffsetRadians', 'headLookPitchOffsetRadians'],
    'engine/src/ui/RuntimeControlContextSystem.cpp': ['BuildForMode', 'RuntimePointerPolicy::RelativeCaptured', 'RuntimePointerPolicy::AbsoluteVisible'],
    'engine/src/input/PlayerControlSystem.cpp': ['InputAction::PilotForward', 'InputAction::PilotThrustUp', 'InputAction::PilotBrake'],
}

FORBIDDEN = {
    'engine/src/application/NativeGameApplication.cpp': ['_fleetStrategyActive'],
}

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--root', required=True); args=ap.parse_args()
    root=Path(args.root)
    for rel,tokens in REQUIRED.items():
        p=root/rel
        if not p.is_file(): print(f'[FAIL] missing {rel}'); return 2
        text=p.read_text(encoding='utf-8')
        for token in tokens:
            if token not in text: print(f'[FAIL] {rel}: missing {token}'); return 3
    for rel,tokens in FORBIDDEN.items():
        text=(root/rel).read_text(encoding='utf-8')
        for token in tokens:
            if token in text: print(f'[FAIL] {rel}: forbidden legacy token {token}'); return 4
    print('[PASS] R191 primary FPS / explicit control-mode source contract')
    return 0

if __name__=='__main__': raise SystemExit(main())
