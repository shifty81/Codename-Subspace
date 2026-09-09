from __future__ import annotations

bl_info = {
    "name": "Codename Subspace Shipyard",
    "author": "Codename Subspace Project",
    "version": (0, 1, 5),
    "blender": (4, 5, 0),
    "location": "3D Viewport > Sidebar > Subspace Shipyard",
    "description": "Subspace-native modular ship authoring, socket editing, procedural generation and design export",
    "category": "Object",
}

import csv
import json
import math
import os
import random
from dataclasses import dataclass, field
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Sequence, Tuple

import bpy
from bpy.props import (
    BoolProperty,
    EnumProperty,
    FloatProperty,
    FloatVectorProperty,
    IntProperty,
    PointerProperty,
    StringProperty,
)
from bpy.types import Operator, Panel, PropertyGroup
from mathutils import Matrix, Vector


ADDON_VERSION = "0.1.5"
DESIGN_SCHEMA = "subspace.shipyard_design"
DESIGN_VERSION = 1
SHIP_COLLECTION_NAME = "SUBSPACE_SHIP"
LIBRARY_COLLECTION_NAME = "SUBSPACE_SHIPYARD_LIBRARY"
MARKER_COLLECTION_NAME = "SUBSPACE_SHIPYARD_MARKERS"
ROOT_OBJECT_NAME = "SSY_SHIP_ROOT"
PREVIEW_CAMERA_NAME = "SSY_PREVIEW_CAMERA"

ROLE_ITEMS = [
    ("INDUSTRIAL", "Industrial", "Large structural pieces and utility mass"),
    ("COMBAT", "Combat", "Compact hull with heavier hardpoint density"),
    ("MINING", "Mining", "Industrial hull with mining/tool hardpoint bias"),
    ("HAULER", "Hauler", "Longer large hull and cargo-oriented silhouette"),
    ("EXPLORATION", "Exploration", "Compact command/sensor-biased silhouette"),
]

EQUIPMENT_ITEMS = [
    ("PrimaryWeapon", "Primary Weapon", "Fixed or primary weapon"),
    ("Turret", "Turret", "Turret hardpoint"),
    ("Missile", "Missile", "Missile or launcher hardpoint"),
    ("MiningLaser", "Mining Laser", "Mining laser/tool mount"),
    ("SalvageBeam", "Salvage Beam", "Salvage beam/tool mount"),
    ("TractorBeam", "Tractor Beam", "Tractor beam mount"),
    ("Shield", "Shield", "Shield generator slot"),
    ("CounterMeasure", "Counter Measure", "Countermeasure slot"),
    ("Scanner", "Scanner", "Scanner/sensor equipment slot"),
    ("Drone", "Drone", "Drone equipment slot"),
    ("RepairBot", "Repair Bot", "Repair bot equipment slot"),
]

MODULE_CLASS_ORDER = {
    "hull": 0,
    "command": 1,
    "propulsion": 2,
    "hardpoint": 3,
    "wing": 4,
    "detail": 5,
    "adapter": 6,
    "component": 7,
}


@dataclass
class SocketDef:
    name: str
    type: str
    position: Vector
    direction: Vector
    insertion_depth: float = 0.0


@dataclass
class ModuleDef:
    module_id: str
    module_class: str
    source_obj: str
    relative_path: str
    grade: str = "A"
    semantic: str = "COMPONENT"
    size: str = "M"
    half_width: float = 0.5
    half_length: float = 0.5
    half_height: float = 0.5
    sockets: List[SocketDef] = field(default_factory=list)
    metadata_path: Optional[Path] = None
    attachment_face: str = ""
    attachment_confidence: float = 0.0


CATALOG: Dict[str, ModuleDef] = {}
CATALOG_ROOT: Optional[Path] = None
TEMPLATE_OBJECTS: Dict[str, str] = {}


def _norm(path: str) -> str:
    return os.path.normpath(bpy.path.abspath(path))


def _catalog_location_from_path(value: Path) -> Optional[Tuple[Path, Path, Path, Path]]:
    """Resolve a project root + certified payload from any useful Shipyard path.

    Accepted inputs intentionally include the repository root, the derived
    greyoxide_shipyard_v07 directory, the certified directory, the catalog CSV
    itself, or any child path inside the repository. This prevents the Blender
    tool from treating a user-selected certified folder as if it were the repo
    root and appending content/derived/... a second time.
    """
    try:
        p = value.expanduser().resolve()
    except Exception:
        p = value.expanduser()

    if p.is_file():
        if p.name.lower() != "certified_module_catalog.csv":
            return None
        certified = p.parent
        root = certified
        # Expected layout: <root>/content/derived/greyoxide_shipyard_v07/certified
        if certified.name.lower() == "certified" and certified.parent.name.lower() == "greyoxide_shipyard_v07":
            try:
                root = certified.parents[3]
            except IndexError:
                root = certified
        return root, p, certified / "modules", certified / "metadata"

    if not p.exists() or not p.is_dir():
        return None

    direct = p / "certified_module_catalog.csv"
    if direct.exists():
        certified = p
        root = certified
        if certified.name.lower() == "certified" and certified.parent.name.lower() == "greyoxide_shipyard_v07":
            try:
                root = certified.parents[3]
            except IndexError:
                pass
        return root, direct, certified / "modules", certified / "metadata"

    greyoxide = p / "certified" / "certified_module_catalog.csv"
    if greyoxide.exists():
        certified = p / "certified"
        root = p
        if p.name.lower() == "greyoxide_shipyard_v07":
            try:
                root = p.parents[2]
            except IndexError:
                pass
        return root, greyoxide, certified / "modules", certified / "metadata"

    repo_catalog = p / "content" / "derived" / "greyoxide_shipyard_v07" / "certified" / "certified_module_catalog.csv"
    if repo_catalog.exists():
        certified = repo_catalog.parent
        return p, repo_catalog, certified / "modules", certified / "metadata"

    # A path somewhere inside the repository is also accepted.
    for parent in [p] + list(p.parents):
        repo_catalog = parent / "content" / "derived" / "greyoxide_shipyard_v07" / "certified" / "certified_module_catalog.csv"
        if repo_catalog.exists():
            certified = repo_catalog.parent
            return parent, repo_catalog, certified / "modules", certified / "metadata"
    return None


def _resolve_catalog_location(context) -> Optional[Tuple[Path, Path, Path, Path]]:
    s = context.scene.ssy_settings
    candidates: List[Path] = []

    catalog_override = getattr(s, "catalog_file", "").strip()
    if catalog_override:
        candidates.append(Path(_norm(catalog_override)))

    project_value = s.project_root.strip()
    if project_value:
        candidates.append(Path(_norm(project_value)))

    env_root = os.environ.get("SUBSPACE_PROJECT_ROOT", "").strip()
    if env_root:
        candidates.append(Path(env_root))

    if bpy.data.filepath:
        candidates.append(Path(bpy.data.filepath).resolve().parent)
    candidates.append(Path.cwd())

    # Common local checkout locations are low-cost fallbacks only; no recursive
    # disk scan is performed.
    home = Path.home()
    candidates.extend([
        home / "Desktop" / "Codename-Subspace-main",
        home / "Documents" / "Codename-Subspace-main",
    ])

    seen = set()
    for candidate in candidates:
        key = os.path.normcase(str(candidate))
        if key in seen:
            continue
        seen.add(key)
        resolved = _catalog_location_from_path(candidate)
        if resolved is not None:
            return resolved
    return None


def _project_root_from_scene(context) -> Optional[Path]:
    resolved = _resolve_catalog_location(context)
    return resolved[0] if resolved else None


def _catalog_paths(root: Path) -> Tuple[Path, Path, Path]:
    resolved = _catalog_location_from_path(root)
    if resolved is not None:
        return resolved[1], resolved[2], resolved[3]
    base = root / "content" / "derived" / "greyoxide_shipyard_v07" / "certified"
    return base / "certified_module_catalog.csv", base / "modules", base / "metadata"


def _safe_float(v, default=0.0) -> float:
    try:
        return float(v)
    except Exception:
        return default


def _canonical_leaf(value: str) -> str:
    value = Path(value).stem.lower()
    nested = value.find("_shipyard_")
    if nested >= 0:
        value = value[nested + 1:]
    for prefix in ("shipyard_a_", "shipyard_"):
        if value.startswith(prefix):
            rest = value[len(prefix):]
            parts = rest.split("_", 2)
            if len(parts) == 3:
                value = parts[2]
    return value


