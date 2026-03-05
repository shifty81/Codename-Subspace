"""Mesh utilities for PCG Exporter addon."""
import bpy
import os


def apply_transforms(obj):
    """Apply all transforms to the object."""
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)


def generate_lods(obj, lod_folder, lod_levels=3):
    """Generate LOD meshes with progressive decimation."""
    os.makedirs(lod_folder, exist_ok=True)

    for level in range(1, lod_levels):
        factor = 1.0 / (2 ** level)  # e.g., LOD1 = 50%, LOD2 = 25%

        # Duplicate object
        obj_copy = obj.copy()
        obj_copy.data = obj.data.copy()
        bpy.context.collection.objects.link(obj_copy)

        # Apply decimate modifier
        decimate = obj_copy.modifiers.new(f"Decimate_LOD{level}", type='DECIMATE')
        decimate.ratio = factor
        bpy.context.view_layer.objects.active = obj_copy
        bpy.ops.object.modifier_apply(modifier=decimate.name)

        # Export
        export_file = os.path.join(lod_folder, f"{obj.name}_LOD{level}.fbx")
        bpy.ops.object.select_all(action='DESELECT')
        obj_copy.select_set(True)
        bpy.ops.export_scene.fbx(filepath=export_file, use_selection=True, apply_unit_scale=True)

        # Remove temp copy
        bpy.data.objects.remove(obj_copy)
