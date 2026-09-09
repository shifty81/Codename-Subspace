#pragma once
#include "procedural/GalaxyGenerator.h"
#include <filesystem>
#include <string>
#include <vector>
namespace subspace {
struct ImportedPlanetVisualProfile {
    std::string id;
    std::string surfaceTexture;
    std::vector<std::string> cloudTextures;
    bool emissiveSurface = false;
    float surfaceRotationRate = 1.0f;
    float primaryCloudRotationRate = 1.0f;
    float secondaryCloudRotationRate = -0.62f;
    float cloudRadiusMultiplier = 1.006f;
    float atmosphereRadiusMultiplier = 1.018f;
    float hueVariation = 0.0f;
};
class ImportedPlanetVisualSystem {
public:
    static ImportedPlanetVisualProfile ProfileFor(PlanetType type);
    static std::filesystem::path PackRoot(const std::filesystem::path& repositoryRoot);
    static bool PackReady(const std::filesystem::path& repositoryRoot);
};
}
