bl_info = {
    "name": "NovaForge Asset Generator",
    "author": "Shifty C",
    "version": (0, 2),
    "blender": (3, 5, 0),
    "location": "View3D > Tool Shelf > NovaForge",
    "description": "Procedural asset generation with profiles, materials, and animations",
    "category": "Object",
}

import bpy
import json
import os

PROFILE_FILE = os.path.join(bpy.utils.user_resource('CONFIG'), "nova_profiles.json")
profiles = {}
active_profile_name = None

def load_profiles():
    global profiles, active_profile_name
    if os.path.exists(PROFILE_FILE):
        with open(PROFILE_FILE, 'r') as f:
            profiles = json.load(f)
        active_profile_name = next(iter(profiles)) if profiles else None
    else:
        profiles = {}
        active_profile_name = None

def save_profiles():
    with open(PROFILE_FILE, 'w') as f:
        json.dump(profiles, f, indent=4)

def get_active_profile():
    if active_profile_name and active_profile_name in profiles:
        return profiles[active_profile_name]
    return None

# -----------------------------
# Operators
# -----------------------------
class NF_OT_create_profile(bpy.types.Operator):
    bl_idname = "nf.create_profile"
    bl_label = "Create New Profile"
    profile_name: bpy.props.StringProperty(name="Profile Name")

    def execute(self, context):
        global active_profile_name
        if self.profile_name not in profiles:
            profiles[self.profile_name] = {
                "panel_defaults": {"width": 2.0, "height": 2.0, "depth": 0.1},
                "door_defaults": {"width": 1.0, "height": 2.0, "hinge": "left"},
                "material_presets": {
                    "Metal": {"diffuse": [0.7,0.7,0.7], "specular": [1,1,1]},
                    "Wood": {"diffuse": [0.6,0.4,0.2], "specular": [0.2,0.2,0.2]}
                },
                "animation_presets": {
                    "DoorOpen": {"rotation": [0,0,90], "duration": 1.0},
                    "PanelSlide": {"translation": [0,1,0], "duration": 1.0}
                },
                "export_path": "//assets/"
            }
            active_profile_name = self.profile_name
            save_profiles()
            self.report({'INFO'}, f"Profile '{self.profile_name}' created")
        else:
            self.report({'WARNING'}, f"Profile '{self.profile_name}' already exists")
        return {'FINISHED'}

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self)

class NF_OT_switch_profile(bpy.types.Operator):
    bl_idname = "nf.switch_profile"
    bl_label = "Switch Profile"
    profile_name: bpy.props.StringProperty(name="Profile Name")

    def execute(self, context):
        global active_profile_name
        if self.profile_name in profiles:
            active_profile_name = self.profile_name
            self.report({'INFO'}, f"Switched to profile '{self.profile_name}'")
        else:
            self.report({'WARNING'}, f"Profile '{self.profile_name}' does not exist")
        return {'FINISHED'}

    def invoke(self, context, event):
        return context.window_manager.invoke_props_dialog(self)

class NF_OT_generate_panel(bpy.types.Operator):
    bl_idname = "nf.generate_panel"
    bl_label = "Generate Panel"
    width: bpy.props.FloatProperty(name="Width", default=2.0)
    height: bpy.props.FloatProperty(name="Height", default=2.0)
    depth: bpy.props.FloatProperty(name="Depth", default=0.1)
    material: bpy.props.StringProperty(name="Material", default="Metal")

    def execute(self, context):
        profile = get_active_profile()
        if profile:
            defaults = profile.get("panel_defaults", {})
            self.width = defaults.get("width", self.width)
            self.height = defaults.get("height", self.height)
            self.depth = defaults.get("depth", self.depth)

            materials = profile.get("material_presets", {})
            if self.material not in materials:
                self.material = list(materials.keys())[0] if materials else "Default"

        bpy.ops.mesh.primitive_cube_add(size=1)
        obj = context.active_object
        obj.scale = (self.width/2, self.depth/2, self.height/2)
        obj.name = "Panel"

        # Assign material
        if profile and self.material in profile.get("material_presets", {}):
            mat = bpy.data.materials.new(name=self.material)
            mat.diffuse_color = profile["material_presets"][self.material]["diffuse"] + [1]
            obj.data.materials.append(mat)

        self.report({'INFO'}, f"Panel created with material '{self.material}'")
        return {'FINISHED'}

