use serde::{Deserialize, Serialize};
use subspace_core::StableId;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ModuleSystemStats {
    pub module: StableId,
    pub hit_points: f32,
    pub armor: f32,
    pub mass_kg: f32,
    pub power_generation_kw: f32,
    pub power_draw_kw: f32,
    pub heat_generation_kw: f32,
    pub heat_dissipation_kw: f32,
    pub thrust_kn: f32,
    pub shield_capacity: f32,
}

#[derive(Debug, Clone, Default, PartialEq, Serialize, Deserialize)]
pub struct ShipSystemSummary {
    pub total_hit_points: f32,
    pub total_mass_kg: f32,
    pub net_power_kw: f32,
    pub net_heat_kw: f32,
    pub total_thrust_kn: f32,
    pub total_shield_capacity: f32,
}

pub fn summarize(modules: &[ModuleSystemStats]) -> ShipSystemSummary {
    let mut out = ShipSystemSummary::default();
    for module in modules {
        out.total_hit_points += module.hit_points.max(0.0);
        out.total_mass_kg += module.mass_kg.max(0.0);
        out.net_power_kw += module.power_generation_kw - module.power_draw_kw;
        out.net_heat_kw += module.heat_generation_kw - module.heat_dissipation_kw;
        out.total_thrust_kn += module.thrust_kn.max(0.0);
        out.total_shield_capacity += module.shield_capacity.max(0.0);
    }
    out
}
