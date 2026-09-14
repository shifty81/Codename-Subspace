use std::env;
use subspace_test_fixtures::load_blueprint;

fn main() {
    let args: Vec<String> = env::args().collect();
    match args.get(1).map(String::as_str) {
        Some("validate-fixture") => {
            let Some(path) = args.get(2) else {
                eprintln!("usage: subspace_tools validate-fixture <path>");
                std::process::exit(2);
            };
            match load_blueprint(path) {
                Ok(blueprint) => {
                    let report = blueprint.validate();
                    if report.valid {
                        println!("PASS: {} is valid", blueprint.name);
                    } else {
                        for error in report.errors {
                            eprintln!("FAIL: {error}");
                        }
                        std::process::exit(3);
                    }
                }
                Err(error) => {
                    eprintln!("FAIL: {error}");
                    std::process::exit(4);
                }
            }
        }
        _ => {
            println!("Codename Subspace tools");
            println!("  validate-fixture <path>");
        }
    }
}
