#include "rendering/ShipRenderer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <utility>

namespace subspace {

// -----------------------------------------------------------------------
// ShipMeshData
// -----------------------------------------------------------------------

void ShipMeshData::Clear() {
    batches.clear();
}

size_t ShipMeshData::TotalInstances() const {
    size_t total = 0;
    for (const auto& batch : batches) {
        total += batch.InstanceCount();
    }
    return total;
}

// -----------------------------------------------------------------------
// ShipChunk
// -----------------------------------------------------------------------

// Floor-division that handles negative coordinates correctly
static int FloorDiv(int a, int b) {
    return (a >= 0) ? (a / b) : ((a - b + 1) / b);
}

Vector3Int ShipChunk::GetChunkPos(const Vector3Int& gridPos) {
    return {
        FloorDiv(gridPos.x, CHUNK_SIZE),
        FloorDiv(gridPos.y, CHUNK_SIZE),
        FloorDiv(gridPos.z, CHUNK_SIZE)
    };
}

// -----------------------------------------------------------------------
// ShipRenderer
// -----------------------------------------------------------------------

// Key for grouping blocks by (shape, material)
using BatchKey = std::pair<BlockShape, MaterialType>;

ShipMeshData ShipRenderer::BuildMeshData(const Ship& ship) {
    ShipMeshData result;

    // Group blocks by (shape, material) using ordered map for determinism
    std::map<BatchKey, MeshBatch> groups;

    for (const auto& blockPtr : ship.blocks) {
        if (!blockPtr) continue;
        const Block& block = *blockPtr;

        BatchKey key{block.shape, block.material};
        auto& batch = groups[key];
        batch.shape = block.shape;
        batch.material = block.material;
        batch.positions.push_back({
            static_cast<float>(block.gridPos.x),
            static_cast<float>(block.gridPos.y),
            static_cast<float>(block.gridPos.z)
        });
        batch.scales.push_back({
            static_cast<float>(block.size.x),
            static_cast<float>(block.size.y),
            static_cast<float>(block.size.z)
        });
        batch.rotationIndices.push_back(block.rotationIndex);
    }

    result.batches.reserve(groups.size());
    for (auto& [key, batch] : groups) {
        result.batches.push_back(std::move(batch));
    }

    return result;
}

void ShipRenderer::RebuildDirtyChunks(
    const Ship& ship,
    std::unordered_map<Vector3Int, ShipChunk>& chunks)
{
    // Collect dirty chunks
    std::vector<Vector3Int> dirtyPositions;
    for (auto& [pos, chunk] : chunks) {
        if (chunk.dirty) {
            dirtyPositions.push_back(pos);
        }
    }

    // Rebuild each dirty chunk
    for (const auto& cpos : dirtyPositions) {
        auto& chunk = chunks[cpos];
        chunk.meshData.Clear();

        // Determine the grid-space bounds for this chunk
        int minX = cpos.x * ShipChunk::CHUNK_SIZE;
        int minY = cpos.y * ShipChunk::CHUNK_SIZE;
        int minZ = cpos.z * ShipChunk::CHUNK_SIZE;
        int maxX = minX + ShipChunk::CHUNK_SIZE;
        int maxY = minY + ShipChunk::CHUNK_SIZE;
        int maxZ = minZ + ShipChunk::CHUNK_SIZE;

        std::map<BatchKey, MeshBatch> groups;

        for (const auto& blockPtr : ship.blocks) {
            if (!blockPtr) continue;
            const Block& block = *blockPtr;

            // Check if block origin falls within this chunk
            if (block.gridPos.x >= minX && block.gridPos.x < maxX &&
                block.gridPos.y >= minY && block.gridPos.y < maxY &&
                block.gridPos.z >= minZ && block.gridPos.z < maxZ)
            {
                BatchKey key{block.shape, block.material};
                auto& batch = groups[key];
                batch.shape = block.shape;
                batch.material = block.material;
                batch.positions.push_back({
                    static_cast<float>(block.gridPos.x),
                    static_cast<float>(block.gridPos.y),
                    static_cast<float>(block.gridPos.z)
                });
                batch.scales.push_back({
                    static_cast<float>(block.size.x),
                    static_cast<float>(block.size.y),
                    static_cast<float>(block.size.z)
                });
                batch.rotationIndices.push_back(block.rotationIndex);
            }
        }

        for (auto& [key, batch] : groups) {
            chunk.meshData.batches.push_back(std::move(batch));
        }

        chunk.dirty = false;
    }
}

void ShipRenderer::MarkDirty(
    std::unordered_map<Vector3Int, ShipChunk>& chunks,
    const Block& block)
{
    // Mark the chunk containing the block's origin
    Vector3Int cpos = ShipChunk::GetChunkPos(block.gridPos);
    chunks[cpos].chunkPos = cpos;
    chunks[cpos].dirty = true;

    // Also mark chunks that the block extends into (multi-cell blocks)
    Vector3Int endPos = block.gridPos + block.size - Vector3Int::One();
    Vector3Int endChunk = ShipChunk::GetChunkPos(endPos);

    for (int cx = cpos.x; cx <= endChunk.x; ++cx) {
        for (int cy = cpos.y; cy <= endChunk.y; ++cy) {
            for (int cz = cpos.z; cz <= endChunk.z; ++cz) {
                Vector3Int cp{cx, cy, cz};
                chunks[cp].chunkPos = cp;
                chunks[cp].dirty = true;
            }
        }
    }
}

// -----------------------------------------------------------------------
// GreedyMesher
// -----------------------------------------------------------------------

std::vector<GreedyMesher::MergedFace> GreedyMesher::MergeFaces(
    const Ship& ship, const ShipChunk& chunk)
{
    std::vector<MergedFace> result;

    const int CS = ShipChunk::CHUNK_SIZE;
    int baseX = chunk.chunkPos.x * CS;
    int baseY = chunk.chunkPos.y * CS;
    int baseZ = chunk.chunkPos.z * CS;

    // Build a local material grid for the chunk (-1 = empty)
    // Using int to store material index; -1 means no block
    std::array<std::array<std::array<int, 16>, 16>, 16> grid;
    for (auto& plane : grid)
        for (auto& row : plane)
            row.fill(-1);

    for (const auto& blockPtr : ship.blocks) {
        if (!blockPtr) continue;
        const Block& block = *blockPtr;

        // Iterate all cells occupied by this block
        for (int dx = 0; dx < block.size.x; ++dx) {
            for (int dy = 0; dy < block.size.y; ++dy) {
                for (int dz = 0; dz < block.size.z; ++dz) {
                    int gx = block.gridPos.x + dx;
                    int gy = block.gridPos.y + dy;
                    int gz = block.gridPos.z + dz;

                    int lx = gx - baseX;
                    int ly = gy - baseY;
                    int lz = gz - baseZ;

                    if (lx >= 0 && lx < CS && ly >= 0 && ly < CS && lz >= 0 && lz < CS) {
                        grid[lx][ly][lz] = static_cast<int>(block.material);
                    }
                }
            }
        }
    }

    // Helper to look up material at a world position (returns -1 if empty or outside chunk)
    auto worldMat = [&](int wx, int wy, int wz) -> int {
        auto it = ship.occupiedCells.find({wx, wy, wz});
        if (it != ship.occupiedCells.end() && it->second) {
            return static_cast<int>(it->second->material);
        }
        return -1;
    };

    // Axis directions: 0=X, 1=Y, 2=Z
    // For each axis, sweep through slices and greedily merge exposed faces
    for (int axis = 0; axis < 3; ++axis) {
        for (int dir = -1; dir <= 1; dir += 2) {
            // For each slice perpendicular to the axis
            for (int d = 0; d < CS; ++d) {
                // Build a 2D mask of exposed faces with material index
                std::array<std::array<int, 16>, 16> mask;
                for (auto& row : mask) row.fill(-1);

                for (int u = 0; u < CS; ++u) {
                    for (int v = 0; v < CS; ++v) {
                        int lx, ly, lz;
                        if (axis == 0)      { lx = d; ly = u; lz = v; }
                        else if (axis == 1)  { lx = u; ly = d; lz = v; }
                        else                 { lx = u; ly = v; lz = d; }

                        int mat = grid[lx][ly][lz];
                        if (mat < 0) continue;

                        // Check neighbor in the face direction
                        int nx = baseX + lx;
                        int ny = baseY + ly;
                        int nz = baseZ + lz;
                        if (axis == 0) nx += dir;
                        else if (axis == 1) ny += dir;
                        else nz += dir;

                        int neighborMat = worldMat(nx, ny, nz);

                        // Emit face only if neighbor is empty or different material
                        if (neighborMat != mat) {
                            mask[u][v] = mat;
                        }
                    }
                }

                // Greedy merge the 2D mask into rectangles
                std::array<std::array<bool, 16>, 16> visited;
                for (auto& row : visited) row.fill(false);

                for (int u = 0; u < CS; ++u) {
                    for (int v = 0; v < CS; ++v) {
                        if (visited[u][v] || mask[u][v] < 0) continue;

                        int mat = mask[u][v];

                        // Expand width (along v)
                        int width = 1;
                        while (v + width < CS && !visited[u][v + width] &&
                               mask[u][v + width] == mat) {
                            ++width;
                        }

                        // Expand height (along u)
                        int height = 1;
                        bool canExpand = true;
                        while (canExpand && u + height < CS) {
                            for (int w = 0; w < width; ++w) {
                                if (visited[u + height][v + w] ||
                                    mask[u + height][v + w] != mat) {
                                    canExpand = false;
                                    break;
                                }
                            }
                            if (canExpand) ++height;
                        }

                        // Mark visited
                        for (int du = 0; du < height; ++du) {
                            for (int dv = 0; dv < width; ++dv) {
                                visited[u + du][v + dv] = true;
                            }
                        }

                        // Emit merged face
                        MergedFace face{};
                        face.axis = axis;
                        face.direction = dir;
                        face.material = static_cast<MaterialType>(mat);

                        // Compute world-space position and size
                        float px, py, pz, sx, sy, sz;
                        if (axis == 0) {
                            px = static_cast<float>(baseX + d) + (dir > 0 ? 1.0f : 0.0f);
                            py = static_cast<float>(baseY + u);
                            pz = static_cast<float>(baseZ + v);
                            sx = 0.0f;
                            sy = static_cast<float>(height);
                            sz = static_cast<float>(width);
                        } else if (axis == 1) {
                            px = static_cast<float>(baseX + u);
                            py = static_cast<float>(baseY + d) + (dir > 0 ? 1.0f : 0.0f);
                            pz = static_cast<float>(baseZ + v);
                            sx = static_cast<float>(height);
                            sy = 0.0f;
                            sz = static_cast<float>(width);
                        } else {
                            px = static_cast<float>(baseX + u);
                            py = static_cast<float>(baseY + v);
                            pz = static_cast<float>(baseZ + d) + (dir > 0 ? 1.0f : 0.0f);
                            sx = static_cast<float>(height);
                            sy = static_cast<float>(width);
                            sz = 0.0f;
                        }

                        face.position = {px, py, pz};
                        face.size = {sx, sy, sz};
                        result.push_back(face);
                    }
                }
            }
        }
    }

    return result;
}

} // namespace subspace
