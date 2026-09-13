# Codename Subspace — Planetary Settlement & Exploration Authority

**Status:** Current architecture authority

## Identity

Planetary settlement and exploration gameplay is inspired by the strengths of large space sandboxes but is implemented as one Subspace-native simulation.

Settlements, exploration records, territory, industry, factions, logistics and physical geography all share persistent authority.

## Settlement model

Each settlement has:
- SettlementId
- world/planet/region coordinates
- controlling faction
- population
- role/economy
- security/law
- production/consumption
- inventories
- infrastructure
- landing facilities
- road/logistics links
- power/life-support dependencies
- territorial influence
- state
- services
- persistent local changes

Settlement scales range from remote field sites and camps to towns, industrial complexes and major cities.

Settlement state can evolve through prosperity, shortage, conflict, occupation, damage, abandonment and rebuilding.

## Physical representation

A settlement exists simultaneously as:
- strategic simulation record;
- regional infrastructure node;
- streamed exterior;
- selected loaded interior cells.

All representations reference the same SettlementId.

## Exploration authority

Exploration produces persistent Survey Records.

Survey Records may cover:
- stars/celestial bodies
- planetary terrain
- atmosphere/weather
- geology/resources
- biology
- settlements/factions
- anomalies
- ruins
- routes
- landing sites
- hazards

Survey quality improves incrementally from detection through ground-verified scientific/industrial-grade knowledge.

## Exploration economy

Information can be:
- sold
- licensed
- auctioned
- shared with a faction/corporation
- retained privately
- used for personal mining/colonization/strategy.

Different buyers value different discoveries.

Value depends on novelty, quality, distance, danger, rarity, freshness, resource potential, strategic importance and buyer demand.

## Exploration career

System scan
-> orbital survey
-> atmospheric approach
-> landing
-> on-foot/vehicle field work
-> samples/verification/POIs
-> return/transmit
-> monetize/license/use information
-> upgrade equipment
-> travel deeper into frontier space.

## Settlement/exploration feedback

Exploration can seed later development:
resource discovery
-> expedition
-> outpost
-> infrastructure
-> settlement
-> town/city.

Conflict or depletion can reverse that process and create abandoned/ruined exploration POIs.

## Reference inspiration

Elite Dangerous demonstrates useful reference patterns in which planetary settlements vary by economy/security/faction and exploration data/first discoveries produce economic value. Subspace retains those high-level lessons but replaces the static vendor-centric model with a persistent information economy integrated into factions, industry, colonization, territory and logistics.