def _name_classification(value: str) -> Tuple[str, str]:
    v = _canonical_leaf(value)
    has = lambda *tokens: any(t in v for t in tokens)
    if v.startswith("hull") or has("_hull", "fuselage"):
        if has("bow", "nose", "front"):
            return "hull", "HULL_BOW"
        if has("aft", "rear", "stern"):
            return "hull", "HULL_AFT"
        return "hull", "HULL_MID"
    if v.startswith("hardpoint"):
        return "hardpoint", ("WEAPON_MOUNT" if has("gun", "weapon", "cannon", "missile") else "TURRET_HARDPOINT")
    if has("bridge", "cockpit", "canopy", "command"):
        return "command", ("COMMAND_COCKPIT" if has("cockpit", "canopy") else "COMMAND_BRIDGE")
    if v.startswith("engine") or v.startswith("enigne"):
        if has("trussworkwing", "wing"):
            return "wing", "WING"
        if has("strut"):
            return "component", "STRUCTURAL_FRAME"
        if has("body", "bracket"):
            return "propulsion", "ENGINE_HOUSING"
        if has("trumpet", "vanes", "flap"):
            return "propulsion", "ENGINE_NOZZLE"
        return "propulsion", "MAIN_ENGINE"
    if has("bipolarengine"):
        return "propulsion", "MAIN_ENGINE"
    if has("rcs", "retro", "maneuver", "thruster"):
        return "propulsion", "RCS_THRUSTER"
    if has("nozzle", "bell", "exhaust"):
        return "propulsion", "ENGINE_NOZZLE"
    if has("housing", "shroud", "nacelle", "enginepod", "engine_pod"):
        return "propulsion", "ENGINE_HOUSING"
    if has("drive", "propulsion"):
        return "propulsion", "MAIN_ENGINE"
    if has("vertmisstube", "misstube", "missiletube", "missile_tube"):
        return "hardpoint", "WEAPON_MOUNT"
    if has("wing", "fin"):
        return "wing", "WING"
    if has("sensor", "antenna", "radar", "instrument", "telescope", "dish", "mast"):
        return "detail", "SENSOR"
    if has("greeble", "detail", "vent", "panel", "leafpanel"):
        return "detail", "SURFACE_DETAIL"
    if has("outrigger", "railrunner"):
        return "component", "STRUCTURAL_FRAME"
    if has("adapter", "connector", "neck", "fairing"):
        return "adapter", "ADAPTER"
    if has("turret", "weapon", "gun", "mount"):
        return "hardpoint", ("WEAPON_MOUNT" if has("gun", "weapon") else "TURRET_HARDPOINT")
    return "component", "COMPONENT"


def _infer_semantic(module_id: str, module_class: str) -> str:
    # The class prefix embedded in old certified IDs is provenance only.
    # Always infer from the original leaf name, matching the native runtime.
    _, semantic = _name_classification(module_id)
    return semantic

def _size_from_dims(w: float, l: float, h: float) -> str:
    major = 2.0 * max(w, l, h)
    if major < 0.8:
        return "XS"
    if major < 1.8:
        return "S"
    if major < 3.8:
        return "M"
    if major < 7.5:
        return "L"
    return "XL"


def _source_to_shipyard_xyz(x: float, y: float, z: float) -> Tuple[float, float, float]:
    """Convert the Greyoxide OBJ convention into Subspace/Blender convention.

    The normalized source corpus is Y-up with ship-forward on +Z after export. Shipyard authoring
    and runtime logic use X=right, +Y=forward, +Z=up. Keeping this conversion
    at the ingestion boundary prevents the source Z length from becoming a
    vertical Blender height and keeps socket math consistent everywhere.
    """
    return x, z, y


def _read_obj_profile(path: Path) -> Tuple[float, float, float, str, float]:
    vertices: List[Tuple[float, float, float]] = []
    edge_use: Dict[Tuple[int, int], int] = {}
    try:
        with path.open("r", encoding="utf-8", errors="ignore") as fh:
            for line in fh:
                if line.startswith("v "):
                    parts = line.split()
                    if len(parts) >= 4:
                        vertices.append(_source_to_shipyard_xyz(
                            _safe_float(parts[1]), _safe_float(parts[2]), _safe_float(parts[3])
                        ))
                elif line.startswith("f "):
                    face: List[int] = []
                    for token in line.split()[1:]:
                        head = token.split("/", 1)[0]
                        if not head:
                            continue
                        try:
                            idx = int(head)
                        except ValueError:
                            continue
                        idx = len(vertices) + idx if idx < 0 else idx - 1
                        if 0 <= idx < len(vertices):
                            face.append(idx)
                    if len(face) >= 3:
                        for a, b in zip(face, face[1:] + face[:1]):
                            if a == b:
                                continue
                            key = (a, b) if a < b else (b, a)
                            edge_use[key] = edge_use.get(key, 0) + 1
    except OSError:
        return 0.5, 0.5, 0.5, "", 0.0
    if not vertices:
        return 0.5, 0.5, 0.5, "", 0.0

    mn = [min(v[i] for v in vertices) for i in range(3)]
    mx = [max(v[i] for v in vertices) for i in range(3)]
    dims = [max(0.002, mx[i] - mn[i]) for i in range(3)]

    # Detect an intentionally open attachment face from mesh boundary edges.
    # This is a hint, not a replacement for explicit metadata. The strongest
    # use is an open ventral/bottom face on bridges, sensors and surface parts.
    boundary = [edge for edge, count in edge_use.items() if count == 1]
    face_names = ((0, 0, "port"), (0, 1, "starboard"),
                  (1, 0, "aft"), (1, 1, "forward"),
                  (2, 0, "ventral"), (2, 1, "dorsal"))
    scores: Dict[str, int] = {name: 0 for _, _, name in face_names}
    if boundary:
        for a, b in boundary:
            midpoint = tuple((vertices[a][i] + vertices[b][i]) * 0.5 for i in range(3))
            for axis, high, name in face_names:
                extreme = mx[axis] if high else mn[axis]
                if abs(midpoint[axis] - extreme) <= dims[axis] * 0.08:
                    scores[name] += 1

    best_face = ""
    confidence = 0.0
    if boundary:
        best_face, best_count = max(scores.items(), key=lambda item: item[1])
        confidence = best_count / max(1, len(boundary))
        # Require enough boundary evidence to avoid treating small decorative
        # holes as authoritative mounting faces. Ventral gets a slightly lower
        # threshold because open-bottom kitbash pieces are a known pack idiom.
        threshold = 0.08 if best_face == "ventral" else 0.12
        if best_count < 4 or confidence < threshold:
            best_face, confidence = "", 0.0

    return (dims[0] * 0.5, dims[1] * 0.5, dims[2] * 0.5, best_face, confidence)


def _read_obj_bounds(path: Path) -> Tuple[float, float, float]:
    w, l, h, _, _ = _read_obj_profile(path)
    return w, l, h


def _face_socket(face: str, w: float, l: float, h: float) -> Tuple[Vector, Vector]:
    mapping = {
        "port": (Vector((-w, 0, 0)), Vector((-1, 0, 0))),
        "starboard": (Vector((w, 0, 0)), Vector((1, 0, 0))),
        "aft": (Vector((0, -l, 0)), Vector((0, -1, 0))),
        "forward": (Vector((0, l, 0)), Vector((0, 1, 0))),
        "ventral": (Vector((0, 0, -h)), Vector((0, 0, -1))),
        "dorsal": (Vector((0, 0, h)), Vector((0, 0, 1))),
    }
    return mapping.get(face, (Vector((0, 0, -h)), Vector((0, 0, -1))))


def _runtime_sockets(m: ModuleDef) -> List[SocketDef]:
    w, l, h = m.half_width, m.half_length, m.half_height
    insertion = max(0.025, min(w, l, h) * 0.10)
    out: List[SocketDef] = []

    def add(name, typ, x, y, z, dx, dy, dz, depth):
        out.append(SocketDef(name, typ, Vector((x, y, z)), Vector((dx, dy, dz)), depth))

    s = m.semantic
    if s in {"HULL_BOW", "HULL_MID", "HULL_AFT", "STRUCTURAL_FRAME", "COMPONENT"}:
        add("forward", "hull_forward", 0, l, 0, 0, 1, 0, insertion)
        add("aft", "hull_aft", 0, -l, 0, 0, -1, 0, insertion)
        add("port", "lateral_surface", -w, 0, 0, -1, 0, 0, insertion)
        add("starboard", "lateral_surface", w, 0, 0, 1, 0, 0, insertion)
        add("dorsal", "dorsal_surface", 0, 0, h, 0, 0, 1, insertion)
        add("ventral", "ventral_surface", 0, 0, -h, 0, 0, -1, insertion)
        add("engine_port", "engine_cavity", -w * 0.42, -l + l * 0.55, 0, 0, -1, 0, max(insertion, l * 0.12))
        add("engine_starboard", "engine_cavity", w * 0.42, -l + l * 0.55, 0, 0, -1, 0, max(insertion, l * 0.12))
    elif s in {"COMMAND_COCKPIT", "COMMAND_BRIDGE"}:
        # Greyoxide bridges commonly use an open bottom as the authored
        # attachment surface. Prefer that geometry-derived mount when present;
        # retain the aft socket for older nose/inline command pieces.
        if m.attachment_face == "ventral":
            pos, direction = _face_socket("ventral", w, l, h)
            add("mount", "command_mount", pos.x, pos.y, pos.z, direction.x, direction.y, direction.z, insertion)
        add("aft", "hull_aft", 0, -l, 0, 0, -1, 0, insertion)
    elif s == "ADAPTER":
        add("forward", "hull_forward", 0, l, 0, 0, 1, 0, insertion)
        add("aft", "hull_aft", 0, -l, 0, 0, -1, 0, insertion)
    elif s == "ENGINE_HOUSING":
        add("mount", "engine_mount", 0, l, 0, 0, 1, 0, insertion)
        add("engine_cavity", "engine_cavity", 0, -l * 0.35, 0, 0, -1, 0, max(insertion, l * 0.15))
    elif s in {"MAIN_ENGINE", "ENGINE_NOZZLE", "RCS_THRUSTER"}:
        add("mount", "engine_mount", 0, l, 0, 0, 1, 0, insertion)
        add("exhaust", "exhaust", 0, -l, 0, 0, -1, 0, 0)
    elif s in {"TURRET_HARDPOINT", "WEAPON_MOUNT"}:
        add("mount", "hardpoint_mount", 0, 0, -h, 0, 0, -1, insertion)
        add("weapon_axis", "weapon_axis", 0, 0, h, 0, 0, 1, 0)
    elif s == "WING":
        add("mount", "lateral_mount", 0, 0, 0, 1, 0, 0, insertion)
    elif s in {"SENSOR", "SURFACE_DETAIL"}:
        face = m.attachment_face if m.attachment_face else "ventral"
        pos, direction = _face_socket(face, w, l, h)
        add("mount", "detail_mount", pos.x, pos.y, pos.z, direction.x, direction.y, direction.z, insertion * 0.5)
    return out


