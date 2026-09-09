#include "core/physics/Octree.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace subspace {

// ---------------------------------------------------------------------------
// AABB
// ---------------------------------------------------------------------------

AABB::AABB(const Vector3& center, const Vector3& halfSize)
    : center(center)
    , halfSize(halfSize)
{
}

bool AABB::Contains(const Vector3& point) const
{
    return std::abs(point.x - center.x) <= halfSize.x
        && std::abs(point.y - center.y) <= halfSize.y
        && std::abs(point.z - center.z) <= halfSize.z;
}

bool AABB::IntersectsSphere(const Vector3& sphereCenter, float radius) const
{
    // Find the closest point on the AABB to the sphere center
    float dx = std::max(0.0f, std::abs(sphereCenter.x - center.x) - halfSize.x);
    float dy = std::max(0.0f, std::abs(sphereCenter.y - center.y) - halfSize.y);
    float dz = std::max(0.0f, std::abs(sphereCenter.z - center.z) - halfSize.z);
    return (dx * dx + dy * dy + dz * dz) <= (radius * radius);
}

bool AABB::Intersects(const AABB& other) const
{
    return std::abs(center.x - other.center.x) <= (halfSize.x + other.halfSize.x)
        && std::abs(center.y - other.center.y) <= (halfSize.y + other.halfSize.y)
        && std::abs(center.z - other.center.z) <= (halfSize.z + other.halfSize.z);
}

// ---------------------------------------------------------------------------
// Octree
// ---------------------------------------------------------------------------

Octree::Octree(const AABB& bounds, int maxDepth, int maxEntriesPerNode)
    : _bounds(bounds)
    , _depth(0)
    , _maxDepth(maxDepth)
    , _maxEntries(maxEntriesPerNode)
{
}

bool Octree::Insert(EntityId id, const Vector3& position, float radius)
{
    // Reject entries whose center is outside our bounds
    if (!_bounds.Contains(position)) {
        return false;
    }

    // If not subdivided and we have room, store here
    if (!_isSubdivided && static_cast<int>(_entries.size()) < _maxEntries) {
        _entries.push_back({id, position, radius});
        return true;
    }

    // Subdivide if we haven't already and haven't hit max depth
    if (!_isSubdivided) {
        if (_depth >= _maxDepth) {
            // At max depth, just store here regardless of capacity
            _entries.push_back({id, position, radius});
            return true;
        }
        Subdivide();
    }

    // Try to insert into a child
    int childIdx = GetChildIndex(position);
    const AABB& childBounds = _children[childIdx]->GetBounds();

    // Check if the entry's bounding sphere fits entirely in the child
    bool fitsInChild =
        (position.x - radius >= childBounds.center.x - childBounds.halfSize.x) &&
        (position.x + radius <= childBounds.center.x + childBounds.halfSize.x) &&
        (position.y - radius >= childBounds.center.y - childBounds.halfSize.y) &&
        (position.y + radius <= childBounds.center.y + childBounds.halfSize.y) &&
        (position.z - radius >= childBounds.center.z - childBounds.halfSize.z) &&
        (position.z + radius <= childBounds.center.z + childBounds.halfSize.z);

    if (fitsInChild) {
        return _children[childIdx]->Insert(id, position, radius);
    }

    // Entry straddles boundary — keep at this level
    _entries.push_back({id, position, radius});
    return true;
}

bool Octree::Remove(EntityId id)
{
    // Search local entries
    for (auto it = _entries.begin(); it != _entries.end(); ++it) {
        if (it->entityId == id) {
            _entries.erase(it);
            return true;
        }
    }

    // Search children
    if (_isSubdivided) {
        for (auto& child : _children) {
            if (child && child->Remove(id)) {
                return true;
            }
        }
    }

    return false;
}

void Octree::Clear()
{
    _entries.clear();
    _isSubdivided = false;
    for (auto& child : _children) {
        child.reset();
    }
}

std::vector<EntityId> Octree::QuerySphere(const Vector3& center, float radius) const
{
    std::vector<EntityId> results;
    QuerySphereInternal(center, radius, results);
    return results;
}

std::vector<EntityId> Octree::QueryBox(const AABB& box) const
{
    std::vector<EntityId> results;
    QueryBoxInternal(box, results);
    return results;
}

EntityId Octree::FindNearest(const Vector3& point) const
{
    EntityId bestId = InvalidEntityId;
    float bestDist = std::numeric_limits<float>::max();
    FindNearestInternal(point, bestId, bestDist);
    return bestId;
}

std::vector<EntityId> Octree::FindKNearest(const Vector3& point, int k) const
{
    if (k <= 0) {
        return {};
    }

    // Collect all entries
    std::vector<OctreeEntry> all;
    CollectAllEntries(all);

    // Sort by distance to point
    std::sort(all.begin(), all.end(),
        [&point](const OctreeEntry& a, const OctreeEntry& b) {
            float da = (a.position - point).length();
            float db = (b.position - point).length();
            return da < db;
        });

    int count = std::min(k, static_cast<int>(all.size()));
    std::vector<EntityId> results;
    results.reserve(count);
    for (int i = 0; i < count; ++i) {
        results.push_back(all[i].entityId);
    }
    return results;
}

size_t Octree::GetEntityCount() const
{
    size_t count = _entries.size();
    if (_isSubdivided) {
        for (const auto& child : _children) {
            if (child) {
                count += child->GetEntityCount();
            }
        }
    }
    return count;
}

size_t Octree::GetNodeCount() const
{
    return CountNodes();
}

int Octree::GetMaxUsedDepth() const
{
    int maxDepthUsed = 0;
    GetMaxDepthInternal(0, maxDepthUsed);
    return maxDepthUsed;
}

