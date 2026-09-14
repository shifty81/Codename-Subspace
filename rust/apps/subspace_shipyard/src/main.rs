use std::env;
use subspace_ember_bridge::contribution;
use subspace_shipyard::{default_layouts, CommandRegistry, ShipyardDocument};
use subspace_test_fixtures::load_blueprint;

fn main() {
    let path = env::args().nth(1).unwrap_or_else(|| "content/fixtures/sample_ship_blueprint.json".into());
    let blueprint = match load_blueprint(&path) {
        Ok(v) => v,
        Err(error) => { eprintln!("Could not load blueprint fixture '{path}': {error}"); std::process::exit(2); }
    };
    let validation = blueprint.validate();
    if !validation.valid {
        for error in validation.errors { eprintln!("Blueprint validation: {error}"); }
        std::process::exit(3);
    }
    let document = ShipyardDocument::new(blueprint);
    let contribution = contribution();
    let commands = CommandRegistry::professional_defaults();
    let layouts = default_layouts();

    println!("Codename Subspace parallel Rust Shipyard lane");
    println!("Document : {}", document.blueprint.name);
    println!("Modules  : {}", document.blueprint.modules.len());
    println!("Panels   : {}", contribution.panels.len());
    println!("Tools    : {}", contribution.tools.len());
    println!("Layouts  : {}", layouts.len());
    println!("Commands : {}", commands.search("").len());
    println!("Workspaces: BUILD / INTERIOR / SYSTEMS / APPEARANCE / TEST / DEV");
    println!("Ember owns panel rendering/docking; Subspace owns domain state and editor contributions.");
}
