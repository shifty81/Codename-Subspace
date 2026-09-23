#!/usr/bin/env python3
"""Read-only inventory of the user-supplied Blender donor archive; never imports it.

This inventories potential donor scripts without executing vendor code, copying assets,
or claiming function parity. It makes no source or licensing determination.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import PurePosixPath, Path
import zipfile

ROOT = "SourceWork/"
ALLOWED_PREFIX = ROOT + "Tools/Blender/"
BRIDGE = ROOT + "Tools/GameTools/compile_blender_scene_to_cpp_pcg_recipe_v1.py"
CATEGORIES = (
    ("NullharborKitbashGenerator/", "geometry-material-kitbash"),
    ("NullharborShipyardPipeline/", "shipyard-plan-socket"),
    ("ShipyardLineageGenerator/", "ship-interior-lineage"),
    ("NullharborShipFoundry/", "procedural-ship-foundry"),
    ("CharacterRigGenerator/", "character-rig-separate"),
    ("Archive/", "archived-version-not-authority"),
)


def inventory(path: Path):
    if not path.is_file():
        raise ValueError("donor archive does not exist")
    result = {"schema": "subspace.kitbash-donor-inventory.v1", "authority": "READ_ONLY_EVIDENCE",
              "archiveSha256": hashlib.sha256(path.read_bytes()).hexdigest(),
              "status": "CANDIDATES_UNREVIEWED", "donors": [], "warnings": []}
    with zipfile.ZipFile(path) as archive:
        names = archive.namelist()
        if len(names) != len(set(names)):
            raise ValueError("duplicate member paths in ZIP")
        for info in archive.infolist():
            name = info.filename
            p = PurePosixPath(name)
            if p.is_absolute() or ".." in p.parts or "\\" in name:
                raise ValueError("unsafe ZIP member: " + name)
            if not name.endswith(".py") or not (name.startswith(ALLOWED_PREFIX) or name == BRIDGE):
                continue
            if info.file_size > 4 * 1024 * 1024:
                raise ValueError("donor script exceeds 4 MiB: " + name)
            if info.compress_size and info.file_size / info.compress_size > 500:
                raise ValueError("donor expansion ratio excessive: " + name)
            contents = archive.read(info)
            if name == BRIDGE:
                group = "blender-scene-recipe-bridge"
            else:
                rel = name[len(ALLOWED_PREFIX):]
                group = next((category for prefix, category in CATEGORIES if rel.startswith(prefix)), "unclassified-blender-donor")
            result["donors"].append({"sourcePath": name, "sha256": hashlib.sha256(contents).hexdigest(),
                                     "bytes": len(contents), "category": group, "status": "REVIEW_BEFORE_PORT"})
    result["donors"].sort(key=lambda item: item["sourcePath"])
    if not result["donors"]:
        result["warnings"].append("No recognized donor Python source was found")
    result["warnings"].extend([
        "Inventory is not evidence that a script runs in the current Blender version.",
        "Donor files are not shipped, imported, executed, or made Subspace runtime dependencies.",
        "License, source permission, scene assets, output geometry, axis parity and tests require separate certification."])
    return result


def main(argv=None):
    parser = argparse.ArgumentParser(description="Inventory donor Blender source in SourceWork ZIP without executing or extracting")
    parser.add_argument("--archive", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        data = inventory(args.archive)
        if args.output.exists():
            raise FileExistsError("output exists; use a fresh staging path")
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        print("READ-ONLY donor scripts inventoried: {}. Nothing imported or executed.".format(len(data["donors"])))
        return 0
    except (OSError, ValueError, zipfile.BadZipFile) as exc:
        print("DONOR INVENTORY FAILED: " + str(exc))
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
