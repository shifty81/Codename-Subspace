"""Metadata utilities for PCG Exporter addon."""
import bpy


def build_snap_json(obj):
    """Build snap point JSON from object children."""
    snap_points = []
    for child in obj.children:
        if child.get("snap", False):
            snap = {
                "id": child.name,
                "position": list(child.location),
                "rotation": list(child.rotation_euler),
                "compatible_tags": child.get("tags", []),
                "snap_type": child.get("snap_type", "universal"),
                "max_connections": child.get("max_connections", 1),
                "strength": child.get("strength", 1.0),
                "alignment_constraint": child.get("alignment_constraint", "free"),
                "attach_offset": child.get("attach_offset", [0.0, 0.0, 0.0]),
                "mirror_id": child.get("mirror_id", "")
            }
            snap_points.append(snap)

    return {
        "asset": {
            "name": obj.name,
            "type": obj.get("type", "module"),
            "category": obj.get("category", "general"),
            "unit_scale": obj.get("unit_scale", 1.0)
        },
        "snap_points": snap_points
    }


def build_metadata_json(obj, lod_levels=3):
    """Build metadata JSON for an object."""
    bbox = obj.bound_box
    min_corner = [min([v[i] for v in bbox]) for i in range(3)]
    max_corner = [max([v[i] for v in bbox]) for i in range(3)]
    volume = (max_corner[0] - min_corner[0]) * (max_corner[1] - min_corner[1]) * (max_corner[2] - min_corner[2])

    return {
        "name": obj.name,
        "category": obj.get("category", "general"),
        "bounding_box": {"min": min_corner, "max": max_corner},
        "volume": volume,
        "lod_levels": lod_levels,
        "collision_mesh": obj.get("collision_mesh", ""),
        "material_tags": obj.get("material_tags", []),
        "custom_properties": {k: v for k, v in obj.items() if not k.startswith("_")}
    }


def build_adjacency_heatmap(collection):
    """Build adjacency matrix for structural intelligence learning."""
    adjacency_data = []

    objects = [obj for obj in collection.objects if obj.type == 'MESH']

    for i, obj1 in enumerate(objects):
        for j, obj2 in enumerate(objects):
            if i >= j:
                continue

            # Calculate distance between objects
            distance = (obj1.location - obj2.location).length

            # Objects within 5 units are considered adjacent
            if distance < 5.0:
                weight = 1.0 - (distance / 5.0)  # Closer = higher weight
                adjacency_data.append([obj1.name, obj2.name, round(weight, 2)])

    return adjacency_data


def build_warp_mining_metadata(obj):
    """Build warp mining zone metadata for planets/asteroids."""
    if not obj.get("warp_mining_zone", False):
        return None

    return {
        "zone_name": obj.name,
        "center_object": obj.get("center_object", obj.name),
        "inner_radius": obj.get("inner_radius", 3000),
        "outer_radius": obj.get("outer_radius", 5000),
        "thickness": obj.get("thickness", 500),
        "density_seed": obj.get("density_seed", 123456789),
        "resource_types": obj.get("resource_types", ["iron", "gold", "platinum"]),
        "npc_spawn_weight": obj.get("npc_spawn_weight", 0.2),
        "player_access": obj.get("player_access", True),
        "snap_points": []
    }
