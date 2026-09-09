#pragma once

#include "core/Math.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace subspace {

enum class WorldBrushMode {
    Raise,
    Lower,
    Smooth,
    Flatten,
    Terrace,
    Erode,
    Noise,
    PaintSurface,
    PaintBiome,
    PaintResource,
    River,
    Lake,
    Canal
};

enum class TerraformOperationType {
    Raise,
    Lower,
    Flatten,
    Smooth,
    Terrace,
    Excavation,
    Fill,
    MaterialPaint,
    BiomePaint,
    ResourceOverride,
    WaterLevel,
    Canal
};

struct PlanetWorldDefinition {
    std::string planetId;
    std::uint64_t seed = 1;
    float radiusMeters = 6371000.0f;
    float seaLevelMeters = 0.0f;
    float elevationAmplitudeMeters = 3200.0f;
    float macroFrequency = 0.000035f;
    float detailFrequency = 0.00021f;
    float erosionStrength = 0.45f;
    float temperatureKelvin = 288.0f;
    float moisture = 0.50f;
    float atmospherePressureKpa = 101.3f;
    float oxygenFraction = 0.21f;
    bool hasHydrosphere = true;
    bool terraformable = true;
};

struct StrategicPlanetHex {
    std::string id;
    int q = 0;
    int r = 0;
    std::string ownerFactionId;
    std::string biome;
    float resourceRichness = 0.0f;
    float habitability = 0.0f;
};

struct TerrainSample {
    Vector3 localPosition{};
    float baseHeightMeters = 0.0f;
    float terraformDeltaMeters = 0.0f;
    float finalHeightMeters = 0.0f;
    float moisture = 0.0f;
    float temperatureKelvin = 0.0f;
    std::string surfaceMaterial = "Rock";
    std::string biome = "Barren";
};

struct TerraformOperation {
    std::uint64_t id = 0;
    TerraformOperationType type = TerraformOperationType::Raise;
    Vector3 center{};
    float radiusMeters = 10.0f;
    float strengthMeters = 1.0f;
    float targetHeightMeters = 0.0f;
    float falloff = 0.5f;
    std::string materialId;
    std::string ownerFactionId;
    bool gameplayAuthorized = false;
};

struct TerraformDeltaLayer {
    std::vector<TerraformOperation> operations;
    std::uint64_t revision = 0;
};

struct PlanetTerrainChunk {
    std::string id;
    Vector3 origin{};
    float sizeMeters = 512.0f;
    std::uint32_t resolution = 33;
    std::vector<float> heights;
    std::uint64_t sourceRevision = 0;
    bool collisionDirty = true;
    bool materialDirty = true;
    bool hydrologyDirty = false;
};

struct FoundationGradePlan {
    bool valid = false;
    float targetHeightMeters = 0.0f;
    float excavationCubicMeters = 0.0f;
    float fillCubicMeters = 0.0f;
    float estimatedSlope = 0.0f;
    std::vector<TerraformOperation> operations;
    std::vector<std::string> warnings;
};

struct PlanetWorldAuthoringState {
    PlanetWorldDefinition world{};
    TerraformDeltaLayer terraform{};
    WorldBrushMode brush = WorldBrushMode::Raise;
    float brushRadiusMeters = 25.0f;
    float brushStrengthMeters = 2.0f;
    bool showStrategicHexes = true;
    bool showHydrology = false;
    bool showResources = false;
    bool liveCollision = true;
    std::string status = "World workspace ready";
};

class PlanetWorldEngineSystem {
public:
    static float BaseHeight(const PlanetWorldDefinition& world, float xMeters, float yMeters);
    static float TerraformDeltaAt(const TerraformDeltaLayer& layer, float xMeters, float yMeters);
    static TerrainSample Sample(const PlanetWorldDefinition& world,
                                const TerraformDeltaLayer& layer,
                                float xMeters,
                                float yMeters);
    static PlanetTerrainChunk BuildChunk(const PlanetWorldDefinition& world,
                                         const TerraformDeltaLayer& layer,
                                         Vector3 origin,
                                         float sizeMeters = 512.0f,
                                         std::uint32_t resolution = 33);
    static bool ApplyOperation(TerraformDeltaLayer& layer, TerraformOperation operation);
    static FoundationGradePlan PlanFoundationGrade(const PlanetWorldDefinition& world,
                                                   const TerraformDeltaLayer& layer,
                                                   Vector3 center,
                                                   Vector3 footprintMeters,
                                                   const std::string& ownerFactionId = {});
    static const char* BrushName(WorldBrushMode mode);
    static const char* TerraformName(TerraformOperationType type);
};

} // namespace subspace