def _can_mate(parent: str, child: str) -> bool:
    return (
        (parent == "hull_aft" and child == "hull_forward")
        or (parent == "hull_forward" and child == "hull_aft")
        or (parent == "engine_cavity" and child == "engine_mount")
        or (parent == "dorsal_surface" and child in {"hardpoint_mount", "detail_mount", "command_mount"})
        or (parent == "lateral_surface" and child in {"lateral_mount", "engine_mount", "detail_mount"})
        or (parent == "ventral_surface" and child in {"hardpoint_mount", "detail_mount"})
    )


def _role_suitable(semantic: str, role: str) -> bool:
    if semantic in {"HULL_BOW", "HULL_MID", "HULL_AFT", "STRUCTURAL_FRAME"}:
        return True
    if semantic == "COMMAND_COCKPIT":
        return role in {"COMBAT", "EXPLORATION"}
    if semantic == "COMMAND_BRIDGE":
        return role != "COMBAT" or role == "HAULER"
    if semantic in {"TURRET_HARDPOINT", "WEAPON_MOUNT"}:
        return role in {"COMBAT", "MINING"}
    if semantic == "SENSOR":
        return role in {"EXPLORATION", "MINING"}
    return True


def _module_enum_items(self, context):
    if not CATALOG:
        return [("__NONE__", "Load catalog first", "")]
    result = []
    for module_id, m in sorted(CATALOG.items(), key=lambda kv: (MODULE_CLASS_ORDER.get(kv[1].module_class, 99), kv[0])):
        label = f"[{m.module_class.upper()} / {m.size}] {module_id}"
        result.append((module_id, label, m.semantic))
    return result


def _ensure_collection(name: str, hide_viewport: bool = False, hide_render: bool = False):
    col = bpy.data.collections.get(name)
    if col is None:
        col = bpy.data.collections.new(name)
        bpy.context.scene.collection.children.link(col)
    col.hide_render = hide_render
    try:
        col.hide_viewport = hide_viewport
    except Exception:
        pass
    return col


def _ensure_root() -> bpy.types.Object:
    root = bpy.data.objects.get(ROOT_OBJECT_NAME)
    if root is None:
        root = bpy.data.objects.new(ROOT_OBJECT_NAME, None)
        root.empty_display_type = "PLAIN_AXES"
        root.empty_display_size = 1.0
        _ensure_collection(SHIP_COLLECTION_NAME).objects.link(root)
        root["ssy_ship_root"] = True
    return root


def _remove_object(obj: bpy.types.Object) -> None:
    bpy.data.objects.remove(obj, do_unlink=True)


def _clear_ship() -> None:
    for obj in list(bpy.data.objects):
        if obj.get("ssy_design_part") or obj.get("ssy_marker") in {"socket", "equipment_slot", "decal"} or obj.get("ssy_ship_root"):
            _remove_object(obj)


def _load_catalog(context) -> Tuple[int, str]:
    global CATALOG_ROOT
    resolved = _resolve_catalog_location(context)
    if resolved is None:
        return 0, "Could not locate certified_module_catalog.csv. Select the repository root, greyoxide_shipyard_v07 folder, certified folder, or the catalog CSV itself."
    root, catalog_csv, modules_dir, metadata_dir = resolved
    if not catalog_csv.exists():
        return 0, f"Missing certified module catalog: {catalog_csv}"

    loaded: Dict[str, ModuleDef] = {}
    with catalog_csv.open("r", encoding="utf-8-sig", newline="") as fh:
        for row in csv.DictReader(fh):
            module_id = row.get("module_id", "").strip()
            if not module_id or row.get("grade", "A").strip().upper() != "A":
                continue
            rel = row.get("relative_path", "").replace("/", os.sep).replace("\\", os.sep)
            obj_path = root / rel if rel else modules_dir / f"{module_id}.obj"
            if not obj_path.exists():
                # Certified file name is the module id in the current corpus.
                fallback = modules_dir / f"{module_id}.obj"
                if fallback.exists():
                    obj_path = fallback
            cls = row.get("class", "component").strip().lower() or "component"
            w, l, h, attachment_face, attachment_confidence = _read_obj_profile(obj_path)
            m = ModuleDef(
                module_id=module_id,
                module_class=cls,
                source_obj=row.get("source_obj", ""),
                relative_path=str(obj_path),
                grade=row.get("grade", "A"),
                semantic=(row.get("semantic", "").strip().upper() or _infer_semantic(module_id, cls)),
                half_width=w,
                half_length=l,
                half_height=h,
                metadata_path=metadata_dir / f"{module_id}.json",
                attachment_face=attachment_face,
                attachment_confidence=attachment_confidence,
            )
            m.size = _size_from_dims(w, l, h)
            m.sockets = _runtime_sockets(m)
            loaded[module_id] = m

    CATALOG.clear()
    CATALOG.update(loaded)
    CATALOG_ROOT = root
    context.scene.ssy_settings.project_root = str(root)
    context.scene.ssy_settings.catalog_file = str(catalog_csv)
    if loaded and context.scene.ssy_settings.module_choice not in loaded:
        context.scene.ssy_settings.module_choice = next(iter(loaded))
    return len(loaded), f"Loaded {len(loaded)} certified Shipyard modules"


def _read_mtl_library(path: Path) -> Dict[str, Dict[str, object]]:
    materials: Dict[str, Dict[str, object]] = {}
    current: Optional[Dict[str, object]] = None
    try:
        with path.open("r", encoding="utf-8", errors="ignore") as fh:
            for raw in fh:
                parts = raw.strip().split()
                if not parts:
                    continue
                key = parts[0].lower()
                if key == "newmtl" and len(parts) >= 2:
                    current = {"name": " ".join(parts[1:]), "kd": (0.55, 0.58, 0.62), "ke": (0.0, 0.0, 0.0), "alpha": 1.0}
                    materials[str(current["name"])] = current
                elif current is not None and key in {"kd", "ke"} and len(parts) >= 4:
                    current[key] = tuple(_safe_float(v) for v in parts[1:4])
                elif current is not None and key == "d" and len(parts) >= 2:
                    current["alpha"] = max(0.0, min(1.0, _safe_float(parts[1], 1.0)))
                elif current is not None and key == "tr" and len(parts) >= 2:
                    current["alpha"] = 1.0 - max(0.0, min(1.0, _safe_float(parts[1], 0.0)))
    except OSError:
        pass
    return materials


def _ensure_authored_material(defn: Dict[str, object]):
    name = "SSY_SRC_" + str(defn.get("name", "Material"))
    mat = bpy.data.materials.get(name)
    if mat is None:
        mat = bpy.data.materials.new(name)
    kd = tuple(defn.get("kd", (0.55, 0.58, 0.62)))
    ke = tuple(defn.get("ke", (0.0, 0.0, 0.0)))
    alpha = float(defn.get("alpha", 1.0))
    mat.diffuse_color = (*kd[:3], alpha)
    mat.use_nodes = True
    try:
        bsdf = mat.node_tree.nodes.get("Principled BSDF") if mat.node_tree else None
        if bsdf:
            bsdf.inputs["Base Color"].default_value = (*kd[:3], 1.0)
            bsdf.inputs["Metallic"].default_value = 0.72 if "Metal" in str(defn.get("name", "")) else 0.25
            bsdf.inputs["Roughness"].default_value = 0.30 if "Metal" in str(defn.get("name", "")) else 0.42
            if "Emission Color" in bsdf.inputs:
                bsdf.inputs["Emission Color"].default_value = (*ke[:3], 1.0)
            elif "Emission" in bsdf.inputs:
                bsdf.inputs["Emission"].default_value = (*ke[:3], 1.0)
            if "Emission Strength" in bsdf.inputs:
                bsdf.inputs["Emission Strength"].default_value = 2.5 if max(ke) > 0.001 else 0.0
            if "Alpha" in bsdf.inputs:
                bsdf.inputs["Alpha"].default_value = alpha
        if alpha < 0.999:
            mat.surface_render_method = "DITHERED"
    except Exception:
        pass
    return mat


def _import_obj_as_template(module: ModuleDef) -> Optional[bpy.types.Object]:
    existing_name = TEMPLATE_OBJECTS.get(module.module_id)
    if existing_name:
        obj = bpy.data.objects.get(existing_name)
        if obj is not None:
            return obj

    path = Path(module.relative_path)
    if not path.exists():
        return None

    # Parse the OBJ ourselves, but canonicalize its Y-up/+Z-forward source
    # coordinates to Shipyard X-right/+Y-forward/+Z-up at ingestion. This is
    # the same coordinate contract used by sockets and generated designs.
    vertices: List[Tuple[float, float, float]] = []
    faces: List[List[int]] = []
    face_materials: List[str] = []
    material_defs: Dict[str, Dict[str, object]] = {}
    material_order: List[str] = []
    active_material = ""
    try:
        with path.open("r", encoding="utf-8", errors="ignore") as fh:
            for line in fh:
                if line.startswith("mtllib "):
                    for lib_name in line.split()[1:]:
                        material_defs.update(_read_mtl_library(path.parent / lib_name))
                elif line.startswith("usemtl "):
                    active_material = line.strip()[7:].strip()
                    if active_material and active_material not in material_order:
                        material_order.append(active_material)
                elif line.startswith("v "):
                    parts = line.split()
                    if len(parts) >= 4:
                        vertices.append(_source_to_shipyard_xyz(_safe_float(parts[1]), _safe_float(parts[2]), _safe_float(parts[3])))
                elif line.startswith("f "):
                    raw = line.split()[1:]
                    face: List[int] = []
                    for token in raw:
                        head = token.split("/", 1)[0]
                        if not head:
                            continue
                        try:
                            idx = int(head)
                        except ValueError:
                            continue
                        if idx < 0:
                            idx = len(vertices) + idx
                        else:
                            idx -= 1
                        if 0 <= idx < len(vertices):
                            face.append(idx)
                    if len(face) >= 3:
                        faces.append(face)
                        face_materials.append(active_material)
    except OSError:
        return None
    if not vertices or not faces:
        return None

    mesh = bpy.data.meshes.new(f"SSY_MESH_{module.module_id}")
    mesh.from_pydata(vertices, [], faces)
    for name in material_order:
        definition = material_defs.get(name, {"name": name})
        mesh.materials.append(_ensure_authored_material(definition))
    material_index = {name: i for i, name in enumerate(material_order)}
    for poly, name in zip(mesh.polygons, face_materials):
        poly.material_index = material_index.get(name, 0)
    mesh.update()
    obj = bpy.data.objects.new(f"SSY_TEMPLATE_{module.module_id}", mesh)
    obj["ssy_template"] = True
    obj["ssy_module_id"] = module.module_id
    lib = _ensure_collection(LIBRARY_COLLECTION_NAME, hide_viewport=True, hide_render=True)
    lib.objects.link(obj)
    obj.hide_render = True
    try:
        obj.hide_set(True)
    except Exception:
        pass
    TEMPLATE_OBJECTS[module.module_id] = obj.name
    return obj


