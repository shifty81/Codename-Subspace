use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum DonorDisposition {
    KeepBehaviorContract,
    PortData,
    PortAlgorithm,
    RewriteRust,
    Retire,
    DonorTestOnly,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DonorSubsystem {
    pub id: String,
    pub source_area: String,
    pub rust_destination: String,
    pub disposition: DonorDisposition,
    pub notes: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ParityFixtureManifest {
    pub schema: String,
    pub donor_commit: String,
    pub subsystem: String,
    pub fixture_version: u32,
    pub expected_semantics: Vec<String>,
    pub intentional_differences: Vec<String>,
}
