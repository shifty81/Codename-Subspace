use serde::{Deserialize, Serialize};
use std::collections::{HashMap, HashSet};
use subspace_core::StableId;

#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize, Default)]
pub struct Vec3 { pub x: f32, pub y: f32, pub z: f32 }

#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
pub struct Transform {
    pub translation: Vec3,
    pub rotation_degrees: Vec3,
    pub scale: Vec3,
}
impl Default for Transform {
    fn default() -> Self {
        Self { translation: Vec3::default(), rotation_degrees: Vec3::default(), scale: Vec3 { x: 1.0, y: 1.0, z: 1.0 } }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash, Serialize, Deserialize)]
#[serde(transparent)]
pub struct ModuleDefinitionId(pub String);

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum SocketKind { Structural, Utility, Weapon, Propulsion, Interior, Aperture, Universal }

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SocketDefinition {
    pub id: String,
    pub kind: SocketKind,
    pub transform: Transform,
    pub compatible_tags: Vec<String>,
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct MountSurface {
    pub face: String,
    pub point: Vec3,
    pub normal: Vec3,
    pub supporting_area: f32,
    pub confidence: f32,
    pub valid: bool,
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct MountProfile {
    pub id: String,
    pub primary_root: MountSurface,
    pub alternates: Vec<MountSurface>,
    pub placement_role: String,
    pub insertion_depth_meters: f32,
    pub minimum_clearance_meters: f32,
    pub generator_eligible: bool,
    pub paired_placement: bool,
    pub provenance: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PaintZone { pub id: String, pub material_family: String }

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ModuleDefinition {
    pub id: ModuleDefinitionId,
    pub display_name: String,
    pub class: String,
    pub semantic: String,
    pub size_class: String,
    pub tags: Vec<String>,
    pub sockets: Vec<SocketDefinition>,
    pub mount_profile: Option<MountProfile>,
    pub paint_zones: Vec<PaintZone>,
    pub provenance: String,
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct ModuleInstance {
    pub id: StableId,
    pub definition: ModuleDefinitionId,
    pub transform: Transform,
    pub mount_profile: Option<String>,
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct Attachment {
    pub id: StableId,
    pub parent_module: StableId,
    pub child_module: StableId,
    pub parent_socket: String,
    pub child_socket: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DecalPlacement {
    pub id: StableId,
    pub decal_asset: String,
    pub target_module: StableId,
    pub position: Vec3,
    pub scale: f32,
    pub rotation_degrees: f32,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ShipAppearance {
    pub primary: String,
    pub secondary: String,
    pub trim: String,
    pub pattern: Option<String>,
    pub camouflage: Option<String>,
    pub decals: Vec<DecalPlacement>,
}
impl Default for ShipAppearance {
    fn default() -> Self {
        Self { primary: "STEEL".into(), secondary: "GRAPHITE".into(), trim: "AMBER".into(), pattern: None, camouflage: None, decals: Vec::new() }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ShipBlueprint {
    pub schema_version: u32,
    pub id: StableId,
    pub name: String,
    pub role: String,
    pub ship_class: String,
    pub seed: u32,
    pub modules: Vec<ModuleInstance>,
    pub attachments: Vec<Attachment>,
    pub appearance: ShipAppearance,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct BlueprintValidation {
    pub valid: bool,
    pub errors: Vec<String>,
    pub warnings: Vec<String>,
}

impl ShipBlueprint {
    pub const SCHEMA_VERSION: u32 = 2;
    pub fn validate(&self) -> BlueprintValidation {
        let mut errors = Vec::new();
        let mut warnings = Vec::new();
        if !self.id.is_valid() { errors.push("blueprint id is invalid".into()); }
        if self.schema_version != Self::SCHEMA_VERSION {
            errors.push(format!("unsupported schema version {} (expected {})", self.schema_version, Self::SCHEMA_VERSION));
        }
        let mut ids = HashSet::new();
        for module in &self.modules {
            if !module.id.is_valid() { errors.push(format!("module {} has invalid id", module.definition.0)); }
            else if !ids.insert(module.id) { errors.push(format!("duplicate module id {}", module.id)); }
        }
        for edge in &self.attachments {
            if !edge.id.is_valid() { errors.push("attachment id is invalid".into()); }
            if !ids.contains(&edge.parent_module) { errors.push(format!("attachment {} parent is missing", edge.id)); }
            if !ids.contains(&edge.child_module) { errors.push(format!("attachment {} child is missing", edge.id)); }
            if edge.parent_module == edge.child_module { errors.push(format!("attachment {} self-links a module", edge.id)); }
        }
        let mut parents: HashMap<StableId, usize> = HashMap::new();
        for edge in &self.attachments { *parents.entry(edge.child_module).or_default() += 1; }
        for (child, count) in parents {
            if count > 1 { warnings.push(format!("module {child} has {count} structural parents")); }
        }
        BlueprintValidation { valid: errors.is_empty(), errors, warnings }
    }
}
