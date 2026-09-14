use subspace_shipyard::CommandRegistry;

fn main() {
    let commands = CommandRegistry::professional_defaults();
    println!("Codename Subspace Rust runtime bootstrap");
    println!("Shipyard command surface: {} command(s)", commands.search("").len());
    println!("Next runtime milestone: shared Ember viewport + canonical ShipBlueprint rendering.");
}
