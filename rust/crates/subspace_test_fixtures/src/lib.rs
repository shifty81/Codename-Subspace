use std::{fs, path::Path};
use subspace_ship::ShipBlueprint;

pub fn load_blueprint(path: impl AsRef<Path>) -> Result<ShipBlueprint, String> {
    let text = fs::read_to_string(path.as_ref()).map_err(|e| e.to_string())?;
    serde_json::from_str(&text).map_err(|e| e.to_string())
}
