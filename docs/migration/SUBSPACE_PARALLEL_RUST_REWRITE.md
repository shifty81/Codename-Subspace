# Codename Subspace Parallel Rust Rewrite

The repository now carries both implementation lanes:
- `engine/` — certified C++ donor/current runtime
- `rust/` — normalized Rust rewrite

The Rust lane is the architectural destination. C++ remains available for behavioral
comparison and gameplay not yet migrated. Do not copy the C++ hand-drawn editor GUI into Rust.
Ember is the editor shell and docking host.
