#pragma once

#include "core/Math.h"
#include "ships/Block.h"
#include "ships/Ship.h"

#include <unordered_map>
#include <vector>

namespace subspace {

// Mesh data for a single shape/material batch
struct MeshBatch {
    BlockShape shape;
    MaterialType material;
    std::vector<Vector3> positions;     // Instance positions
    std::vector<Vector3> scales;        // Instance scales
    std::vector<int> rotationIndices;   // Instance rotations

    size_t InstanceCount() const { return positions.size(); }
};

// Ship mesh data organized for instanced rendering
struct ShipMeshData {
    std::vector<MeshBatch> batches;

    void Clear();
    size_t TotalInstances() const;
};

// Chunk-based ship rendering (16x16x16 chunks)
class ShipChunk {
public:
    static constexpr int CHUNK_SIZE = 16;

    Vector3Int chunkPos;    // Chunk position in chunk coordinates
    ShipMeshData meshData;
    bool dirty = true;

    // Get chunk position for a block grid position
    static Vector3Int GetChunkPos(const Vector3Int& gridPos);
};

// Ship renderer - builds mesh data from ship blocks
class ShipRenderer {
public:
    // Rebuild all mesh data from ship blocks
    static ShipMeshData BuildMeshData(const Ship& ship);

    // Rebuild only dirty chunks
    static void RebuildDirtyChunks(const Ship& ship,
                                   std::unordered_map<Vector3Int, ShipChunk>& chunks);

    // Mark chunks as dirty for affected blocks
    static void MarkDirty(std::unordered_map<Vector3Int, ShipChunk>& chunks,
                          const Block& block);
};

// Greedy meshing for face merging
class GreedyMesher {
public:
    struct MergedFace {
        Vector3 position;
        Vector3 size;
        int axis;           // 0=X, 1=Y, 2=Z
        int direction;      // +1 or -1
        MaterialType material;
    };

    // Merge faces within a chunk
    static std::vector<MergedFace> MergeFaces(const Ship& ship, const ShipChunk& chunk);
};

} // namespace subspace