def _next_instance_id(prefix="M") -> str:
    used = {str(o.get("ssy_instance_id")) for o in bpy.data.objects if o.get("ssy_instance_id")}
    i = 1
    while f"{prefix}{i:03d}" in used:
        i += 1
    return f"{prefix}{i:03d}"


def _instantiate_module(module: ModuleDef, location=(0, 0, 0), scale=1.0, instance_id: Optional[str] = None) -> Optional[bpy.types.Object]:
    template = _import_obj_as_template(module)
    if template is None:
        return None
    resolved_instance_id = instance_id or _next_instance_id()
    obj = template.copy()
    obj.data = template.data
    obj.name = f"SSY_{resolved_instance_id}_{module.module_id}"
    obj.hide_render = False
    obj.location = location
    obj.rotation_euler = (0.0, 0.0, 0.0)
    obj.scale = (scale, scale, scale)
    obj["ssy_design_part"] = True
    obj["ssy_module_id"] = module.module_id
    obj["ssy_instance_id"] = resolved_instance_id
    obj["ssy_module_class"] = module.module_class
    obj["ssy_semantic"] = module.semantic
    obj["ssy_size"] = module.size
    obj["ssy_source_obj"] = module.source_obj
    obj["ssy_attachment_face"] = module.attachment_face
    obj["ssy_attachment_confidence"] = float(module.attachment_confidence)
    obj["ssy_parent_instance"] = ""
    obj["ssy_parent_socket"] = ""
    obj["ssy_child_socket"] = ""
    obj["ssy_uniform_scale_required"] = module.module_class != "detail"
    ship_col = _ensure_collection(SHIP_COLLECTION_NAME)
    for col in list(obj.users_collection):
        col.objects.unlink(obj)
    ship_col.objects.link(obj)
    try:
        obj.hide_set(False)
    except Exception:
        pass
    obj.parent = _ensure_root()
    return obj


def _module_for_object(obj) -> Optional[ModuleDef]:
    if obj is None:
        return None
    return CATALOG.get(str(obj.get("ssy_module_id", "")))


def _find_socket(module: ModuleDef, name: str) -> Optional[SocketDef]:
    for socket in module.sockets:
        if socket.name == name:
            return socket
    return None


def _socket_world(obj: bpy.types.Object, socket: SocketDef) -> Tuple[Vector, Vector]:
    pos = obj.matrix_world @ socket.position
    direction = (obj.matrix_world.to_3x3() @ socket.direction).normalized()
    return pos, direction


def _attach(child: bpy.types.Object, child_socket: SocketDef, parent: bpy.types.Object, parent_socket: SocketDef, insertion_multiplier=1.0):
    # Align both socket position *and orientation*. Previous versions only
    # translated the child, so a side/bottom mounting face could never rotate
    # into its parent's socket and vertical-looking assemblies were easy to
    # create accidentally. Socket normals face away from each part, so mating
    # requires the child's normal to oppose the parent's normal.
    parent_pos, parent_dir = _socket_world(parent, parent_socket)
    child_pos, child_dir = _socket_world(child, child_socket)
    target_dir = (-parent_dir).normalized()
    if child_dir.length > 1e-6 and target_dir.length > 1e-6:
        delta = child_dir.rotation_difference(target_dir)
        current = child.rotation_euler.to_quaternion()
        child.rotation_euler = (delta @ current).to_euler()
        try:
            bpy.context.view_layer.update()
        except Exception:
            pass

    parent_scale = max(abs(parent.scale.x), abs(parent.scale.y), abs(parent.scale.z))
    child_linear = child.matrix_world.to_3x3()
    child_offset = child_linear @ child_socket.position
    insertion = parent_socket.insertion_depth * parent_scale * insertion_multiplier
    child.location = parent_pos - child_offset - parent_dir * insertion
    child["ssy_parent_instance"] = str(parent.get("ssy_instance_id", ""))
    child["ssy_parent_socket"] = parent_socket.name
    child["ssy_child_socket"] = child_socket.name


def _sockets_for_obj(obj: bpy.types.Object) -> List[SocketDef]:
    m = _module_for_object(obj)
    return m.sockets if m else []


def _create_socket_markers(obj: bpy.types.Object) -> int:
    m = _module_for_object(obj)
    if m is None:
        return 0
    for child in list(obj.children):
        if child.get("ssy_marker") == "socket":
            _remove_object(child)
    marker_col = _ensure_collection(MARKER_COLLECTION_NAME)
    count = 0
    for s in m.sockets:
        marker = bpy.data.objects.new(f"SOCKET_{obj.get('ssy_instance_id')}_{s.name}", None)
        marker.empty_display_type = "ARROWS"
        marker.empty_display_size = max(0.05, min(m.half_width, m.half_length, m.half_height) * 0.25)
        marker.location = s.position
        marker["ssy_marker"] = "socket"
        marker["socket_name"] = s.name
        marker["socket_type"] = s.type
        marker["direction"] = [s.direction.x, s.direction.y, s.direction.z]
        marker["insertion_depth"] = s.insertion_depth
        marker.parent = obj
        marker_col.objects.link(marker)
        count += 1
    return count


def _ship_modules() -> List[bpy.types.Object]:
    return sorted([o for o in bpy.data.objects if o.get("ssy_design_part")], key=lambda o: str(o.get("ssy_instance_id", o.name)))


def _equipment_markers() -> List[bpy.types.Object]:
    return [o for o in bpy.data.objects if o.get("ssy_marker") == "equipment_slot"]


def _module_pool(module_class: Optional[str] = None, semantics: Optional[set] = None) -> List[ModuleDef]:
    pool = list(CATALOG.values())
    if module_class is not None:
        pool = [m for m in pool if m.module_class == module_class]
    if semantics is not None:
        pool = [m for m in pool if m.semantic in semantics]
    return pool


