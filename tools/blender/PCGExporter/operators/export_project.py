"""Export project operator for PCG Exporter addon."""
import bpy
import os
import json

from ..utils.file_utils import ensure_folders
from ..utils.mesh_utils import apply_transforms, generate_lods
from ..utils.metadata_utils import (
    build_snap_json,
    build_metadata_json,
    build_adjacency_heatmap,
    build_warp_mining_metadata
)


class ExportProjectOperator(bpy.types.Operator):
    """Batch Export Project for PCG Engine"""
    bl_idname = "pcg.export_project"
    bl_label = "Export Project for PCG"
    bl_options = {'REGISTER', 'UNDO'}

    export_path: bpy.props.StringProperty(
        name="Export Folder",
        description="Root folder for exported project",
        default=os.path.join(os.path.expanduser("~"), "Desktop", "Export_Project"),
        subtype='DIR_PATH'
    )

    lod_levels: bpy.props.IntProperty(
        name="LOD Levels",
        description="Number of LOD tiers to generate",
        default=3,
        min=1,
        max=5
    )

    profile_name: bpy.props.StringProperty(
        name="Project Profile",
        description="Select the active project profile",
        default="default_project_profile.json"
    )

    generate_thumbnails: bpy.props.BoolProperty(
        name="Generate Thumbnails",
        default=True
    )

    export_adjacency: bpy.props.BoolProperty(
        name="Export Adjacency Matrix",
        description="Generate adjacency matrix for structural intelligence",
        default=True
    )

    def execute(self, context):
        root_path = self.export_path
        ensure_folders(root_path)

        # Load active project profile
        profile_path = os.path.join(os.path.dirname(__file__), "../templates", self.profile_name)
        if os.path.exists(profile_path):
            with open(profile_path, 'r') as f:
                self.profile = json.load(f)
        else:
            self.report({'WARNING'}, f"Profile {self.profile_name} not found. Using defaults.")
            self.profile = {}

        # Active collection
        collection = context.view_layer.active_layer_collection.collection

        adjacency_data = []
        warp_mining_zones = []

        for obj in collection.objects:
            if obj.type != 'MESH':
                continue

            apply_transforms(obj)

            # Determine type for folder organization
            obj_type = obj.get("type", "general")
            mesh_folder = os.path.join(root_path, "Meshes", obj_type.capitalize())
            os.makedirs(mesh_folder, exist_ok=True)

            # Export main mesh
            export_file = os.path.join(mesh_folder, f"{obj.name}.fbx")
            bpy.ops.object.select_all(action='DESELECT')
            obj.select_set(True)
            bpy.ops.export_scene.fbx(filepath=export_file, use_selection=True, apply_unit_scale=True)

            # Generate LODs
            lod_folder = os.path.join(root_path, "LOD", obj_type.capitalize())
            generate_lods(obj, lod_folder, self.lod_levels)

            # Export collision mesh
            if "collision" in obj.keys() and obj["collision"]:
                collision_folder = os.path.join(root_path, "Collision", obj_type.capitalize())
                os.makedirs(collision_folder, exist_ok=True)
                collision_file = os.path.join(collision_folder, f"{obj.name}_collision.obj")
                bpy.ops.export_scene.obj(filepath=collision_file, use_selection=True, use_triangles=True)

            # Export snap points JSON
            snap_folder = os.path.join(root_path, "SnapPoints")
            snap_json = build_snap_json(obj)
            snap_file = os.path.join(snap_folder, f"{obj.name}_snap.json")
            with open(snap_file, 'w') as f:
                json.dump(snap_json, f, indent=4)

            # Export metadata JSON
            metadata_folder = os.path.join(root_path, "Metadata")
            metadata_json = build_metadata_json(obj, lod_levels=self.lod_levels)
            metadata_file = os.path.join(metadata_folder, f"{obj.name}_metadata.json")
            with open(metadata_file, 'w') as f:
                json.dump(metadata_json, f, indent=4)

            # Check for warp mining zone
            warp_mining = build_warp_mining_metadata(obj)
            if warp_mining:
                warp_mining_zones.append(warp_mining)

            # Generate thumbnail if requested
            if self.generate_thumbnails:
                self.generate_thumbnail(obj, root_path)

        # Export adjacency matrix if requested
        if self.export_adjacency:
            adjacency_data = build_adjacency_heatmap(collection)
            adjacency_file = os.path.join(root_path, "Metadata", "adjacency_matrix.json")
            with open(adjacency_file, 'w') as f:
                json.dump({
                    "adjacency_matrix": adjacency_data
                }, f, indent=4)

        # Export warp mining zones
        if warp_mining_zones:
            warp_file = os.path.join(root_path, "Metadata", "warp_mining_zones.json")
            with open(warp_file, 'w') as f:
                json.dump({
                    "zones": warp_mining_zones
                }, f, indent=4)

        self.report({'INFO'}, f"Project exported successfully to {root_path}")
        return {'FINISHED'}

    def generate_thumbnail(self, obj, root_path):
        """Generate a thumbnail render for the object."""
        thumbnail_folder = os.path.join(root_path, "Thumbnails")
        os.makedirs(thumbnail_folder, exist_ok=True)

        # Store current render settings
        original_resolution_x = bpy.context.scene.render.resolution_x
        original_resolution_y = bpy.context.scene.render.resolution_y
        original_film_transparent = bpy.context.scene.render.film_transparent

        # Set render settings for thumbnail
        bpy.context.scene.render.resolution_x = 512
        bpy.context.scene.render.resolution_y = 512
        bpy.context.scene.render.film_transparent = True

        # Render thumbnail
        thumbnail_path = os.path.join(thumbnail_folder, f"{obj.name}_thumb.png")
        bpy.context.scene.render.filepath = thumbnail_path

        # Restore original settings
        bpy.context.scene.render.resolution_x = original_resolution_x
        bpy.context.scene.render.resolution_y = original_resolution_y
        bpy.context.scene.render.film_transparent = original_film_transparent

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self)
