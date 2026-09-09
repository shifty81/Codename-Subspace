#pragma once

#include "core/Math.h"
#include "core/ecs/Entity.h"

#include <array>
#include <memory>
#include <vector>

namespace subspace {

/// Axis-aligned bounding box used by the octree.
struct AABB {
    Vector3 center;
    Vector3 halfSize;

    AABB() = default;
    AABB(const Vector3& center, const Vector3& halfSize);

    /// Check if a point is inside this box.
    bool Contains(const Vector3& point) const;

    /// Check if a sphere overlaps this box.
    bool IntersectsSphere(const Vector3& sphereCenter, float radius) const;

    /// Check if another AABB overlaps this box.
    bool Intersects(const AABB& other) const;
};

/// Entry stored in the octree: an entity at a position with a radius.
struct OctreeEntry {
    EntityId entityId = InvalidEntityId;
    Vector3 position;
    float radius = 0.0f;
};

/// Octree for spatial partitioning of 3D entities.
/// Supports insertion, removal, and sphere/box queries.
///
/// The tree subdivides into 8 children when a node exceeds its capacity,
/// up to a configurable maximum depth.
class Octree {
public:
    /// Construct an octree covering the given region.
    /// @param bounds  AABB defining the region this node covers.
    /// @param maxDepth  Maximum subdivision depth (default 8).
    /// @param maxEntriesPerNode  Entries before subdivision (default 8).
    Octree(const AABB& bounds, int maxDepth = 8, int maxEntriesPerNode = 8);

    /// Insert an entity into the tree.
    bool Insert(EntityId id, const Vector3& position, float radius = 0.0f);

    /// Remove an entity from the tree. Returns true if found and removed.
    bool Remove(EntityId id);

    /// Clear all entries and collapse children.
    void Clear();

    /// Query all entities whose bounding sphere intersects the given sphere.
    std::vector<EntityId> QuerySphere(const Vector3& center, float radius) const;

    /// Query all entities within the given AABB.
    std::vector<EntityId> QueryBox(const AABB& box) const;

    /// Find the nearest entity to a given point. Returns InvalidEntityId if empty.
    EntityId FindNearest(const Vector3& point) const;

    /// Find the K nearest entities to a given point.
    std::vector<EntityId> FindKNearest(const Vector3& point, int k) const;

    /// Total number of entities in the tree.
    size_t GetEntityCount() const;

    /// Number of nodes (internal + leaf) in the tree.
    size_t GetNodeCount() const;

    /// Maximum depth actually used.
    int GetMaxUsedDepth() const;

    /// Get the bounds of this octree.
    const AABB& GetBounds() const { return _bounds; }

    /// Rebuild the tree from scratch (useful after many removals).
    void Rebuild();

private:
    void Subdivide();
    int GetChildIndex(const Vector3& point) const;
    void CollectAllEntries(std::vector<OctreeEntry>& out) const;
    void QuerySphereInternal(const Vector3& center, float radius, std::vector<EntityId>& results) const;
    void QueryBoxInternal(const AABB& box, std::vector<EntityId>& results) const;
    void FindNearestInternal(const Vector3& point, EntityId& bestId, float& bestDist) const;
    void GetMaxDepthInternal(int currentDepth, int& maxDepth) const;
    size_t CountNodes() const;

    AABB _bounds;
    int _depth;
    int _maxDepth;
    int _maxEntries;
    bool _isSubdivided = false;
    std::vector<OctreeEntry> _entries;
    std::array<std::unique_ptr<Octree>, 8> _children;
};

} // namespace subspace