def _pick_role_hull(pool: List[ModuleDef], role: str, rng: random.Random) -> Optional[ModuleDef]:
    suitable = [m for m in pool if _role_suitable(m.semantic, role)] or list(pool)
    if not suitable:
        return None
    size_rank = {"XS": 0, "S": 1, "M": 2, "L": 3, "XL": 4}
    suitable.sort(key=lambda m: (size_rank.get(m.size, 2), m.module_id))
    if role in {"INDUSTRIAL", "HAULER", "MINING"}:
        start = len(suitable) // 2
        return rng.choice(suitable[start:])
    return rng.choice(suitable[: max(1, (len(suitable) + 1) // 2)])


def _set_connection(obj, parent, parent_socket: str, child_socket: str):
    obj["ssy_parent_instance"] = str(parent.get("ssy_instance_id", "")) if parent else ""
    obj["ssy_parent_socket"] = parent_socket
    obj["ssy_child_socket"] = child_socket


def _place_direct(m: ModuleDef, position: Vector, scale: float, prefix="M") -> Optional[bpy.types.Object]:
    return _instantiate_module(m, tuple(position), scale, _next_instance_id(prefix))


def _generate_ship(context) -> Tuple[int, str]:
    settings = context.scene.ssy_settings
    if not CATALOG:
        count, message = _load_catalog(context)
        if count == 0:
            return 0, message
    if settings.clear_before_generate:
        _clear_ship()
    root = _ensure_root()
    root["ssy_design_name"] = settings.ship_name
    root["ssy_generator_role"] = settings.role
    root["ssy_generator_seed"] = settings.seed
    root["ssy_generator_version"] = ADDON_VERSION

    rng = random.Random(settings.seed)
    hulls = _module_pool("hull")
    commands = _module_pool("command")
    engines = [m for m in _module_pool("propulsion") if m.semantic == "MAIN_ENGINE"]
    if not engines:
        engines = _module_pool("propulsion")
    hardpoints = _module_pool("hardpoint")
    details = _module_pool("detail")
    wings = _module_pool("wing")
    adapters = _module_pool("adapter")
    if not hulls:
        hulls = [m for m in CATALOG.values() if m.module_class == "component" and m.half_length >= m.half_height]
    if not hulls:
        return 0, "No hull-capable modules in the loaded catalog"

    role = settings.role
    heavy = role in {"INDUSTRIAL", "HAULER", "MINING"}
    combat = role == "COMBAT"
    exploration = role == "EXPLORATION"
    ship_scale = settings.global_scale * (0.72 if heavy else (0.66 if combat else (0.60 if exploration else 0.64)))
    hull_count = max(1, settings.hull_sections)

    first_m = _pick_role_hull(hulls, role, rng)
    if first_m is None:
        return 0, "No suitable hull module"
    first = _place_direct(first_m, Vector((0, 0, 0)), ship_scale, "H")
    if first is None:
        return 0, f"Failed to import {first_m.module_id}"
    first["ssy_anchor"] = "HULL_1"
    current_obj, current_m = first, first_m
    created = [first]

    for i in range(1, hull_count):
        next_m = _pick_role_hull(hulls, role, rng) or first_m
        next_obj = _place_direct(next_m, Vector((0, 0, 0)), ship_scale, "H")
        if next_obj is None:
            continue
        p = _find_socket(current_m, "aft")
        c = _find_socket(next_m, "forward")
        if p and c:
            _attach(next_obj, c, current_obj, p, 1.0)
        next_obj["ssy_anchor"] = f"HULL_{i + 1}"
        created.append(next_obj)
        current_obj, current_m = next_obj, next_m
    last, last_m = current_obj, current_m

    # Command module on the forward socket of the first hull.
    command_pool = [m for m in commands if _role_suitable(m.semantic, role)] or commands
    if command_pool:
        cm = rng.choice(command_pool)
        co = _place_direct(cm, Vector((0, 0, 0)), ship_scale, "C")
        if co:
            # Open-bottom command pieces seat on the hull dorsal surface.
            # Inline/nose command pieces retain forward/aft assembly behavior.
            cs = _find_socket(cm, "mount")
            ps = _find_socket(first_m, "dorsal") if cs else None
            if ps and cs and _can_mate(ps.type, cs.type):
                _attach(co, cs, first, ps, 0.55)
            else:
                ps, cs = _find_socket(first_m, "forward"), _find_socket(cm, "aft")
                if ps and cs:
                    _attach(co, cs, first, ps, 0.75)
            co["ssy_anchor"] = "COMMAND"
            created.append(co)

    # Seat engines inside aft-hull engine cavities.
    engine_slots = ["engine_port", "engine_starboard"]
    engine_target_count = min(2, max(1, settings.engine_count))
    if heavy or combat:
        engine_target_count = min(2, max(engine_target_count, 2))
    for i in range(engine_target_count):
        if not engines:
            break
        em = rng.choice(engines)
        eo = _place_direct(em, Vector((0, 0, 0)), ship_scale, "E")
        if not eo:
            continue
        ps, cs = _find_socket(last_m, engine_slots[i]), _find_socket(em, "mount")
        if ps and cs:
            _attach(eo, cs, last, ps, 0.40)
        eo["ssy_anchor"] = "DRIVE_PORT" if i == 0 else "DRIVE_STARBOARD"
        created.append(eo)

    # Calculate approximate hull envelope using current design parts.
    min_y, max_y = float("inf"), float("-inf")
    half_width, top_z = 0.0, 0.0
    for obj in created:
        m = _module_for_object(obj)
        if not m or m.module_class != "hull":
            continue
        sx, sy, sz = [abs(v) for v in obj.scale]
        half_width = max(half_width, abs(obj.location.x) + m.half_width * sx)
        top_z = max(top_z, obj.location.z + m.half_height * sz)
        min_y = min(min_y, obj.location.y - m.half_length * sy)
        max_y = max(max_y, obj.location.y + m.half_length * sy)
    if min_y > max_y:
        min_y, max_y = -first_m.half_length * ship_scale, first_m.half_length * ship_scale
    y_center = (min_y + max_y) * 0.5

    hp_count = settings.hardpoint_count
    if hp_count < 0:
        hp_count = 4 if combat else (3 if role == "MINING" else 2)
    for i in range(max(0, hp_count)):
        if not hardpoints:
            break
        hm = rng.choice(hardpoints)
        side = 1.0 if i & 1 else -1.0
        x = side * max(0.5, half_width) * (0.52 if hp_count > 2 else 0.34)
        y = y_center + ((i // 2) - 0.5) * hm.half_length * ship_scale
        z = max(0.2, top_z) + hm.half_height * ship_scale * 0.92
        ho = _place_direct(hm, Vector((x, y, z)), ship_scale, "P")
        if ho:
            ho["ssy_anchor"] = f"HARDPOINT_{i + 1}"
            created.append(ho)
            # Add a gameplay equipment-slot marker at the visible hardpoint.
            marker = _add_equipment_slot_marker(ho, "Turret" if combat else ("MiningLaser" if role == "MINING" else "PrimaryWeapon"), 2 if heavy else 1)
            if marker:
                marker.location = Vector((0, 0, hm.half_height))

    detail_count = max(0, int(round(settings.detail_density * 4.0)))
    for i in range(detail_count):
        if not details:
            break
        dm = rng.choice(details)
        ds = ship_scale * 0.38
        x = (-1.0 if i % 2 == 0 else 1.0) * max(0.25, half_width * (0.22 + 0.05 * (i // 2)))
        y = y_center + ((i // 2) * 0.35 * max(0.25, first_m.half_length * ship_scale))
        z = max(0.2, top_z) + dm.half_height * ds * 0.92
        do = _place_direct(dm, Vector((x, y, z)), ds, "D")
        if do:
            do["ssy_anchor"] = f"DETAIL_{i + 1}"
            created.append(do)

    # Designer-enhanced mode adds explicit wings/sensor silhouettes while still
    # using the same module catalog and exported module transforms.
    if settings.designer_enhanced and wings:
        wm = rng.choice(wings)
        for side_name in ("port", "starboard"):
            wo = _place_direct(wm, Vector((0, 0, 0)), ship_scale, "W")
            if not wo:
                continue
            ps, cs = _find_socket(first_m, side_name), _find_socket(wm, "mount")
            if ps and cs:
                _attach(wo, cs, first, ps, 0.4)
                if side_name == "port":
                    wo.scale.x *= -1.0
            wo["ssy_anchor"] = f"WING_{side_name.upper()}"
            created.append(wo)
    if settings.designer_enhanced and role in {"EXPLORATION", "MINING"} and details:
        sensor_pool = [m for m in details if m.semantic == "SENSOR"] or details
        sm = rng.choice(sensor_pool)
        so = _place_direct(sm, Vector((0, y_center, max(0.2, top_z) + sm.half_height * ship_scale * 0.5)), ship_scale * 0.55, "S")
        if so:
            so["ssy_anchor"] = "SENSOR_MAST"
            _add_equipment_slot_marker(so, "Scanner", 1)
            created.append(so)

    # Optional adapter between the command section and first hull, exposed as a
    # generation toggle because the current pack has limited adapter variety.
    if settings.designer_enhanced and settings.use_adapter and adapters:
        root["ssy_generator_adapter_hint"] = rng.choice(adapters).module_id

    _apply_ship_colors(context)
    for obj in created:
        if settings.show_sockets:
            _create_socket_markers(obj)
    return len(created), f"Generated {len(created)} module placements for {role.title()} seed {settings.seed}"


def _ensure_material(name: str, rgba: Sequence[float]):
    mat = bpy.data.materials.get(name)
    if mat is None:
        mat = bpy.data.materials.new(name)
    mat.diffuse_color = tuple(rgba)
    try:
        mat.use_nodes = True
        bsdf = mat.node_tree.nodes.get("Principled BSDF") if mat.node_tree else None
        if bsdf:
            bsdf.inputs["Base Color"].default_value = tuple(rgba)
            bsdf.inputs["Metallic"].default_value = 0.7
            bsdf.inputs["Roughness"].default_value = 0.32
    except Exception:
        pass
    return mat


def _apply_ship_colors(context):
    s = context.scene.ssy_settings
    palette = [
        _ensure_material("SSY_PAINT_PRIMARY", (*s.primary_color[:3], 1.0)),
        _ensure_material("SSY_PAINT_SECONDARY", (*s.secondary_color[:3], 1.0)),
        _ensure_material("SSY_PAINT_ACCENT", (*s.accent_color[:3], 1.0)),
    ]
    for obj in _ship_modules():
        if obj.type != "MESH":
            continue
        if len(obj.data.materials) == 0:
            obj.data.materials.append(palette[0])
        else:
            # Preserve the source material-region topology. Repaint only the
            # two hull paint channels; dark metals, glass and emissive regions
            # retain their authored assignments.
            for i, material in enumerate(obj.data.materials):
                source_name = material.name if material else ""
                if source_name.endswith("Mat_Main") or source_name == "Mat_Main":
                    obj.data.materials[i] = palette[0]
                elif source_name.endswith("Mat_Seco") or source_name == "Mat_Seco":
                    obj.data.materials[i] = palette[1]
            if len(obj.data.materials) == 1:
                obj.data.materials[0] = palette[0]
        # Keep authored extra material slots intact; use explicit custom color
        # metadata for the runtime even when the source mesh only has one slot.
        obj["ssy_paint_primary"] = list(s.primary_color[:3])
        obj["ssy_paint_secondary"] = list(s.secondary_color[:3])
        obj["ssy_paint_accent"] = list(s.accent_color[:3])


def _add_equipment_slot_marker(parent: bpy.types.Object, equipment_type: str, max_size: int) -> Optional[bpy.types.Object]:
    if parent is None:
        return None
    marker = bpy.data.objects.new(f"SLOT_{parent.get('ssy_instance_id', 'M')}_{len(_equipment_markers()) + 1:02d}", None)
    marker.empty_display_type = "CIRCLE"
    marker.empty_display_size = 0.18
    marker.parent = parent
    marker["ssy_marker"] = "equipment_slot"
    marker["slot_id"] = marker.name
    marker["allowed_type"] = equipment_type
    marker["max_size"] = int(max_size)
    marker["mount_name"] = str(parent.get("ssy_anchor", parent.get("ssy_instance_id", "mount")))
    _ensure_collection(MARKER_COLLECTION_NAME).objects.link(marker)
    return marker


def _serialize_transform(obj: bpy.types.Object):
    e = obj.rotation_euler
    mirror_x = float(obj.scale.x) < 0.0
    return {
        "position": [round(float(obj.location.x), 6), round(float(obj.location.y), 6), round(float(obj.location.z), 6)],
        "rotationEulerDeg": [round(math.degrees(float(e.x)), 6), round(math.degrees(float(e.y)), 6), round(math.degrees(float(e.z)), 6)],
        "scale": [round(abs(float(obj.scale.x)), 6), round(float(obj.scale.y), 6), round(float(obj.scale.z), 6)],
        "mirrorX": mirror_x,
    }


def _serialize_slots(obj: bpy.types.Object):
    slots = []
    for marker in obj.children:
        if marker.get("ssy_marker") != "equipment_slot":
            continue
        e = marker.rotation_euler
        slots.append({
            "id": str(marker.get("slot_id", marker.name)),
            "allowedType": str(marker.get("allowed_type", "PrimaryWeapon")),
            "maxSize": int(marker.get("max_size", 1)),
            "mountName": str(marker.get("mount_name", "")),
            "position": [round(float(v), 6) for v in marker.location],
            "rotationEulerDeg": [round(math.degrees(float(v)), 6) for v in e],
        })
    return slots


def _design_dict(context) -> dict:
    s = context.scene.ssy_settings
    modules = []
    for obj in _ship_modules():
        m = _module_for_object(obj)
        if m is None:
            continue
        modules.append({
            "instanceId": str(obj.get("ssy_instance_id", obj.name)),
            "moduleId": m.module_id,
            "moduleClass": m.module_class,
            "semantic": m.semantic,
            "size": m.size,
            "anchor": str(obj.get("ssy_anchor", "")),
            "transform": _serialize_transform(obj),
            "connection": {
                "parentInstanceId": str(obj.get("ssy_parent_instance", "")),
                "parentSocket": str(obj.get("ssy_parent_socket", "")),
                "childSocket": str(obj.get("ssy_child_socket", "")),
            },
            "equipmentSlots": _serialize_slots(obj),
        })
    return {
        "schema": DESIGN_SCHEMA,
        "version": DESIGN_VERSION,
        "tool": {"name": "Codename Subspace Shipyard", "version": ADDON_VERSION},
        "coordinateSystem": {"right": "+X", "forward": "+Y", "up": "+Z", "units": "meters", "sourceObj": "X-right/Y-up/+Z-forward", "sourceToShipyard": "(X,Z,Y)"},
        "ship": {
            "name": s.ship_name,
            "author": s.author,
            "role": s.role,
            "seed": s.seed,
            "decalCode": s.decal_code,
            "orientation": {
                "canonicalForward": "+Y",
                "visualYawDeg": float(context.scene.get("ssy_forward_visual_yaw", 180.0)),
                "authority": str(context.scene.get("ssy_forward_authority", "COCKPIT")),
                "cockpitModuleIndex": int(context.scene.get("ssy_cockpit_module_index", -1)),
            },
            "appearance": {
                "primary": [round(float(v), 6) for v in s.primary_color[:3]],
                "secondary": [round(float(v), 6) for v in s.secondary_color[:3]],
                "accent": [round(float(v), 6) for v in s.accent_color[:3]],
            },
            "generation": {
                "hullSections": s.hull_sections,
                "engineCount": s.engine_count,
                "hardpointCount": s.hardpoint_count,
                "detailDensity": s.detail_density,
                "designerEnhanced": s.designer_enhanced,
                "globalScale": s.global_scale,
            },
        },
        "modules": modules,
    }


def _validate_design(context) -> List[str]:
    issues: List[str] = []
    mods = _ship_modules()
    if not mods:
        return ["No ship modules are present"]
    instances = set()
    has_command = False
    has_drive = False
    for obj in mods:
        instance_id = str(obj.get("ssy_instance_id", ""))
        if not instance_id:
            issues.append(f"{obj.name}: missing instance id")
        elif instance_id in instances:
            issues.append(f"{obj.name}: duplicate instance id {instance_id}")
        instances.add(instance_id)
        m = _module_for_object(obj)
        if m is None:
            issues.append(f"{obj.name}: module id not found in loaded catalog")
            continue
        has_command |= m.module_class == "command"
        has_drive |= m.semantic in {"MAIN_ENGINE", "ENGINE_NOZZLE", "RCS_THRUSTER"}
        if obj.get("ssy_uniform_scale_required"):
            sx, sy, sz = [abs(float(v)) for v in obj.scale]
            if max(sx, sy, sz) - min(sx, sy, sz) > 0.0001:
                issues.append(f"{instance_id}: non-uniform structural scale is forbidden")
        parent_id = str(obj.get("ssy_parent_instance", ""))
        if parent_id and parent_id not in instances and not any(str(x.get("ssy_instance_id", "")) == parent_id for x in mods):
            issues.append(f"{instance_id}: missing parent instance {parent_id}")
    if not has_command:
        issues.append("Ship has no command/cockpit/bridge module")
    if not has_drive:
        issues.append("Ship has no propulsion module")
    # Equipment slots must stay parented to a real module.
    for marker in _equipment_markers():
        if not marker.parent or not marker.parent.get("ssy_design_part"):
            issues.append(f"{marker.name}: equipment slot is not parented to a ship module")
    return issues


def _ship_world_bounds() -> Optional[Tuple[Vector, Vector]]:
    points = []
    for obj in _ship_modules():
        if obj.type != "MESH":
            continue
        for corner in obj.bound_box:
            points.append(obj.matrix_world @ Vector(corner))
    if not points:
        return None
    mn = Vector((min(p.x for p in points), min(p.y for p in points), min(p.z for p in points)))
    mx = Vector((max(p.x for p in points), max(p.y for p in points), max(p.z for p in points)))
    return mn, mx


def _preview_setup(context) -> Tuple[bool, str]:
    bounds = _ship_world_bounds()
    if bounds is None:
        return False, "No ship geometry to preview"
    mn, mx = bounds
    center = (mn + mx) * 0.5
    radius = max(1.0, (mx - mn).length * 0.65)
    scene = context.scene
    world = scene.world or bpy.data.worlds.new("SSY_PREVIEW_WORLD")
    scene.world = world
    world.color = (0.004, 0.006, 0.012)

    cam = bpy.data.objects.get(PREVIEW_CAMERA_NAME)
    if cam is None:
        cam_data = bpy.data.cameras.new(PREVIEW_CAMERA_NAME)
        cam = bpy.data.objects.new(PREVIEW_CAMERA_NAME, cam_data)
        scene.collection.objects.link(cam)
    cam.location = center + Vector((radius * 1.35, -radius * 1.55, radius * 0.85))
    direction = center - cam.location
    cam.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()
    cam.data.lens = 52
    scene.camera = cam

    for name, offset, energy, size in [
        ("SSY_KEY", Vector((radius, -radius * 0.5, radius * 1.2)), 1300, radius * 0.8),
        ("SSY_FILL", Vector((-radius, -radius * 0.2, radius * 0.35)), 700, radius * 1.1),
        ("SSY_RIM", Vector((0, radius, radius)), 1000, radius * 0.7),
    ]:
        light = bpy.data.objects.get(name)
        if light is None:
            data = bpy.data.lights.new(name, type="AREA")
            light = bpy.data.objects.new(name, data)
            scene.collection.objects.link(light)
        light.location = center + offset
        light.data.energy = energy
        light.data.shape = "DISK"
        light.data.size = size
        light.rotation_euler = (center - light.location).to_track_quat("-Z", "Y").to_euler()
    scene.render.resolution_x = 768
    scene.render.resolution_y = 768
    scene.render.resolution_percentage = 100
    scene.render.film_transparent = True
    try:
        scene.render.engine = "BLENDER_EEVEE_NEXT"
    except Exception:
        pass
    return True, "Preview camera and lighting configured"


def _load_design(context, path: Path) -> Tuple[int, str]:
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except Exception as exc:
        return 0, f"Could not read design: {exc}"
    if data.get("schema") != DESIGN_SCHEMA:
        return 0, f"Unsupported design schema: {data.get('schema')}"
    if not CATALOG:
        count, message = _load_catalog(context)
        if count == 0:
            return 0, message
    _clear_ship()
    settings = context.scene.ssy_settings
    ship = data.get("ship", {})
    settings.ship_name = str(ship.get("name", settings.ship_name))
    settings.author = str(ship.get("author", settings.author))
    if ship.get("role") in {x[0] for x in ROLE_ITEMS}:
        settings.role = ship["role"]
    settings.seed = int(ship.get("seed", settings.seed))
    settings.decal_code = str(ship.get("decalCode", settings.decal_code))
    appearance = ship.get("appearance", {})
    for key, attr in (("primary", "primary_color"), ("secondary", "secondary_color"), ("accent", "accent_color")):
        c = appearance.get(key)
        if isinstance(c, list) and len(c) >= 3:
            setattr(settings, attr, (float(c[0]), float(c[1]), float(c[2]), 1.0))

    orientation = ship.get("orientation", {})
    context.scene["ssy_forward_visual_yaw"] = float(orientation.get("visualYawDeg", 180.0))
    context.scene["ssy_forward_authority"] = str(orientation.get("authority", "COCKPIT"))
    context.scene["ssy_cockpit_module_index"] = int(orientation.get("cockpitModuleIndex", -1))

    created = {}
    module_records = data.get("modules", [])
    for rec in module_records:
        module_id = rec.get("moduleId", "")
        m = CATALOG.get(module_id)
        if m is None:
            continue
        t = rec.get("transform", {})
        pos = t.get("position", [0, 0, 0])
        scale = t.get("scale", [1, 1, 1])
        obj = _instantiate_module(m, tuple(pos[:3]), 1.0, str(rec.get("instanceId", _next_instance_id())))
        if not obj:
            continue
        rot = t.get("rotationEulerDeg", [0, 0, 0])
        obj.rotation_euler = tuple(math.radians(float(v)) for v in rot[:3])
        sx = abs(float(scale[0])) * (-1.0 if bool(t.get("mirrorX", False)) else 1.0)
        obj.scale = (sx, float(scale[1]), float(scale[2]))
        obj["ssy_anchor"] = str(rec.get("anchor", ""))
        conn = rec.get("connection", {})
        obj["ssy_parent_instance"] = str(conn.get("parentInstanceId", ""))
        obj["ssy_parent_socket"] = str(conn.get("parentSocket", ""))
        obj["ssy_child_socket"] = str(conn.get("childSocket", ""))
        for slot in rec.get("equipmentSlots", []):
            marker = _add_equipment_slot_marker(obj, str(slot.get("allowedType", "PrimaryWeapon")), int(slot.get("maxSize", 1)))
            if marker:
                marker["slot_id"] = str(slot.get("id", marker.name))
                marker["mount_name"] = str(slot.get("mountName", ""))
                marker.location = Vector(slot.get("position", [0, 0, 0])[:3])
                rr = slot.get("rotationEulerDeg", [0, 0, 0])
                marker.rotation_euler = tuple(math.radians(float(v)) for v in rr[:3])
        created[str(obj.get("ssy_instance_id"))] = obj
    _apply_ship_colors(context)
    return len(created), f"Loaded {len(created)} module placements from {path.name}"


class SSY_Settings(PropertyGroup):
    project_root: StringProperty(name="Project / Certified Folder", subtype="DIR_PATH", description="Codename Subspace repository root, greyoxide_shipyard_v07 folder, or certified folder")
    catalog_file: StringProperty(name="Catalog Override", subtype="FILE_PATH", description="Optional direct path to certified_module_catalog.csv")
    module_choice: EnumProperty(name="Module", items=_module_enum_items)
    role: EnumProperty(name="Role", items=ROLE_ITEMS, default="INDUSTRIAL")
    ship_name: StringProperty(name="Ship Name", default="New Ship")
    author: StringProperty(name="Author", default="")
    seed: IntProperty(name="Seed", default=425026, min=0, max=2147483647)
    hull_sections: IntProperty(name="Hull Sections", default=3, min=1, max=8)
    engine_count: IntProperty(name="Engines", default=2, min=1, max=2)
    hardpoint_count: IntProperty(name="Hardpoints", default=-1, min=-1, max=12, description="-1 uses the runtime role default")
    detail_density: FloatProperty(name="Detail Density", default=0.5, min=0.0, max=1.0)
    global_scale: FloatProperty(name="Global Scale", default=1.0, min=0.05, max=10.0)
    designer_enhanced: BoolProperty(name="Designer Enhanced", default=True, description="Add optional wings and role-specific visual pieces beyond the runtime showcase recipe")
    use_adapter: BoolProperty(name="Adapter Hint", default=False)
    clear_before_generate: BoolProperty(name="Clear Existing Ship", default=True)
    show_sockets: BoolProperty(name="Show Generated Sockets", default=False)
    equipment_type: EnumProperty(name="Equipment Type", items=EQUIPMENT_ITEMS, default="PrimaryWeapon")
    equipment_size: IntProperty(name="Max Equipment Size", default=1, min=1, max=5)
    primary_color: FloatVectorProperty(name="Primary", subtype="COLOR", size=4, min=0.0, max=1.0, default=(0.16, 0.22, 0.28, 1.0))
    secondary_color: FloatVectorProperty(name="Secondary", subtype="COLOR", size=4, min=0.0, max=1.0, default=(0.05, 0.07, 0.09, 1.0))
    accent_color: FloatVectorProperty(name="Accent", subtype="COLOR", size=4, min=0.0, max=1.0, default=(0.78, 0.42, 0.08, 1.0))
    decal_code: StringProperty(name="Decal Code", default="SY-001")
    export_path: StringProperty(name="Design JSON", subtype="FILE_PATH", default="//exports/shipyard/new_ship.subspace_shipyard.json")
    icon_path: StringProperty(name="Icon PNG", subtype="FILE_PATH", default="//exports/shipyard/new_ship_icon.png")
    last_status: StringProperty(name="Status", default="Catalog not loaded")


class SSY_OT_load_catalog(Operator):
    bl_idname = "ssy.load_catalog"
    bl_label = "Load Certified Module Catalog"
    bl_options = {"REGISTER"}

    def execute(self, context):
        count, message = _load_catalog(context)
        context.scene.ssy_settings.last_status = message
        self.report({"INFO" if count else "ERROR"}, message)
        return {"FINISHED" if count else "CANCELLED"}


class SSY_OT_add_module(Operator):
    bl_idname = "ssy.add_module"
    bl_label = "Add Module at Cursor"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        s = context.scene.ssy_settings
        if not CATALOG:
            count, msg = _load_catalog(context)
            if count == 0:
                self.report({"ERROR"}, msg)
                return {"CANCELLED"}
        m = CATALOG.get(s.module_choice)
        if not m:
            self.report({"ERROR"}, "Choose a valid module")
            return {"CANCELLED"}
        obj = _instantiate_module(m, tuple(context.scene.cursor.location), s.global_scale)
        if not obj:
            self.report({"ERROR"}, f"Could not import {m.module_id}")
            return {"CANCELLED"}
        bpy.ops.object.select_all(action="DESELECT")
        obj.select_set(True)
        context.view_layer.objects.active = obj
        s.last_status = f"Added {m.module_id}"
        return {"FINISHED"}


class SSY_OT_refresh_sockets(Operator):
    bl_idname = "ssy.refresh_sockets"
    bl_label = "Create / Refresh Socket Markers"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        obj = context.active_object
        if not obj or not obj.get("ssy_design_part"):
            self.report({"ERROR"}, "Select a Shipyard module placement")
            return {"CANCELLED"}
        count = _create_socket_markers(obj)
        self.report({"INFO"}, f"Created {count} runtime-compatible socket markers")
        return {"FINISHED"}


class SSY_OT_snap_selected(Operator):
    bl_idname = "ssy.snap_selected"
    bl_label = "Snap Active Module to Selected Parent"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        child = context.active_object
        parents = [o for o in context.selected_objects if o != child and o.get("ssy_design_part")]
        if not child or not child.get("ssy_design_part") or not parents:
            self.report({"ERROR"}, "Select parent module plus active child module")
            return {"CANCELLED"}
        parent = parents[0]
        cm, pm = _module_for_object(child), _module_for_object(parent)
        if not cm or not pm:
            return {"CANCELLED"}
        choices = []
        for ps in pm.sockets:
            pp, _ = _socket_world(parent, ps)
            for cs in cm.sockets:
                if not _can_mate(ps.type, cs.type):
                    continue
                cp, _ = _socket_world(child, cs)
                choices.append(((pp - cp).length, ps, cs))
        if not choices:
            self.report({"ERROR"}, "No compatible socket pair between selected modules")
            return {"CANCELLED"}
        _, ps, cs = min(choices, key=lambda x: x[0])
        _attach(child, cs, parent, ps, 1.0)
        self.report({"INFO"}, f"Snapped {cs.name} to {ps.name}")
        return {"FINISHED"}


class SSY_OT_add_equipment_slot(Operator):
    bl_idname = "ssy.add_equipment_slot"
    bl_label = "Add Equipment Slot at Cursor"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        parent = context.active_object
        if not parent or not parent.get("ssy_design_part"):
            self.report({"ERROR"}, "Select the module that owns this equipment slot")
            return {"CANCELLED"}
        s = context.scene.ssy_settings
        marker = _add_equipment_slot_marker(parent, s.equipment_type, s.equipment_size)
        if marker is None:
            return {"CANCELLED"}
        marker.location = parent.matrix_world.inverted() @ context.scene.cursor.location
        self.report({"INFO"}, f"Added {s.equipment_type} slot to {parent.get('ssy_instance_id')}")
        return {"FINISHED"}


class SSY_OT_apply_colors(Operator):
    bl_idname = "ssy.apply_colors"
    bl_label = "Apply Ship Paint"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        _apply_ship_colors(context)
        return {"FINISHED"}


class SSY_OT_generate_ship(Operator):
    bl_idname = "ssy.generate_ship"
    bl_label = "Generate Ship"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        count, message = _generate_ship(context)
        context.scene.ssy_settings.last_status = message
        self.report({"INFO" if count else "ERROR"}, message)
        return {"FINISHED" if count else "CANCELLED"}


class SSY_OT_randomize_seed(Operator):
    bl_idname = "ssy.randomize_seed"
    bl_label = "Randomize Seed"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        context.scene.ssy_settings.seed = random.SystemRandom().randint(1, 2147483647)
        return {"FINISHED"}


class SSY_OT_clear_ship(Operator):
    bl_idname = "ssy.clear_ship"
    bl_label = "Clear Ship"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        _clear_ship()
        return {"FINISHED"}


class SSY_OT_validate_ship(Operator):
    bl_idname = "ssy.validate_ship"
    bl_label = "Validate Ship"
    bl_options = {"REGISTER"}

    def execute(self, context):
        issues = _validate_design(context)
        if issues:
            text = bpy.data.texts.get("Subspace Shipyard Validation") or bpy.data.texts.new("Subspace Shipyard Validation")
            text.clear()
            text.write("Subspace Shipyard validation issues:\n\n" + "\n".join(f"- {x}" for x in issues))
            context.scene.ssy_settings.last_status = f"Validation: {len(issues)} issue(s)"
            self.report({"WARNING"}, f"Validation found {len(issues)} issue(s); see Text Editor")
        else:
            context.scene.ssy_settings.last_status = "Validation passed"
            self.report({"INFO"}, "Shipyard validation passed")
        return {"FINISHED"}


class SSY_OT_export_design(Operator):
    bl_idname = "ssy.export_design"
    bl_label = "Export Ship Design JSON"
    bl_options = {"REGISTER"}

    filepath: StringProperty(subtype="FILE_PATH")

    def invoke(self, context, event):
        self.filepath = _norm(context.scene.ssy_settings.export_path)
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

    def execute(self, context):
        path = Path(self.filepath)
        if path.suffix.lower() != ".json":
            path = path.with_suffix(".subspace_shipyard.json")
        path.parent.mkdir(parents=True, exist_ok=True)
        data = _design_dict(context)
        path.write_text(json.dumps(data, indent=2), encoding="utf-8")
        context.scene.ssy_settings.export_path = str(path)
        context.scene.ssy_settings.last_status = f"Exported {path.name}"
        self.report({"INFO"}, f"Exported {len(data['modules'])} modules to {path}")
        return {"FINISHED"}


class SSY_OT_import_design(Operator):
    bl_idname = "ssy.import_design"
    bl_label = "Import Ship Design JSON"
    bl_options = {"REGISTER", "UNDO"}

    filepath: StringProperty(subtype="FILE_PATH")

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

    def execute(self, context):
        count, msg = _load_design(context, Path(self.filepath))
        context.scene.ssy_settings.last_status = msg
        self.report({"INFO" if count else "ERROR"}, msg)
        return {"FINISHED" if count else "CANCELLED"}


class SSY_OT_preview_setup(Operator):
    bl_idname = "ssy.preview_setup"
    bl_label = "Setup Real-Time Preview"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        ok, msg = _preview_setup(context)
        self.report({"INFO" if ok else "ERROR"}, msg)
        return {"FINISHED" if ok else "CANCELLED"}


class SSY_OT_render_icon(Operator):
    bl_idname = "ssy.render_icon"
    bl_label = "Render Blueprint / Item Icon"
    bl_options = {"REGISTER"}

    filepath: StringProperty(subtype="FILE_PATH")

    def invoke(self, context, event):
        self.filepath = _norm(context.scene.ssy_settings.icon_path)
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

    def execute(self, context):
        ok, msg = _preview_setup(context)
        if not ok:
            self.report({"ERROR"}, msg)
            return {"CANCELLED"}
        path = Path(self.filepath)
        if path.suffix.lower() != ".png":
            path = path.with_suffix(".png")
        path.parent.mkdir(parents=True, exist_ok=True)
        context.scene.render.filepath = str(path)
        context.scene.render.resolution_x = 512
        context.scene.render.resolution_y = 512
        context.scene.render.film_transparent = True
        bpy.ops.render.render(write_still=True)
        context.scene.ssy_settings.icon_path = str(path)
        self.report({"INFO"}, f"Rendered ship icon to {path}")
        return {"FINISHED"}


class SSY_OT_export_preview_glb(Operator):
    bl_idname = "ssy.export_preview_glb"
    bl_label = "Export Preview GLB"
    bl_options = {"REGISTER"}

    filepath: StringProperty(subtype="FILE_PATH")

    def invoke(self, context, event):
        self.filepath = _norm("//exports/shipyard/ship_preview.glb")
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

    def execute(self, context):
        path = Path(self.filepath)
        if path.suffix.lower() != ".glb":
            path = path.with_suffix(".glb")
        path.parent.mkdir(parents=True, exist_ok=True)
        bpy.ops.object.select_all(action="DESELECT")
        mods = _ship_modules()
        for o in mods:
            o.select_set(True)
        if not mods:
            self.report({"ERROR"}, "No ship modules to export")
            return {"CANCELLED"}
        context.view_layer.objects.active = mods[0]
        try:
            bpy.ops.export_scene.gltf(filepath=str(path), export_format="GLB", use_selection=True)
        except TypeError:
            bpy.ops.export_scene.gltf(filepath=str(path), export_format="GLB", export_selected=True)
        self.report({"INFO"}, f"Exported preview GLB to {path}")
        return {"FINISHED"}


class SSY_PT_project(Panel):
    bl_label = "Subspace Shipyard"
    bl_idname = "SSY_PT_project"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Subspace Shipyard"

    def draw(self, context):
        s = context.scene.ssy_settings
        layout = self.layout
        layout.prop(s, "project_root")
        layout.prop(s, "catalog_file")
        row = layout.row(align=True)
        row.operator("ssy.load_catalog", icon="FILE_REFRESH")
        row.label(text=f"{len(CATALOG)} modules")
        box = layout.box()
        box.label(text=s.last_status, icon="INFO")


class SSY_PT_author(Panel):
    bl_label = "Module Authoring & Assembly"
    bl_idname = "SSY_PT_author"
    bl_parent_id = "SSY_PT_project"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"

    def draw(self, context):
        s = context.scene.ssy_settings
        layout = self.layout
        layout.prop(s, "module_choice")
        m = CATALOG.get(s.module_choice)
        if m is not None:
            info = layout.box()
            info.label(text=f"{m.module_class.upper()} / {m.semantic} / {m.size}")
            info.label(text=f"W {m.half_width*2.0:.2f}  L {m.half_length*2.0:.2f}  H {m.half_height*2.0:.2f}")
            if m.attachment_face:
                info.label(text=f"Mount hint: {m.attachment_face} ({m.attachment_confidence:.0%})", icon="SNAP_ON")
        layout.operator("ssy.add_module", icon="ADD")
        row = layout.row(align=True)
        row.operator("ssy.refresh_sockets", icon="EMPTY_ARROWS")
        row.operator("ssy.snap_selected", icon="SNAP_ON")
        layout.separator()
        layout.label(text="Equipment slot on selected module")
        layout.prop(s, "equipment_type")
        layout.prop(s, "equipment_size")
        layout.operator("ssy.add_equipment_slot", icon="EMPTY_DATA")


class SSY_PT_generator(Panel):
    bl_label = "Procedural Ship Generator"
    bl_idname = "SSY_PT_generator"
    bl_parent_id = "SSY_PT_project"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"

    def draw(self, context):
        s = context.scene.ssy_settings
        layout = self.layout
        layout.prop(s, "ship_name")
        layout.prop(s, "role")
        row = layout.row(align=True)
        row.prop(s, "seed")
        row.operator("ssy.randomize_seed", text="", icon="FILE_REFRESH")
        layout.prop(s, "hull_sections")
        layout.prop(s, "engine_count")
        layout.prop(s, "hardpoint_count")
        layout.prop(s, "detail_density")
        layout.prop(s, "global_scale")
        layout.prop(s, "designer_enhanced")
        if s.designer_enhanced:
            layout.prop(s, "use_adapter")
        layout.prop(s, "show_sockets")
        layout.prop(s, "clear_before_generate")
        row = layout.row(align=True)
        row.scale_y = 1.4
        row.operator("ssy.generate_ship", icon="MOD_ARRAY")
        row.operator("ssy.clear_ship", text="Clear", icon="TRASH")


class SSY_PT_appearance(Panel):
    bl_label = "Paint, Decals & Preview"
    bl_idname = "SSY_PT_appearance"
    bl_parent_id = "SSY_PT_project"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"

    def draw(self, context):
        s = context.scene.ssy_settings
        layout = self.layout
        layout.prop(s, "primary_color")
        layout.prop(s, "secondary_color")
        layout.prop(s, "accent_color")
        layout.prop(s, "decal_code")
        layout.operator("ssy.apply_colors", icon="MATERIAL")
        row = layout.row(align=True)
        row.operator("ssy.preview_setup", icon="CAMERA_DATA")
        row.operator("ssy.render_icon", icon="RENDER_STILL")


class SSY_PT_export(Panel):
    bl_label = "Validate & Export"
    bl_idname = "SSY_PT_export"
    bl_parent_id = "SSY_PT_project"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"

    def draw(self, context):
        s = context.scene.ssy_settings
        layout = self.layout
        layout.prop(s, "author")
        layout.operator("ssy.validate_ship", icon="CHECKMARK")
        layout.prop(s, "export_path")
        row = layout.row(align=True)
        row.operator("ssy.export_design", icon="EXPORT")
        row.operator("ssy.import_design", icon="IMPORT")
        layout.operator("ssy.export_preview_glb", icon="MESH_DATA")
        layout.label(text="JSON is authoritative; GLB is preview/reference only")


CLASSES = [
    SSY_Settings,
    SSY_OT_load_catalog,
    SSY_OT_add_module,
    SSY_OT_refresh_sockets,
    SSY_OT_snap_selected,
    SSY_OT_add_equipment_slot,
    SSY_OT_apply_colors,
    SSY_OT_generate_ship,
    SSY_OT_randomize_seed,
    SSY_OT_clear_ship,
    SSY_OT_validate_ship,
    SSY_OT_export_design,
    SSY_OT_import_design,
    SSY_OT_preview_setup,
    SSY_OT_render_icon,
    SSY_OT_export_preview_glb,
    SSY_PT_project,
    SSY_PT_author,
    SSY_PT_generator,
    SSY_PT_appearance,
    SSY_PT_export,
]


def register():
    for cls in CLASSES:
        bpy.utils.register_class(cls)
    bpy.types.Scene.ssy_settings = PointerProperty(type=SSY_Settings)


def unregister():
    if hasattr(bpy.types.Scene, "ssy_settings"):
        del bpy.types.Scene.ssy_settings
    for cls in reversed(CLASSES):
        try:
            bpy.utils.unregister_class(cls)
        except RuntimeError:
            pass
    CATALOG.clear()
    TEMPLATE_OBJECTS.clear()


if __name__ == "__main__":
    register()
