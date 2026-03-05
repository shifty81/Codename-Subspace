bl_info = {
    "name": "PCG Exporter",
    "author": "Shifty C",
    "version": (1, 0),
    "blender": (3, 5, 0),
    "location": "View3D > Tool Shelf > PCG Exporter",
    "description": "Procedural Content Generation exporter with LOD, collision, snap points, and metadata",
    "category": "Import-Export",
}

import bpy

from .operators.export_project import ExportProjectOperator


# -----------------------------
# Panel
# -----------------------------
class PCG_PT_main_panel(bpy.types.Panel):
    bl_label = "PCG Exporter"
    bl_idname = "PCG_PT_main_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'PCG Exporter'

    def draw(self, context):
        layout = self.layout
        layout.label(text="Procedural Content Export")
        layout.operator("pcg.export_project")


# -----------------------------
# Registration
# -----------------------------
classes = [
    ExportProjectOperator,
    PCG_PT_main_panel
]


def register():
    for cls in classes:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)


if __name__ == "__main__":
    register()