void Octree::Rebuild()
{
    std::vector<OctreeEntry> all;
    CollectAllEntries(all);
    Clear();
    for (const auto& entry : all) {
        Insert(entry.entityId, entry.position, entry.radius);
    }
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void Octree::Subdivide()
{
    _isSubdivided = true;

    Vector3 quarter = _bounds.halfSize * 0.5f;
    float cx = _bounds.center.x;
    float cy = _bounds.center.y;
    float cz = _bounds.center.z;
    float qx = quarter.x;
    float qy = quarter.y;
    float qz = quarter.z;

    // Create 8 children with octant offsets
    // Index bits: x=bit0, y=bit1, z=bit2
    for (int i = 0; i < 8; ++i) {
        float ox = (i & 1) ? qx : -qx;
        float oy = (i & 2) ? qy : -qy;
        float oz = (i & 4) ? qz : -qz;

        AABB childBounds(Vector3(cx + ox, cy + oy, cz + oz), quarter);
        _children[i] = std::make_unique<Octree>(childBounds, _maxDepth, _maxEntries);
        _children[i]->_depth = _depth + 1;
    }

    // Re-insert existing entries into children where possible
    std::vector<OctreeEntry> remaining;
    for (const auto& entry : _entries) {
        int childIdx = GetChildIndex(entry.position);
        const AABB& childBounds = _children[childIdx]->GetBounds();

        bool fitsInChild =
            (entry.position.x - entry.radius >= childBounds.center.x - childBounds.halfSize.x) &&
            (entry.position.x + entry.radius <= childBounds.center.x + childBounds.halfSize.x) &&
            (entry.position.y - entry.radius >= childBounds.center.y - childBounds.halfSize.y) &&
            (entry.position.y + entry.radius <= childBounds.center.y + childBounds.halfSize.y) &&
            (entry.position.z - entry.radius >= childBounds.center.z - childBounds.halfSize.z) &&
            (entry.position.z + entry.radius <= childBounds.center.z + childBounds.halfSize.z);

        if (fitsInChild) {
            _children[childIdx]->Insert(entry.entityId, entry.position, entry.radius);
        } else {
            remaining.push_back(entry);
        }
    }
    _entries = std::move(remaining);
}

int Octree::GetChildIndex(const Vector3& point) const
{
    int index = 0;
    if (point.x >= _bounds.center.x) index |= 1;
    if (point.y >= _bounds.center.y) index |= 2;
    if (point.z >= _bounds.center.z) index |= 4;
    return index;
}

void Octree::CollectAllEntries(std::vector<OctreeEntry>& out) const
{
    out.insert(out.end(), _entries.begin(), _entries.end());
    if (_isSubdivided) {
        for (const auto& child : _children) {
            if (child) {
                child->CollectAllEntries(out);
            }
        }
    }
}

void Octree::QuerySphereInternal(const Vector3& center, float radius, std::vector<EntityId>& results) const
{
    // Early out if query sphere doesn't intersect this node
    if (!_bounds.IntersectsSphere(center, radius)) {
        return;
    }

    // Check local entries
    for (const auto& entry : _entries) {
        float dist = (entry.position - center).length();
        if (dist <= radius + entry.radius) {
            results.push_back(entry.entityId);
        }
    }

    // Recurse into children
    if (_isSubdivided) {
        for (const auto& child : _children) {
            if (child) {
                child->QuerySphereInternal(center, radius, results);
            }
        }
    }
}

void Octree::QueryBoxInternal(const AABB& box, std::vector<EntityId>& results) const
{
    // Early out if query box doesn't intersect this node
    if (!_bounds.Intersects(box)) {
        return;
    }

    // Check local entries
    for (const auto& entry : _entries) {
        AABB entryBox(entry.position, Vector3(entry.radius, entry.radius, entry.radius));
        if (box.Intersects(entryBox)) {
            results.push_back(entry.entityId);
        }
    }

    // Recurse into children
    if (_isSubdivided) {
        for (const auto& child : _children) {
            if (child) {
                child->QueryBoxInternal(box, results);
            }
        }
    }
}

void Octree::FindNearestInternal(const Vector3& point, EntityId& bestId, float& bestDist) const
{
    // Prune: closest point on this node's AABB to the query point
    float dx = std::max(0.0f, std::abs(point.x - _bounds.center.x) - _bounds.halfSize.x);
    float dy = std::max(0.0f, std::abs(point.y - _bounds.center.y) - _bounds.halfSize.y);
    float dz = std::max(0.0f, std::abs(point.z - _bounds.center.z) - _bounds.halfSize.z);
    float minDistToBox = std::sqrt(dx * dx + dy * dy + dz * dz);

    if (minDistToBox > bestDist) {
        return;
    }

    // Check local entries
    for (const auto& entry : _entries) {
        float dist = (entry.position - point).length();
        if (dist < bestDist) {
            bestDist = dist;
            bestId = entry.entityId;
        }
    }

    // Recurse into children
    if (_isSubdivided) {
        for (const auto& child : _children) {
            if (child) {
                child->FindNearestInternal(point, bestId, bestDist);
            }
        }
    }
}

void Octree::GetMaxDepthInternal(int currentDepth, int& maxDepth) const
{
    if (!_entries.empty() && currentDepth > maxDepth) {
        maxDepth = currentDepth;
    }

    if (_isSubdivided) {
        for (const auto& child : _children) {
            if (child) {
                child->GetMaxDepthInternal(currentDepth + 1, maxDepth);
            }
        }
    }
}

size_t Octree::CountNodes() const
{
    size_t count = 1; // this node
    if (_isSubdivided) {
        for (const auto& child : _children) {
            if (child) {
                count += child->CountNodes();
            }
        }
    }
    return count;
}

} // namespace subspace
