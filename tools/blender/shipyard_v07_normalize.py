"""Normalize Greyoxide Shipyard v0.7 mesh objects into individual centered OBJ derivatives.

Called headlessly by scripts/subspace_fetch_shipyard_v07.ps1 when the fetched source
contains Blender/FBX/DAE data rather than ready-to-use OBJ files.
"""
from __future__ import annotations

import argparse
import pathlib
import re
import sys

import bpy


def args_after_double_dash() -> list[str]:
    return sys.argv[sys.argv.index("--") + 1 :] if "--" in sys.argv else []


def classify(name: str) -> str:
    v = name.lower()
    groups = [
        ("command", ("cockpit", "canopy", "bridge", "command")),
        ("propulsion", ("engine", "thruster", "nozzle", "exhaust", "drive")),
        ("hardpoint", ("hardpoint", "turret", "weapon", "gun", "mount")),
        ("detail", ("greeble", "detail", "vent", "panel", "antenna", "radar", "sensor")),
        ("wing", ("wing", "fin")),
        ("adapter", ("adapter", "connector", "join", "neck", "fairing")),
        ("hull", ("hull", "body", "fuselage", "section", "frame")),
    ]
    for label, tokens in groups:
        if any(t in v for t in tokens):
            return label
    return "component"


def safe(name: str) -> str:
    result = re.sub(r"[^a-z0-9]+", "_", name.lower()).strip("_")
    return result or "mesh"


def import_file(path: pathlib.Path) -> None:
    ext = path.suffix.lower()
    if ext == ".blend":
        bpy.ops.wm.open_mainfile(filepath=str(path))
        return
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    if ext == ".fbx":
        bpy.ops.import_scene.fbx(filepath=str(path))
    elif ext == ".obj":
        if hasattr(bpy.ops.wm, "obj_import"):
            bpy.ops.wm.obj_import(filepath=str(path))
        else:
            bpy.ops.import_scene.obj(filepath=str(path))
    elif ext == ".dae":
        bpy.ops.wm.collada_import(filepath=str(path))
    else:
        raise RuntimeError(f"Unsupported source extension: {ext}")


def center_object(obj: bpy.types.Object) -> None:
    # Bake object transforms, then move mesh geometry so its bounding-box center
    # is the local origin.  Relative scale within each source object is kept.
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    try:
        bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    except Exception:
        pass
    mesh = obj.data
    if not mesh.vertices:
        return
    xs = [v.co.x for v in mesh.vertices]
    ys = [v.co.y for v in mesh.vertices]
    zs = [v.co.z for v in mesh.vertices]
    center = ((min(xs) + max(xs)) * 0.5, (min(ys) + max(ys)) * 0.5, (min(zs) + max(zs)) * 0.5)
    for vertex in mesh.vertices:
        vertex.co.x -= center[0]
        vertex.co.y -= center[1]
        vertex.co.z -= center[2]
    obj.location = (0.0, 0.0, 0.0)
    try:
        mesh.update(calc_edges=True)
    except TypeError:
        mesh.update()


def export_selected(path: pathlib.Path) -> None:
    # Material slots, UVs, normals, MTL sidecars and texture references are part
    # of the authored object contract. Never normalize geometry by silently
    # stripping the material graph. Newer Blender OBJ exporters expose PBR
    # extensions/path_mode; older versions get the compatible material-preserving
    # fallback.
    if hasattr(bpy.ops.wm, "obj_export"):
        try:
            bpy.ops.wm.obj_export(
                filepath=str(path),
                export_selected_objects=True,
                export_uv=True,
                export_normals=True,
                export_materials=True,
                export_pbr_extensions=True,
                path_mode="COPY",
            )
        except TypeError:
            bpy.ops.wm.obj_export(
                filepath=str(path),
                export_selected_objects=True,
                export_uv=True,
                export_normals=True,
                export_materials=True,
            )
    else:
        bpy.ops.export_scene.obj(
            filepath=str(path),
            use_selection=True,
            use_uvs=True,
            use_normals=True,
            use_materials=True,
            path_mode="COPY",
            axis_forward="-Z",
            axis_up="Y",
        )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", default="")
    parser.add_argument("--output", required=True)
    parser.add_argument("--source-label", default="shipyard")
    ns = parser.parse_args(args_after_double_dash())

    out = pathlib.Path(ns.output)
    out.mkdir(parents=True, exist_ok=True)
    if ns.input:
        import_file(pathlib.Path(ns.input))

    meshes = [o for o in bpy.context.scene.objects if o.type == "MESH"]
    meshes.sort(key=lambda o: o.name.lower())
    counts: dict[str, int] = {}
    exported = 0
    for obj in meshes:
        category = classify(obj.name)
        counts[category] = counts.get(category, 0) + 1
        name = f"shipyard_{category}_{safe(ns.source_label)}_{counts[category]:03d}_{safe(obj.name)}.obj"
        bpy.ops.object.select_all(action="DESELECT")
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj
        center_object(obj)
        export_selected(out / name)
        exported += 1
    print(f"SHIPYARD_NORMALIZED={exported}")
    return 0 if exported else 2


if __name__ == "__main__":
    raise SystemExit(main())
