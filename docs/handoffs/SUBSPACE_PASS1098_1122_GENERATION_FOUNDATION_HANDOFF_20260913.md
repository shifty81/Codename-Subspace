# Subspace Pass1098-1122 — Generation Foundation Handoff

## Apply/test order

1. Drop this `.patch` unextracted into the project root.
2. Use the project-owned internal PCC while ForgePY's PowerShell-interpreter routing remains under repair.
3. Approve the patch and run **FULL QUALITY GATE / CERTIFY GREEN**.
4. Launch Shipyard and reproduce the prior large-class failure case.

## Manual acceptance

### Deterministic request

- Pick one Ship class/size/role/domain and note the seed.
- Press GENERATE twice without pressing REROLL.
- Both attempts must use the same request/seed. The result must not silently vary because GENERATE changed seed.
- Press REROLL once; only then should the seed/request identity change.

### Domain integrity

- Set the generator domain to `Prop / Fixture`, `Station`, or another non-ship domain while in the legacy Systems/PCG surface.
- Pressing the ship GENERATE command must refuse with a clear domain message. It must not generate a ship behind the misleading domain label.

### Flat-plane regression

- Reproduce the prior Battlecruiser/L generation attempt.
- The system must **not** create a ~28x-scaled flat slab.
- If the current certified module vocabulary cannot produce a legal Battlecruiser topology, generation should fail closed and leave the current ship unchanged.
- That rejection is expected until the next topology synthesis lane lands.

### Scale/detail protection

- Inspect generated module transforms. No generated instance should exceed the certified scale ceiling.
- Small parts should no longer become enormous merely to satisfy a class hull-length target.

### Existing authoring

- Staged drag/move/rotate/snap authoring must remain functional.
- Previously authored/manual ships should remain editable even if the procedural generator rejects a class request.

## Expected temporary limitation

The old showcase ship generator still supplies candidate drafts. Pass1098-1122 intentionally puts a hard integrity boundary in front of it rather than claiming it has already been replaced. Large classes can therefore reject until player-scale calibration and topology-first generation are implemented.
