use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub enum AssetKind {
    ShipModule,
    Texture,
    Material,
    Interior,
    Blueprint,
    Planet,
    Station,
    Reference,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub enum LicenseStatus {
    Certified,
    Review,
    Quarantined,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct AssetProvenance {
    pub source_uri: String,
    pub author: Option<String>,
    pub license: Option<String>,
    pub source_hash: Option<String>,
    pub status: LicenseStatus,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct AssetRecord {
    pub id: String,
    pub display_name: String,
    pub kind: AssetKind,
    pub tags: Vec<String>,
    pub provenance: AssetProvenance,
}
