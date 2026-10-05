#pragma once

#include "interior/InteriorInteractionSystem.h"
#include "interior/ShipInteriorLayoutSystem.h"
#include "interior/ShipEmbodimentSystem.h"

#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

struct StarterInteriorFixture {
    InteriorFixtureState interaction{};
    std::string label;
    Vector3 localCenter{};
    Vector3 halfExtents{0.3f,0.3f,0.4f};
    Vector3 useFeet{};
    bool blocksMovement=true;
};

struct StarterInteriorScene {
    bool ready=false;
    Vector3 boundsMin{};
    Vector3 boundsMax{};
    Vector3 spawnFeet{};
    std::vector<StarterInteriorFixture> fixtures;
    std::vector<std::string> warnings;
};

struct StarterInteriorFocus {
    int fixtureIndex=-1;
    float distanceMeters=0.0f;
    float aimScore=0.0f;
    bool valid() const { return fixtureIndex>=0; }
};

/// Compact, data-driven starter-ship furnishing layer built inside the
/// certified carved walkable shell. It does not replace hull/collision
/// authority; it gives the first playable interior physical stations and a
/// deterministic interaction vocabulary instead of an empty pressure box.
class StarterShipInteriorSceneSystem {
public:
    static StarterInteriorScene Build(const InteriorLayoutPlan& layout);
    static StarterInteriorFocus Focus(const StarterInteriorScene& scene,
                                      const InteriorAvatarState& avatar,
                                      float maxDistanceMeters=2.6f,
                                      float minimumAimScore=0.68f);
    static Vector3 ResolveFixtureCollision(const StarterInteriorScene& scene,
                                           Vector3 fromFeet,
                                           Vector3 toFeet,
                                           float capsuleRadiusMeters);
};

} // namespace subspace
