# Parallel Rust Lane

Codename Subspace remains one repository during the rewrite.

```text
/
├─ engine/            certified C++ donor/current runtime lane
├─ rust/              new Rust implementation lane
├─ docs/
├─ content/
└─ tools/
```

Rules:
1. No C++ GUI architecture is copied into Rust.
2. Rust lane is independently buildable/testable.
3. Ember renders/docks panels; Subspace contributes domain panels/tools/contracts.
4. Forge and Cortex remain external products connected through stable contracts.
5. C++/Rust parity is fixture/contract based, not source-line based.
6. Every subsystem moves inventory → fixture → Rust implementation → parity → promotion.
7. Once a Rust subsystem is promoted, feature work for it stops in C++ unless needed for a migration blocker.
