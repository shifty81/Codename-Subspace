use serde::{Deserialize, Serialize};
use subspace_ship::ShipBlueprint;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GeneratorRequest {
    pub seed: u32,
    pub role: String,
    pub hull_class: String,
    pub target_size: String,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum CandidateState { Preview, Draft, Validated, Certified, Rejected }

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GeneratorExplanation {
    pub summary: String,
    pub decisions: Vec<String>,
    pub warnings: Vec<String>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GeneratorCandidate {
    pub request: GeneratorRequest,
    pub state: CandidateState,
    pub blueprint: Option<ShipBlueprint>,
    pub explanation: GeneratorExplanation,
}

pub trait ShipGenerator {
    fn generate(&self, request: &GeneratorRequest) -> GeneratorCandidate;
}

pub fn next_seed(seed: u32) -> u32 {
    seed.wrapping_mul(1664525).wrapping_add(1013904223)
}

pub fn new_seed_and_generate(generator: &dyn ShipGenerator, request: &GeneratorRequest) -> GeneratorCandidate {
    let mut rerolled = request.clone();
    rerolled.seed = next_seed(request.seed);
    generator.generate(&rerolled)
}

#[cfg(test)]
mod tests {
    use super::*;
    struct Echo;
    impl ShipGenerator for Echo {
        fn generate(&self, request: &GeneratorRequest) -> GeneratorCandidate {
            GeneratorCandidate {
                request: request.clone(),
                state: CandidateState::Preview,
                blueprint: None,
                explanation: GeneratorExplanation {
                    summary: format!("seed={}", request.seed),
                    decisions: vec![],
                    warnings: vec![],
                },
            }
        }
    }
    #[test]
    fn new_seed_generate_uses_same_generator() {
        let request = GeneratorRequest {
            seed: 7, role: "INDUSTRIAL".into(), hull_class: "FRIGATE".into(), target_size: "M".into(),
        };
        let candidate = new_seed_and_generate(&Echo, &request);
        assert_eq!(candidate.request.seed, next_seed(7));
        assert_ne!(candidate.request.seed, request.seed);
    }
}
