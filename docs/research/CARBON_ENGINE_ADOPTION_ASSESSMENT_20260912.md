# Carbon Engine Adoption Assessment

## Context

The Carbon Engine technology behind EVE Online/EVE Frontier is now being published as open source component repositories. Subspace should use that opportunity selectively rather than attempt to replace its engine wholesale.

The project-level rule is:

> **one Subspace engine/runtime authority; external technology only enters through bounded interfaces after license, dependency, build, data-model and performance review.**

## High-value candidates

### Carbon Mesh — evaluate for integration

Repository: https://github.com/carbonengine/mesh  
License: MIT

The public project describes itself as handling 3D mesh, skeleton and animation serialization plus basic animation runtime support.

Potential Subspace value:

- Character workspace mesh/skeleton/animation pipeline;
- animation import/retarget support;
- unified ship/character animated-mesh processing where appropriate;
- offline inspection/conversion tooling.

Do not make Carbon mesh identity the canonical asset identity. Imported/converted data must remain registered through Subspace canonical asset/provenance systems.

### Carbon Resources — evaluate for integration/patterns

Repository: https://github.com/carbonengine/resources  
License: MIT

Potential value:

- resource manipulation/delivery patterns;
- asset CLI tooling;
- indexed resource/caching concepts;
- production build/resource packaging ideas.

Constraint: Subspace already has `CanonicalAssetRegistry`, `ContentAuthorityRegistry`, source-pack intake and Vault direction. Carbon Resources may strengthen those layers but must not create a second resource authority.

### Carbon Trinity — architecture/reference first

Repository: https://github.com/carbonengine/trinity  
License: MIT

The public repository exposes Carbon's rendering engine and build options for DirectX 11, DirectX 12, Metal and shader compiler targets.

Potential value:

- render-graph/backend organization research;
- shader compiler/effect pipeline research;
- large-space rendering techniques;
- DX11/DX12 modernization reference;
- asset/render object architecture.

Current decision: **do not replace the Subspace renderer during Foundation Convergence**. First extract backend-neutral render data and remove immediate-mode bottlenecks. Trinity can then be compared as a component/source reference against a clean Subspace renderer interface.

### Carbon Destiny — architecture reference

Repository: https://github.com/carbonengine/destiny  
License: MIT

The project describes itself as EVE's core game-world simulation engine and contains benchmark/tests/tooling. Current public build instructions still require access to private Perforce dependencies.

Use now for:

- authoritative simulation architecture research;
- spatial/world update patterns;
- benchmark methodology;
- large-fleet simulation ideas.

Do **not** make it a Subspace build dependency while its standalone public dependency closure is incomplete.

### Carbon Core — selective adoption only

Repository: https://github.com/carbonengine/core  
License: MIT

Use individual low-level utilities only when they replace a bounded Subspace implementation cleanly. Avoid importing a competing platform/runtime foundation wholesale.

### Carbon IO — defer transport choice

Repository: https://github.com/carbonengine/io  
License: MIT

Compare it later against GameNetworkingSockets and other transport candidates after Subspace defines authoritative replication, interest management, reconnect and persistence semantics. Transport must not dictate the higher-level network model.

### Carbon Audio — reference/optional integration

Repository: https://github.com/carbonengine/audio  
License: MIT

Carbon Audio wraps Wwise and includes sound prioritization/spatial-audio systems. It requires Wwise SDK/tooling, so Subspace should first define an audio-backend interface.

### Carbon Spatial Audio Clustering — evaluate algorithm

Repository: https://github.com/carbonengine/spatial-audio-clustering  
License: Apache-2.0

The public plugin groups nearby spatial audio objects to reduce endpoint/audio-thread pressure and reports large CPU savings in dense EVE/EVE Frontier scenes.

This is especially relevant to:

- fleets;
- stations;
- settlements/cities;
- turret/missile battles;
- weather/particle ambience;
- complex machinery.

The current implementation is a Wwise plugin. Subspace can evaluate either direct use after audio-backend selection or an independent clustering layer behind its own audio interface.

## Character technology note

The currently published Carbon component list exposes useful mesh/skeleton/animation foundations, but this review did **not** find a public modern Carbon character-creator/paperdoll repository that should simply be imported.

Subspace therefore owns its `CharacterDefinition`/sculpt/morph/apparel/portrait contracts. Carbon Mesh is evaluated underneath those contracts, not instead of them.

## Adoption order

1. Finish Foundation Convergence and the Shipyard editor/client contract.
2. Prototype Carbon Mesh against a copy of the Character workspace import/animation path.
3. Audit Carbon Resources against current canonical asset/Vault tooling.
4. Use Trinity as renderer modernization reference after the renderer interface is clean.
5. Use Destiny as simulation/benchmark reference; do not depend on it yet.
6. Evaluate Carbon Audio/spatial clustering once the audio abstraction exists.
7. Evaluate Carbon IO only when multiplayer transport selection begins.
