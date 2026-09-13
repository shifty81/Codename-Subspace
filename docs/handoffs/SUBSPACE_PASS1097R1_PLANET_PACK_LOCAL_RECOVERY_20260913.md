# Codename Subspace — Pass1097R1 Planet Pack Local Recovery

## Purpose

The Pass1023–1097 cumulative patch applied successfully under the project-owned PCC, but the Full Gate stopped before build/test because the governed Various Planets runtime pack could not be materialized: `various_planets.glb` was not present in the current normalized project root.

This repair keeps the fidelity gate fail-closed while making local recovery practical after project-root renames.

## Changes

- Reuse an already hash-verified `content/derived/various_planets_v1` pack from an immediate sibling Subspace/Nullharbor checkout.
- Accept `SUBSPACE_PLANET_PACK_CACHE` as an explicit verified-pack cache.
- Continue to accept `SUBSPACE_PLANET_PACK_SOURCE` or `--source` for the licensed GLB.
- Check `Downloads` and `Desktop` for `various_planets.glb`.
- Check immediate sibling Subspace/Nullharbor project roots for the original GLB.
- Never recursively scan an entire drive.
- Never download Sketchfab content automatically.
- Verify every copied texture against `planet_pack_manifest.json` before publishing the runtime pack.

## Baseline

This repair is intended to apply after Pass1023–1097 has already been installed while Git HEAD remains:

`9440a5254ee80ab901f5c2d67e6a5bc14ae8423a`

## Test

After applying, rerun the internal PCC Full Gate.

Expected first stages:

1. Pre-patch safety snapshot — PASS
2. Pending-update guard — PASS
3. Supply-chain gate — PASS
4. Planet texture/cloud fidelity pack — PASS if a verified prior pack or licensed GLB is locally available

If no local source/cache exists, the gate remains intentionally blocked and prints the supported recovery locations/environment variables.