class NF_OT_generate_door(bpy.types.Operator):
    bl_idname = "nf.generate_door"
    bl_label = "Generate Door"
    width: bpy.props.FloatProperty(name="Width", default=1.0)
    height: bpy.props.FloatProperty(name="Height", default=2.0)
    hinge: bpy.props.EnumProperty(name="Hinge", items=[('LEFT','Left',''),('RIGHT','Right','')])
    material: bpy.props.StringProperty(name="Material", default="Wood")
    animation: bpy.props.StringProperty(name="Animation", default="DoorOpen")

    def execute(self, context):
        profile = get_active_profile()
        if profile:
            defaults = profile.get("door_defaults", {})
            self.width = defaults.get("width", self.width)
            self.height = defaults.get("height", self.height)
            self.hinge = defaults.get("hinge", self.hinge)

            materials = profile.get("material_presets", {})
            if self.material not in materials:
                self.material = list(materials.keys())[0] if materials else "Default"

            animations = profile.get("animation_presets", {})
            if self.animation not in animations:
                self.animation = list(animations.keys())[0] if animations else ""

        bpy.ops.mesh.primitive_cube_add(size=1)
        obj = context.active_object
        obj.scale = (self.width/2, 0.1, self.height/2)
        obj.name = "Door"

        # Material assignment
        if profile:
            mat = bpy.data.materials.new(name=self.material)
            mat.diffuse_color = profile["material_presets"][self.material]["diffuse"] + [1]
            obj.data.materials.append(mat)

            # Placeholder for animation (keyframe creation)
            anim_data = profile["animation_presets"].get(self.animation)
            if anim_data:
                obj.rotation_euler = (0,0,0)
                obj.keyframe_insert(data_path="rotation_euler", frame=1)
                obj.rotation_euler[2] = anim_data.get("rotation",[0,0,90])[2] * (3.14159/180)
                obj.keyframe_insert(data_path="rotation_euler", frame=24*anim_data.get("duration",1.0))

        self.report({'INFO'}, f"Door '{self.animation}' created with material '{self.material}'")
        return {'FINISHED'}

class NF_OT_export_assets(bpy.types.Operator):
    bl_idname = "nf.export_assets"
    bl_label = "Export Assets"

    def execute(self, context):
        profile = get_active_profile()
        export_path = bpy.path.abspath(profile.get("export_path", "//assets/")) if profile else "//assets/"
        if not os.path.exists(export_path):
            os.makedirs(export_path)

        # Export FBX
        bpy.ops.export_scene.fbx(filepath=os.path.join(export_path, "assets.fbx"), use_selection=False)

        # Export JSON of procedural parameters for engine
        json_path = os.path.join(export_path, "assets.json")
        if profile:
            with open(json_path, 'w') as f:
                json.dump(profile, f, indent=4)

        self.report({'INFO'}, f"Assets exported to {export_path}")
        return {'FINISHED'}

# -----------------------------
# Panels
# -----------------------------
class NF_PT_main_panel(bpy.types.Panel):
    bl_label = "NovaForge Asset Tool"
    bl_idname = "NF_PT_main_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = 'NovaForge'

    def draw(self, context):
        layout = self.layout
        layout.label(text=f"Active Profile: {active_profile_name}")
        layout.operator("nf.create_profile")
        layout.operator("nf.switch_profile")
        layout.separator()
        layout.label(text="Procedural Generators")
        layout.operator("nf.generate_panel")
        layout.operator("nf.generate_door")
        layout.separator()
        layout.operator("nf.export_assets")

# -----------------------------
# Registration
# -----------------------------
classes = [
    NF_OT_create_profile,
    NF_OT_switch_profile,
    NF_OT_generate_panel,
    NF_OT_generate_door,
    NF_OT_export_assets,
    NF_PT_main_panel
]

def register():
    for cls in classes:
        bpy.utils.register_class(cls)
    load_profiles()

def unregister():
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)

if __name__ == "__main__":
    register()
