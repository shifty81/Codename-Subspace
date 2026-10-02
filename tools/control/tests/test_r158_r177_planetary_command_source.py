from __future__ import annotations

import sys
from pathlib import Path


def require(text: str, token: str, path: str) -> None:
    if token not in text:
        raise AssertionError(f"{path}: missing {token!r}")


def main() -> int:
    root = Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
    checks = {
        "engine/include/economy/PlanetaryIndustrySystem.h": [
            "enum class PiClaimState", "enum class PiOverlayMode", "enum class PiProjectionMode",
            "PiSectorIdentity", "PlaceGoverned", "CanClaim", "AdvanceSector",
        ],
        "engine/src/economy/PlanetaryIndustrySystem.cpp": [
            "CLAIM REQUIRES CONTIGUOUS OWNED BORDER", "ClaimFrontier", "PlaceGoverned",
        ],
        "engine/include/input/InputState.h": [
            "FleetCommandCancel", "PlanetaryCommandCycleOverlay", "Count",
        ],
        "engine/src/platform/NativeWindow.cpp": ["VK_F5", "PlanetaryCommandCycleOverlay"],
        "engine/src/integration/PlayerFacingIntegrationSystem.cpp": [
            "HitTestPlanetaryHex", "CyclePlanetaryOverlay", "TogglePlanetaryProjection",
            "AdvancePlanetarySector", "RecommendedIndustryKind",
        ],
        "engine/src/application/NativeGameApplication.cpp": [
            "R179 Planetary Command owns pointer clicks", "PlanetaryCommandCycleOverlay",
            "RecommendedIndustryKind",
        ],
        "engine/src/application/NativeBattlefieldRenderer.cpp": [
            "PLANETARY COMMAND - ", "SECTOR INSPECTOR", "CLAIM FRONTIER",
        ],
        "engine/src/ui/SandboxWorkspaceSystem.cpp": ["PLANETARY COMMAND", "Claim contiguous sector"],
    }
    for rel, tokens in checks.items():
        path = root / rel
        if not path.is_file():
            raise AssertionError(f"missing source file: {rel}")
        text = path.read_text(encoding="utf-8")
        for token in tokens:
            require(text, token, rel)

    header = (root / "engine/include/input/InputState.h").read_text(encoding="utf-8")
    if header.index("PlanetaryCommandCycleOverlay") < header.index("FleetCommandCancel"):
        raise AssertionError("R179 input action must remain append-only after R178 fleet actions")

    print("R158-R177 -> R179 Planetary Command source verification PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
